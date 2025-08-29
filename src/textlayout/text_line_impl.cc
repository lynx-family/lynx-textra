// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/text_line_impl.h"

#include <textra/layout_region.h>

#include <algorithm>
#include <cassert>
#include <utility>

#include "src/textlayout/internal/boundary_analyst.h"
#include "src/textlayout/internal/line_range.h"
#include "src/textlayout/internal/run_range.h"
#include "src/textlayout/layout_measurer.h"
#include "src/textlayout/run/ghost_run.h"
#include "src/textlayout/run/object_run.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/utils/log_util.h"
#include "src/textlayout/utils/u_8_string.h"
#include "utils/float_comparison.h"
namespace ttoffice {
namespace tttext {
TextLineImpl::TextLineImpl(ParagraphImpl* paragraph, LayoutRegion* lp,
                           const LayoutPosition& pos)
    : paragraph_(paragraph), layout_page_(lp) {
  line_start_pos_ = pos;
  line_end_pos_ = pos;
  auto run_idx = pos.GetRunIdx();
  auto* run = paragraph_->GetRun(run_idx);
  while (run->IsGhostRun() && run_idx + 1 < paragraph_->GetRunCount())
    run = paragraph_->GetRun(++run_idx);
  const auto& paragraph_style = paragraph->paragraph_style_;
  start_indent_ = paragraph_style.GetStartIndentInPx();
  end_indent_ = paragraph_style.GetEndIndentInPx();
  if (paragraph->IsFirstCharOfParagraph(run->GetStartCharPos() +
                                        pos.GetCharIdx())) {
    start_indent_ = paragraph_style.GetFirstLineIndentInPx();
  } else {
    start_indent_ += paragraph_style.GetHangingIndentInPx();
  }
}
TextLineImpl::~TextLineImpl() = default;
void TextLineImpl::SetRangeLst(const std::vector<std::array<float, 2>>& lst) {
  range_lst_.clear();
  for (auto& range : lst) {
    range_lst_.emplace_back(std::make_unique<LineRange>(range[0], range[1]));
  }
  if (!range_lst_.empty()) current_available_range_index_ = 0;
}
LayoutPosition TextLineImpl::UpdateLine(LayoutPosition pos, float max_ascent,
                                        float max_descent,
                                        float desired_height) {
  line_end_pos_ = pos;

  if (-max_ascent > max_ascent_) max_ascent_ = -max_ascent;
  if (max_descent > max_descent_) max_descent_ = max_descent;

  auto& para_style = paragraph_->paragraph_style_;
  auto diff = desired_height - GetContentHeight();
  switch (para_style.GetVerticalAlign()) {
    case ParagraphVerticalAlignment::kTop:
      top_extra_ = 0;
      bottom_extra_ = diff;
      break;
    case ParagraphVerticalAlignment::kCenter:
      top_extra_ = bottom_extra_ = diff / 2;
      break;
    case ParagraphVerticalAlignment::kBottom:
      top_extra_ = diff;
      bottom_extra_ = 0;
      break;
  }
  return pos;
}

bool TextLineImpl::IsEqualRangeList(
    const std::vector<std::array<float, 2>>& list) const {
  if (range_lst_.size() != list.size()) return false;
  for (auto k = 0u; k < range_lst_.size(); k++) {
    if (!FloatsEqual(range_lst_[k]->GetXMin(), list[k][0]) ||
        !FloatsEqual(range_lst_[k]->GetXMax(), list[k][1]))
      return false;
  }
  return true;
}
bool TextLineImpl::IsEmpty() const {
  if (range_lst_.empty()) return true;
  for (auto& range : range_lst_) {
    if (!range->Empty()) return false;
  }
  return true;
}
float TextLineImpl::GetLineLeft() const {
  TTASSERT(IsLayouted());
  if (!IsLayouted() || range_lst_.empty()) return 0;
  return drawer_list_.empty() ? range_lst_[0]->GetContentLeft()
                              : drawer_list_[0]->GetLeft();
}
float TextLineImpl::GetLineRight() const {
  TTASSERT(IsLayouted());
  if (!IsLayouted() || range_lst_.empty()) return 0;
  return drawer_list_.empty() ? range_lst_.back()->GetContentRight()
                              : drawer_list_.back()->GetRight();
}
CharPos TextLineImpl::GetStartCharPos() const {
  return paragraph_->LayoutPositionToCharPos(line_start_pos_);
}
CharPos TextLineImpl::GetEndCharPos() const {
  return paragraph_->LayoutPositionToCharPos(line_end_pos_);
}
void TextLineImpl::SplitToWordDrawer(const LineRange& line_range,
                                     float word_spacing) {
  for (const auto& run_range : line_range.run_range_lst_) {
    if (run_range->GetRun()->IsGhostRun()) {
      drawer_list_.push_back(std::make_unique<DrawerPiece>(*run_range));
    } else {
      auto* run = run_range->GetRun();
      auto start_char = run_range->GetStartCharPosInParagraph();
      auto end_char = run_range->GetEndCharPosInParagraph();
      auto next_word_boundary = paragraph_->boundary_analyst_->FindNextBoundary(
          start_char, BoundaryType::kWord);
      while (next_word_boundary < end_char) {
        auto drawer = std::make_unique<DrawerPiece>(
            run, run_range->GetParent(), start_char - run->GetStartCharPos(),
            next_word_boundary - run->GetStartCharPos());
        InsertDrawerPiece(std::move(drawer));
        start_char = next_word_boundary;
        next_word_boundary = paragraph_->boundary_analyst_->FindNextBoundary(
            start_char, BoundaryType::kWord);
      }
      if (start_char < end_char) {
        auto drawer = std::make_unique<DrawerPiece>(
            run, run_range->GetParent(), start_char - run->GetStartCharPos(),
            end_char - run->GetStartCharPos());
        InsertDrawerPiece(std::move(drawer));
      }
    }
  }
}
void TextLineImpl::CreateDrawerPiece() {
  if (!drawer_list_.empty()) drawer_list_.clear();
  auto align = paragraph_->GetParagraphStyle().GetHorizontalAlign();
  for (const auto& line_range : range_lst_) {
    if (align == ParagraphHorizontalAlignment::kJustify) {
      SplitToWordDrawer(*line_range, 0);
    } else {
      for (const auto& run_range : line_range->run_range_lst_) {
        auto drawer_piece = std::make_unique<DrawerPiece>(*run_range);
        InsertDrawerPiece(std::move(drawer_piece));
      }
    }
  }
}
void TextLineImpl::InsertDrawerPiece(
    std::unique_ptr<DrawerPiece> drawer_piece) {
  auto iter = std::lower_bound(
      drawer_list_.begin(), drawer_list_.end(), drawer_piece,
      [&](const std::unique_ptr<DrawerPiece>& dpa,
          const std::unique_ptr<DrawerPiece>& dpb) {
        return dpa->GetVisualOrderIndex() <= dpb->GetVisualOrderIndex();
      });
  drawer_list_.insert(iter, std::move(drawer_piece));
}
bool TextLineImpl::StripContentByWidth(float space) {
  auto& range = range_lst_.back();
  auto range_width = range->x_max_;
  if (range_width < space) return false;
  float total_width = range->x_min_;
  for (auto& drawer : drawer_list_) {
    if (drawer->GetParent() == range.get()) {
      total_width += drawer->GetWidthWithIndent();
    }
  }
  space = space + total_width - range_width;
  if (space <= 0 || drawer_list_.empty()) {
    return true;
  }
  int32_t index = static_cast<int32_t>(drawer_list_.size()) - 1;
  while (space > 0 && index >= 0) {
    auto& piece = drawer_list_[index];
    if (space > piece->GetWidthWithIndent() || !piece->GetRun()->IsTextRun()) {
      index--;
      space -= piece->GetWidthWithIndent();
      drawer_list_.pop_back();
    } else {
      index--;
      auto end_pos =
          piece->FindCharPosInParagraphByX(piece->GetWidthWithIndent() - space);
      if (end_pos == piece->GetStartCharPosInParagraph()) {
        drawer_list_.pop_back();
      } else {
        auto* piece_run = piece->GetRun();
        auto new_piece = std::make_unique<RunRange>(
            piece_run, piece->GetParent(),
            piece->GetStartCharPosInParagraph() - piece_run->GetStartCharPos(),
            end_pos - piece_run->GetStartCharPos());
        drawer_list_.pop_back();
        drawer_list_.emplace_back(std::move(new_piece));
      }
      space = 0;
    }
  }
  return FloatsLargerOrEqual(0, space);
}

void TextLineImpl::AppendGhostRun(std::unique_ptr<BaseRun> ghost_run) {
  auto drawer_piece = std::make_unique<RunRange>(
      ghost_run.get(), range_lst_.back().get(), 0, ghost_run->GetCharCount());
  // Invalid paragraph direction for now
  WriteDirection para_dir = paragraph_->GetParagraphStyle().GetWriteDirection();
  TTASSERT(para_dir != WriteDirection::kTTB &&
           para_dir != WriteDirection::kBTT);
  if (para_dir == WriteDirection::kAuto) {
    // Follow the way of paragraph direction detection in Lynx
    if (GetParagraph()->GetCharCount() > 0 &&
        GetParagraph()->IsRtlCharacter(0)) {
      para_dir = WriteDirection::kRTL;
    } else {
      para_dir = WriteDirection::kLTR;
    }
  }
  if (para_dir == WriteDirection::kRTL) {
    std::vector<std::unique_ptr<DrawerPiece>> drawer_list;
    drawer_list.emplace_back(std::move(drawer_piece));
    drawer_list.insert(std::end(drawer_list),
                       std::make_move_iterator(std::begin(drawer_list_)),
                       std::make_move_iterator(std::end(drawer_list_)));
    drawer_list_.clear();
    drawer_list_.swap(drawer_list);
  } else {
    drawer_list_.emplace_back(std::move(drawer_piece));
  }
  extra_contents_.emplace_back(std::move(ghost_run));
}

void TextLineImpl::ModifyHorizontalAlignment(
    ParagraphHorizontalAlignment h_align) {
  if (h_align == ParagraphHorizontalAlignment::kJustify &&
      paragraph_->GetParagraphStyle().GetHorizontalAlign() !=
          ParagraphHorizontalAlignment::kJustify) {
    CreateDrawerPiece();
  }
  ApplyAlignment(h_align);
}

void TextLineImpl::ApplyAlignment() {
  ApplyAlignment(paragraph_->GetParagraphStyle().GetHorizontalAlign());
}

void TextLineImpl::ApplyAlignment(ParagraphHorizontalAlignment h_align) {
  const auto align = h_align;
  auto drawer_iter_begin = drawer_list_.begin();
  while (drawer_iter_begin != drawer_list_.end()) {
    const auto* line_range = (*drawer_iter_begin)->GetParent();
    auto drawer_iter_end = drawer_iter_begin;
    float total_width = 0;
    auto drawer_count = 0;
    while (drawer_iter_end != drawer_list_.end() &&
           ((*drawer_iter_end)->GetParent() == line_range)) {
      total_width += (*drawer_iter_end)->GetWidthWithIndent();
      ++drawer_iter_end;
      ++drawer_count;
    }
    auto start_offset = line_range->GetXMin();
    auto word_spacing = 0.f;
    float rest_space = std::max(0.f, line_range->GetRangeWidth() - total_width);
    if (align == ParagraphHorizontalAlignment::kCenter) {
      start_offset += rest_space / 2;
    } else if (align == ParagraphHorizontalAlignment::kRight) {
      start_offset += rest_space;
    } else if (align == ParagraphHorizontalAlignment::kJustify) {
      if (line_end_pos_.GetRunIdx() != paragraph_->GetRunCount() &&
          (paragraph_->boundary_analyst_->GetBoundaryTypeBefore(
               paragraph_->LayoutPositionToCharPos(line_end_pos_)) <
           BoundaryType::kMustLineBreak)) {
        auto available_space = line_range->GetRangeWidth() - total_width;
        word_spacing = available_space / static_cast<float>(drawer_count - 1);
      }
    }
    for (; drawer_iter_begin != drawer_iter_end; ++drawer_iter_begin) {
      auto& drawer = *drawer_iter_begin;
      drawer->SetXOffset(start_offset);
      auto* run = drawer->GetRun();
      auto& metrics = run->GetMetrics();
      auto cva = CharacterVerticalAlignment::kBaseLine;
      if (run->GetLayoutStyle().HasVerticalAlignment()) {
        cva = run->GetLayoutStyle().GetVerticalAlignment();
      } else if (paragraph_->GetDefaultStyle().HasVerticalAlignment()) {
        cva = paragraph_->GetDefaultStyle().GetVerticalAlignment();
      }
      auto y_offset =
          LayoutMeasurer::CalcElementY(cva, GetContentTop(), GetContentBottom(),
                                       GetContentBaseline(), metrics);
      drawer->SetYOffsetInLine(y_offset);
      if (drawer->GetRun()->GetType() == RunType::kInlineObject ||
          drawer->GetRun()->GetType() == RunType::kFloatObject) {
        auto y = GetLineTop() + y_offset;
        drawer->GetRun()->GetRunDelegate()->SetOffset(
            start_offset, y + metrics.GetMaxAscent());
      }
      start_offset += drawer->GetWidthWithIndent() + word_spacing;
    }
  }
}

void TextLineImpl::ClearForRelayout() {
  range_lst_.clear();
  empty_ = true;
  layouted_ = false;
  max_ascent_ = max_descent_ = 0;
  current_available_range_index_ = 0;
}
bool TextLineImpl::IsLastLineOfParagraph() const {
  return paragraph_->GetBoundaryTypeBefore(line_end_pos_) !=
         BoundaryType::kParagraph;
}

void TextLineImpl::StripByEllipsis(const char32_t* ellipsis) {
  auto& para_style = paragraph_->GetParagraphStyle();
  if (ellipsis == nullptr) {
    ellipsis = para_style.GetEllipsis().c_str();
  }
  auto ellipsis_len = base::U32Strlen(ellipsis);
  if ((ellipsis != nullptr && ellipsis_len > 0) ||
      para_style.GetEllipsisDelegate() != nullptr) {
    if (para_style.GetHorizontalAlign() !=
        ParagraphHorizontalAlignment::kLeft) {
      // re-generate drawer piece
      CreateDrawerPiece();
    }
    auto* shaper = paragraph_->shaper_;
    const auto font_mgr = shaper->GetFontCollection().GetDefaultFontManager();
    if (drawer_list_.empty()) return;
    const BaseRun* run = nullptr;
    for (int i = static_cast<int>(drawer_list_.size() - 1); i >= 0; --i) {
      run = drawer_list_[i]->GetRun();
      if (run->shape_result_.Valid()) {
        break;
      }
      if (!run->IsObjectRun() && !run->IsGhostRun()) {
        run = nullptr;
        TTASSERT(false);
        break;
      }
    }
    if (run == nullptr || !run->shape_result_.Valid()) {
      return;
    }
    auto font = run->shape_result_.FontByCharId(0);
    auto run_style = run->GetLayoutStyle();
    std::unique_ptr<BaseRun> ellipsis_run;
    if (ellipsis != nullptr && ellipsis_len > 0) {
      ellipsis_run = std::make_unique<GhostRun>(paragraph_, run_style, 0,
                                                ellipsis, ellipsis_len);
    } else {
      ellipsis_run = std::make_unique<ObjectRun>(
          paragraph_, para_style.GetEllipsisDelegate(), 0, 0,
          RunType::kInlineObject);
    }
    float ellipsis_width = 0;
    if (ellipsis_run->GetType() == RunType::kInlineObject) {
      ellipsis_run->Layout();
      ellipsis_width = ellipsis_run->GetWidth(0);
    } else {
      auto ellipsis_content = ellipsis_run->GetGhostContent().ToUTF32();
      ShapeStyle shape_style = run->GetShapeStyle();
      auto fd = shape_style.GetFontDescriptor();
      if (font_mgr != nullptr) {
        fd.platform_font_ = reinterpret_cast<uint64_t>(
            font_mgr->getPlatformFontFromTypeface(font));
      }
      shape_style.SetFontDescriptor(fd);
      auto result =
          shaper->ShapeText(ellipsis_content.c_str(),
                            static_cast<uint32_t>(ellipsis_content.length()),
                            &shape_style, false);
      ellipsis_run->shape_result_.InitWithShapeResult(result, 0,
                                                      run->GetCharCount());
      ellipsis_run->Layout();
      ellipsis_width = ellipsis_run->GetWidth(0);
    }
    if (StripContentByWidth(ellipsis_width)) {
      AppendGhostRun(std::move(ellipsis_run));
    }
    ApplyAlignment();
  }
}

void TextLineImpl::GetBoundingRectByCharRange(float bounding_rect[4],
                                              CharPos start_char_pos,
                                              CharPos end_char_pos) const {
  TTASSERT(!drawer_list_.empty());
  auto* left_p = bounding_rect;
  auto* top_p = bounding_rect + 1;
  auto* width_p = bounding_rect + 2;
  auto* height_p = bounding_rect + 3;
  if (drawer_list_.empty()) {
    *left_p = 0;
    *top_p = 0;
    *width_p = 0;
    *height_p = 0;
    return;
  }
  if (end_char_pos <= GetStartCharPos()) {
    *left_p = drawer_list_[0]->GetXOffset();
    *top_p = line_top_;
    *width_p = 0;
    *height_p = 0;
    return;
  }

  if (start_char_pos >= GetEndCharPos()) {
    auto& drawer = drawer_list_.back();
    *left_p = drawer->GetXOffset() + drawer->GetWidthWithIndent();
    *top_p = line_top_;
    *width_p = 0;
    *height_p = 0;
    return;
  }
  start_char_pos = std::max(start_char_pos, GetStartCharPos());
  end_char_pos = std::min(end_char_pos, GetEndCharPos());
  TTASSERT(start_char_pos <= end_char_pos);
  float left = drawer_list_[0]->GetXOffset(), top = GetLineBottom(),
        right = left, bottom = line_top_;
  enum SearchStatus : uint8_t { kNotStart, kInProgress, kEnd };
  SearchStatus status = kNotStart;
  auto iter = drawer_list_.begin();
  while (iter != drawer_list_.end()) {
    auto& drawer = *iter++;
    if (drawer->GetRun()->IsGhostRun()) {
      continue;
    }
    if (status == kNotStart) {
      if (drawer->GetStartCharPosInParagraph() <= start_char_pos &&
          start_char_pos < drawer->GetEndCharPosInParagraph()) {
        status = kInProgress;
        left = drawer->GetXOffset();
        left +=
            start_char_pos == drawer->GetStartCharPosInParagraph()
                ? 0
                : drawer->GetWidthInRange(
                      0, start_char_pos - drawer->GetStartCharPosInParagraph());
      }
    }
    if (status == kInProgress) {
      auto& metrics = drawer->GetRun()->GetMetrics();
      top = std::min(
          top, drawer->GetYOffsetInLine() + metrics.GetMaxAscent() + line_top_);
      bottom = std::max(bottom, drawer->GetYOffsetInLine() +
                                    metrics.GetMaxDescent() + line_top_);
      if (drawer->GetStartCharPosInParagraph() < end_char_pos &&
          end_char_pos <= drawer->GetEndCharPosInParagraph()) {
        right = drawer->GetXOffset() +
                drawer->GetWidthInRange(
                    0, std::min(drawer->GetCharCount(),
                                end_char_pos -
                                    drawer->GetStartCharPosInParagraph()));
        status = kEnd;
      }
    }
    if (status == kEnd) {
      break;
    }
  }
  if (status != kEnd && !drawer_list_.empty()) {
    const auto* drawer = drawer_list_.back().get();
    right =
        drawer->GetXOffset() +
        drawer->GetWidthInRange(
            0, std::min(drawer->GetCharCount(),
                        end_char_pos - drawer->GetStartCharPosInParagraph()));
  }
  *left_p = left;
  *top_p = top;
  *width_p = std::max(0.f, right - left);
  *height_p = std::max(0.f, bottom - top);
}
uint32_t TextLineImpl::GetCharPosByCoordinateX(float x) const {
  RunRange* prev_drawer = nullptr;
  float prev_right = 0;
  for (auto& drawer : drawer_list_) {
    if (drawer->GetRun()->IsGhostRun()) {
      continue;
    }
    const auto drawer_left = drawer->GetXOffset();
    const auto drawer_right = drawer_left + drawer->GetWidth();
    if (FloatsLargerOrEqual(x, drawer_left) &&
        FloatsLargerOrEqual(drawer_right, x)) {
      return drawer->FindCharPosInParagraphByX(x - drawer_left) -
             GetStartCharPos();
    }
    if (FloatsLarger(drawer_left, x)) {
      if (prev_drawer == nullptr ||
          FloatsLarger(x - prev_right, drawer->GetXOffset() - x)) {
        return drawer->GetStartCharPosInParagraph() - GetStartCharPos();
      }
      return prev_drawer->GetEndCharPosInParagraph() - GetStartCharPos();
    }
    prev_drawer = drawer.get();
    prev_right = drawer_right;
  }
  return GetCharCount();
}

void TextLineImpl::UpdateXMax(float width) {
  if (!IsLayouted() || range_lst_.empty()) return;
  range_lst_.back()->SetXMax(width);
}
TTStringPiece TextLineImpl::GetText() const {
  return paragraph_->content_.SubStr(GetStartCharPos(), GetCharCount());
}
}  // namespace tttext
}  // namespace ttoffice
