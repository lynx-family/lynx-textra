// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/paragraph_impl.h"

#include <textra/run_delegate.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "src/textlayout/internal/boundary_analyst.h"
#include "src/textlayout/layout_position.h"
#include "src/textlayout/run/ghost_run.h"
#include "src/textlayout/run/object_run.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/u_8_string.h"
#include "style/style_manager.h"

namespace ttoffice {
namespace tttext {
std::unique_ptr<Paragraph> Paragraph::Create() {
  return std::make_unique<ParagraphImpl>();
}
ParagraphImpl::ParagraphImpl()
    : Paragraph(),
      formated_(false),
      style_manager_(std::make_unique<StyleManager>()),
      boundary_analyst_(nullptr),
      shaper_(nullptr) {}
ParagraphImpl::~ParagraphImpl() = default;
void ParagraphImpl::AddTextRun(const Style& style, const char* content,
                               uint32_t length, bool ghost_text) {
  if (length == 0) {
    LogUtil::W("textlayout AddTextRun discard content.empty()");
    return;
  }
  if (!base::CheckValidUTF8String(content)) {
    LogUtil::E("textlayout AddTextRun discard not valid utf8 string :%s",
               content);
    return;
  }
  if (ghost_text) {
    TTASSERT(false);
  } else {
    auto start_char_pos = GetCharCount();
    auto end_char_pos = AddTextContent({content, length});
    TTASSERT(end_char_pos > start_char_pos);
    if (start_char_pos >= end_char_pos) {
      return;
    }
    style_manager_->ApplyStyleInRange(style, start_char_pos,
                                      end_char_pos - start_char_pos);
    run_lst_.emplace_back(std::make_unique<BaseRun>(
        this, style, start_char_pos, end_char_pos, RunType::kTextRun));
  }
}
/**
 * @brief Append a ObjectRun to paragraph, backed by the Style and RunDelegate.
 *
 * @param style
 * @param shape
 * @param need_placeholder True to append U+FFFC (Object Replacement Character)
 * to the text content.
 * @param is_float unused
 * @param offset_y unused
 */
void ParagraphImpl::AddShapeRun(const Style& style,
                                std::shared_ptr<RunDelegate> shape,
                                bool need_placeholder, bool is_float,
                                float offset_y) {
  auto start_char_pos = GetCharCount();
  auto run = std::make_unique<ObjectRun>(
      this, std::move(shape), start_char_pos,
      need_placeholder ? AddTextContent(BaseRun::ObjectReplacementCharacter())
                       : start_char_pos,
      RunType::kInlineObject);
  run->layout_style_ = style;
  style_manager_->ApplyStyleInRange(style, start_char_pos, 1);
  run_lst_.emplace_back(std::move(run));
}
bool ParagraphImpl::SplitRun(uint32_t idx, uint32_t char_pos_in_run) {
  auto* run = run_lst_[idx].get();
  TTASSERT(run->GetType() == RunType::kTextRun);
  run_lst_.insert(
      run_lst_.begin() + idx,
      std::make_unique<BaseRun>(
          this, run->GetLayoutStyle(), run->GetStartCharPos(),
          run->GetStartCharPos() + char_pos_in_run, run->GetType()));
  run->start_char_pos_ = run->GetStartCharPos() + char_pos_in_run;
  run->UpdateRunContent();
  return true;
}
void ParagraphImpl::FormatRunList() {
  if (shaper_ == nullptr) return;
  auto u32_content = content_.ToUTF32();
  if (!formated_) {
    style_manager_->SetParagraphStyle(paragraph_style_.GetDefaultStyle());
    FormatIndent();
    if (content_.Empty() && run_lst_.empty()) {
      AddTextRun(nullptr, "\n", 1);
    }
    TTASSERT(!content_.Empty() || !run_lst_.empty());
    auto u32_content = content_.ToUTF32();
    boundary_analyst_ = std::make_unique<BoundaryAnalyst>(
        u32_content.data(), u32_content.length(),
        paragraph_style_.line_break_strategy_);
    TTASSERT(u32_content.length() == GetCharCount());

    std::vector<bool> must_split_run_pos(GetCharCount(), false);
    visual_map_.resize(GetCharCount(), 0);
    logical_map_.resize(GetCharCount(), 0);
    bidi_level_.resize(GetCharCount(), 0);
    shaper_->ProcessBidirection(
        u32_content.data(), static_cast<uint32_t>(u32_content.length()),
        paragraph_style_.GetWriteDirection(), visual_map_.data(),
        logical_map_.data(), bidi_level_.data());

    bool need_split = false;
    for (auto k = 0u; k + 1 < GetCharCount(); k++) {
      auto ch = u32_content[k];
      auto next_ch32 = k + 1 < GetCharCount() ? u32_content[k + 1] : 0;
      // CRLF special processing to synthesize a run
      if (ch == '\n' || (ch == '\r' && next_ch32 != '\n')) {
        boundary_analyst_->UpgradeBoundaryType(Range::MakeLW(k, 1),
                                               BoundaryType::kMustLineBreak);
        must_split_run_pos[k] = true;
        need_split = true;
      }
      // split text with different bidirection
      if (IsRtlCharacter(k) != IsRtlCharacter(k + 1)) {
        boundary_analyst_->UpgradeBoundaryType(Range::MakeLW(k, 1),
                                               BoundaryType::kLineBreakable);
        must_split_run_pos[k] = true;
        need_split = true;
      }
    }

    auto idx = 0u;
    StyleRange style_range;
    while (idx < GetCharCount()) {
      style_manager_->GetStyleRange(
          &style_range, idx, Style::BaselineOffsetFlag(), GetCharCount());
      auto k = style_range.GetRange().GetEnd() - 1;
      boundary_analyst_->UpgradeBoundaryType(Range::MakeLW(k, 1),
                                             BoundaryType::kLineBreakable);
      must_split_run_pos[k] = true;
      need_split = true;
      idx = style_range.GetRange().GetEnd();
    }

    auto run_iter = run_lst_.begin();
    if (need_split) {
      for (auto kk = 0u; kk < must_split_run_pos.size(); kk++) {
        if (must_split_run_pos[kk]) {
          while ((*run_iter)->GetEndCharPos() < kk + 1) ++run_iter;
          TTASSERT(run_iter->get()->GetStartCharPos() <= kk &&
                   run_iter->get()->GetEndCharPos() > kk);
          if ((*run_iter)->GetEndCharPos() == kk + 1) continue;
          if (kk >= run_iter->get()->GetStartCharPos()) {
            auto iter_offset =
                static_cast<uint32_t>(run_iter - run_lst_.begin());
            SplitRun(iter_offset, kk - run_iter->get()->GetStartCharPos() + 1);
            run_iter = run_lst_.begin() + iter_offset + 1;
          }
        }
      }
    }

    run_iter = run_lst_.begin();
    while (run_iter != run_lst_.end()) {
      auto next_run = run_iter + 1;
      auto prev_run =
          run_iter == run_lst_.begin() ? run_lst_.end() : run_iter - 1;
      if (next_run == run_lst_.end())
        (*run_iter)->SetBoundaryType(BoundaryType::kLineBreakable);
      switch ((*run_iter)->GetType()) {
        case RunType::kTextRun:
          TTASSERT((*run_iter)->GetCharCount() > 0);
          TTASSERT((*run_iter)->GetEndCharPos() >= 1);
          (*run_iter)->SetBoundaryType(boundary_analyst_->GetBoundaryType(
              (*run_iter)->GetEndCharPos() - 1));
          break;
        case RunType::kGhostRun:
        case RunType::kInlineObject:
        case RunType::kFloatObject:
          if (prev_run != run_lst_.end())
            (*prev_run)->SetBoundaryType(BoundaryType::kLineBreakable);
          (*run_iter)->SetBoundaryType(BoundaryType::kLineBreakable);
          break;
        default:
          break;
      }
      ++run_iter;
    }
  }

  auto run_iter = run_lst_.begin();
  while (run_iter != run_lst_.end()) {
    auto& run = *run_iter;
    if (run->IsObjectRun()) {
      ++run_iter;
      continue;
    }
    if (run->IsGhostRun() || run->IsBlockRun()) {
      if (run->GetCharCount() > 0) {
        auto& text = run->ghost_content_;
        run->shape_result_.InitWithShapeResult(
            shaper_->ShapeText(text.ToUTF32().data(), text.GetCharCount(),
                               &run->GetShapeStyle(), run->IsRtl()),
            0, run->GetCharCount());
      }
      ++run_iter;
    } else {
      std::vector<BaseRun*> shape_list_;
      for (; run_iter != run_lst_.end() &&
             (*run_iter)->CanBeAppendToShaping(*run);
           ++run_iter) {
        shape_list_.push_back(run_iter->get());
      }
      TTASSERT(!shape_list_.empty());
      auto start_pos = shape_list_[0]->GetStartCharPos();
      auto end_pos = shape_list_.back()->GetEndCharPos();
      auto shape_result = shaper_->ShapeText(
          u32_content.data() + start_pos, end_pos - start_pos,
          &run->GetShapeStyle(), run->IsRtl());
      for (auto& shape_run : shape_list_) {
        shape_run->shape_result_.InitWithShapeResult(
            shape_result, shape_run->GetStartCharPos() - start_pos,
            shape_run->GetEndCharPos() - start_pos);
      }
      shape_list_.clear();
    }
  }
  for (auto& run : run_lst_) {
    run->Layout();
  }
  TTASSERT(!run_lst_.empty());
  formated_ = true;
}
void ParagraphImpl::FormatIndent() {
  const auto one_char_advance = GetDefaultStyle().GetTextSize();
  int start_chars = paragraph_style_.GetStartIndentInCharCnt();
  if (start_chars > 0) {
    paragraph_style_.SetStartIndentInPx(static_cast<float>(start_chars) *
                                        one_char_advance);
  }
  int end_chars = paragraph_style_.GetEndIndentInCharCnt();
  if (end_chars > 0) {
    paragraph_style_.SetEndIndentInPx(static_cast<float>(end_chars) *
                                      one_char_advance);
  }
  int first_line_chars = paragraph_style_.GetFirstLineIndentInCharCnt();
  if (first_line_chars > 0) {
    paragraph_style_.SetFirstLineIndentInPx(
        static_cast<float>(first_line_chars) * one_char_advance);
  }
  int hanging_chars = paragraph_style_.GetHangingIndentInCharCnt();
  if (hanging_chars > 0) {
    paragraph_style_.SetHangingIndentInPx(static_cast<float>(hanging_chars) *
                                          one_char_advance);
  }

  if (paragraph_style_.GetFirstLineIndentInPx() > 0 &&
      paragraph_style_.GetHangingIndentInPx() > 0) {
    LogUtil::W(
        "First line indentation and hanging indentation should be mutually "
        "exclusive");
  }
}
CharPos ParagraphImpl::LayoutPositionToCharPos(
    const LayoutPosition& pos) const {
  BaseRun* run = GetRun(pos.GetRunIdx());
  TTASSERT(run == nullptr
               ? (pos.GetRunIdx() == run_lst_.size() && pos.GetCharIdx() == 0)
               : run->GetCharCount() == 0 ||
                     pos.GetCharIdx() < run->GetCharCount());
  return run == nullptr ? GetCharCount()
                        : run->GetStartCharPos() + pos.GetCharIdx();
}
LayoutPosition ParagraphImpl::CharPosToLayoutPosition(uint32_t char_pos) const {
  auto iter = std::lower_bound(
      run_lst_.begin(), run_lst_.end(), char_pos,
      [](const std::unique_ptr<BaseRun>& run, const uint32_t& cpos) {
        return run->GetEndCharPos() <= cpos;
      });
  /**
   * Here char_pos defaults to characters with corresponding context text, skip
   * 0-character FFRun
   */
  while (iter != run_lst_.end() && (*iter)->GetCharCount() == 0) {
    TTASSERT((*iter)->IsControlRun());
    iter++;
  }
  if (iter == run_lst_.end()) {
    //    TTASSERT(run_lst_.size() == 1 && run_lst_[0]->GetCharCount() == 0);
    return {static_cast<uint32_t>(run_lst_.size()), 0};
  }
  TTASSERT(iter != run_lst_.end());
  TTASSERT(((*iter)->GetStartCharPos() <= char_pos &&
            (*iter)->GetEndCharPos() > char_pos));
  return {static_cast<uint32_t>(iter - run_lst_.begin()),
          char_pos - (*iter)->GetStartCharPos()};
}
bool ParagraphImpl::IsFirstCharOfParagraph(uint32_t char_pos) const {
  if (char_pos == 0) return true;
  if (char_pos == GetCharCount()) return true;
  TTASSERT(char_pos < GetCharCount());
  return boundary_analyst_->GetBoundaryTypeBefore(char_pos) ==
         BoundaryType::kParagraph;
}
LayoutPosition ParagraphImpl::FindNextBoundary(const LayoutPosition& start,
                                               const BoundaryType& type) const {
  auto* run = GetRun(start.GetRunIdx());
  TTASSERT(start.GetCharIdx() <= run->GetCharCount());
  // if (run->IsGhostRun()) return {start.GetRunIdx() + 1, 0};
  const auto char_pos_in_para = run->GetStartCharPos() + start.GetCharIdx();
  const auto boundary_start =
      boundary_analyst_->FindNextBoundary(char_pos_in_para, type);
  auto ret = CharPosToLayoutPosition(boundary_start);
  auto pos = start;
  while (pos < ret) {
    run = GetRun(pos.GetRunIdx());
    if (run->GetBoundaryType() >= type) {
      ret = pos.NextRun();
      break;
    }
    pos.NextRun();
  }
  return ret;
}
LayoutPosition ParagraphImpl::FindPrevBoundary(const LayoutPosition& start,
                                               const BoundaryType& type) const {
  TTASSERT(start.GetRunIdx() < run_lst_.size() || start.GetCharIdx() == 0);
  const auto* run = GetRun(start.GetRunIdx());
  if (run == nullptr) {
    TTASSERT(start.GetRunIdx() == run_lst_.size());
    return start;
  }

  auto pos = start;
  while (run->GetCharCount() == 0) {
    if (pos.GetRunIdx() == 0) {
      return {0, 0};
    }
    run = GetRun(pos.GetRunIdx() - 1);
    TTASSERT(run != nullptr);
    if (run->GetBoundaryType() >= type) {
      return {pos.GetRunIdx(), 0};
    }
    if (run->GetCharCount() > 0)
      pos = {pos.GetRunIdx() - 1, run->GetCharCount() - 1};
  }
  TTASSERT(start.GetCharIdx() <= run->GetCharCount());
  auto char_pos_in_para = run->GetStartCharPos() + pos.GetCharIdx();
  auto boundary_start =
      boundary_analyst_->FindPrevBoundary(char_pos_in_para, type);
  auto ret = CharPosToLayoutPosition(boundary_start);
  TTASSERT(ret.GetCharIdx() < GetRun(ret.GetRunIdx())->GetCharCount());
  while (pos.GetRunIdx() > ret.GetRunIdx()) {
    run = GetRun(pos.GetRunIdx() - 1);
    if (run == nullptr) {
      break;
    }
    if (run->GetBoundaryType() >= type) {
      ret = pos;
      break;
    }
    pos.PrevRun();
  }
  return ret;
}
BoundaryType ParagraphImpl::GetBoundaryTypeBefore(
    const LayoutPosition& position) const {
  if (position.GetRunIdx() == 0) return BoundaryType::kParagraph;
  auto* run = GetRun(position.GetRunIdx() - 1);
  return run->GetBoundaryType();
}
BoundaryType ParagraphImpl::GetBoundaryType(
    const LayoutPosition& position) const {
  if (position.GetRunIdx() == run_lst_.size()) return BoundaryType::kParagraph;
  auto* run = GetRun(position.GetRunIdx());
  return run->GetBoundaryType();
}
void ParagraphImpl::ApplyStyleInRange(const Style& style, const CharPos start,
                                      const uint32_t len) const {
  style_manager_->ApplyStyleInRange(style, start, len);
}
float ParagraphImpl::GetMaxIntrinsicWidth() const {
  if (!formated_) return 0;
  float max_width = 0;
  float tmp_width = 0;
  uint32_t start_pos = 0;
  auto next_hard_break_pos =
      boundary_analyst_->FindNextBoundary(0, BoundaryType::kMustLineBreak);
  auto iter = run_lst_.begin();
  while (iter != run_lst_.end()) {
    auto& run = *iter;
    auto piece_start_pos = std::max(start_pos, run->GetStartCharPos());
    auto piece_end_pos = std::min(next_hard_break_pos, run->GetEndCharPos());
    if (piece_end_pos > piece_start_pos) {
      tmp_width += run->GetWidth(piece_start_pos - run->GetStartCharPos(),
                                 piece_end_pos - piece_start_pos);
    }
    if (piece_end_pos == next_hard_break_pos) {
      max_width = std::fmax(max_width, tmp_width);
      start_pos = next_hard_break_pos;
      next_hard_break_pos = boundary_analyst_->FindNextBoundary(
          next_hard_break_pos, BoundaryType::kMustLineBreak);
      tmp_width = 0;
    }
    if (run->GetEndCharPos() == piece_end_pos) {
      ++iter;
    }
  }
  return max_width;
}
float ParagraphImpl::GetMinIntrinsicWidth() const {
  if (!formated_) return 0;
  TTASSERT(false);
  return 0;
}

std::pair<uint32_t, uint32_t> ParagraphImpl::GetWordBoundary(
    uint32_t offset) const {
  uint32_t start =
      boundary_analyst_->FindPrevBoundary(offset, BoundaryType::kWord);
  uint32_t end =
      boundary_analyst_->FindNextBoundary(offset, BoundaryType::kWord);
  if (start > end) {
    return {0, 0};
  }
  return {start, end};
}

RunDelegate* ParagraphImpl::GetRunDelegateForChar(uint32_t char_index) const {
  auto pos = CharPosToLayoutPosition(char_index);
  auto run = GetRun(pos.GetRunIdx());
  if (run == nullptr) return nullptr;
  if (run->GetType() != RunType::kInlineObject &&
      run->GetType() != RunType::kFloatObject)
    return nullptr;
  return run->GetRunDelegate();
}

#ifdef TTTEXT_DEBUG
std::u32string ParagraphImpl::GetContentWithGhost() const {
  std::u32string content;
  for (const auto& run : run_lst_) {
    if (run->IsGhostRun()) {
      content += run->ghost_content_.ToUTF32();
    } else {
      content += run->run_content_;
    }
  }
  return content;
}
#endif
}  // namespace tttext
}  // namespace ttoffice
