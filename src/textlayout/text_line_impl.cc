// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/text_line_impl.h"

#include <textra/layout_region.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <utility>

#include "src/textlayout/dominate_baseline.h"
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
namespace {
CharPos GetVisualLeftCharPos(const DrawerPiece* drawer) {
  return drawer->GetRun()->IsRtl() ? drawer->GetEndCharPosInParagraph()
                                   : drawer->GetStartCharPosInParagraph();
}

CharPos GetVisualRightCharPos(const DrawerPiece* drawer) {
  return drawer->GetRun()->IsRtl() ? drawer->GetStartCharPosInParagraph()
                                   : drawer->GetEndCharPosInParagraph();
}
}  // namespace

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

void TextLineImpl::AdjustInlineObjectLineBox() {
  if (paragraph_->GetParagraphStyleImpl().GetInlineVerticalAlignmentMode() !=
      InlineVerticalAlignmentMode::kLineBox) {
    return;
  }
  const float baseline = GetContentBaseline();
  float top_extension = 0.f;
  float bottom_extension = 0.f;
  float max_top_object_height = 0.f;
  float max_bottom_object_height = 0.f;
  for (const auto& range : range_lst_) {
    for (const auto& run_range : range->run_range_lst_) {
      const auto* run = run_range->GetRun();
      if (!run->IsObjectRun()) {
        continue;
      }
      const auto alignment =
          LayoutMeasurer::ResolveCharacterVerticalAlignment(run);
      const auto metrics = run->GetScaledMetrics();
      const float object_height = metrics.GetHeight();
      if (alignment == CharacterVerticalAlignment::kTop) {
        max_top_object_height = std::max(max_top_object_height, object_height);
        continue;
      }
      if (alignment == CharacterVerticalAlignment::kBottom) {
        max_bottom_object_height =
            std::max(max_bottom_object_height, object_height);
        continue;
      }
      const float offset = LayoutMeasurer::CalcInlineObjectBaselineOffset(
          run, max_ascent_, max_descent_);
      top_extension = std::max(top_extension,
                               -(baseline + offset + metrics.GetMaxAscent()));
      bottom_extension = std::max(
          bottom_extension,
          baseline + offset + metrics.GetMaxDescent() - GetLineHeight());
    }
  }
  top_extra_ += std::max(0.f, top_extension);
  bottom_extra_ += std::max(0.f, bottom_extension);

  const float line_height = GetLineHeight();
  const float top_object_extension =
      std::max(0.f, max_top_object_height - line_height);
  const float bottom_object_extension =
      std::max(0.f, max_bottom_object_height - line_height);
  if (top_object_extension >= bottom_object_extension) {
    bottom_extra_ += top_object_extension;
  } else {
    top_extra_ += bottom_object_extension;
  }

  for (const auto& range : range_lst_) {
    for (const auto& run_range : range->run_range_lst_) {
      const auto* run = run_range->GetRun();
      if (!run->IsObjectRun()) {
        continue;
      }
      const auto alignment =
          LayoutMeasurer::ResolveCharacterVerticalAlignment(run);
      const bool align_to_line =
          alignment == CharacterVerticalAlignment::kTop ||
          alignment == CharacterVerticalAlignment::kBottom;
      const float line_baseline = GetContentBaseline();
      run_range->SetYOffsetInLine(
          line_baseline +
          LayoutMeasurer::CalcInlineObjectBaselineOffset(
              run, align_to_line ? line_baseline : max_ascent_,
              align_to_line ? GetLineHeight() - line_baseline : max_descent_));
    }
  }
  range_height_floor_ = std::max(range_height_floor_, GetLineHeight());
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
void TextLineImpl::SplitToWordDrawer(const LineRange& line_range) {
  for (const auto& run_range : line_range.run_range_lst_) {
    if (run_range->GetRun()->IsGhostRun()) {
      drawer_list_.push_back(std::make_unique<DrawerPiece>(*run_range));
    } else {
      auto* run = run_range->GetRun();
      auto start_char = run_range->GetStartCharPosInParagraph();
      auto end_char = run_range->GetEndCharPosInParagraph();
      auto next_justify_opportunity =
          paragraph_->FindNextJustifyOpportunity(start_char);
      while (next_justify_opportunity < end_char) {
        auto drawer = std::make_unique<DrawerPiece>(
            run, run_range->GetParent(), start_char - run->GetStartCharPos(),
            next_justify_opportunity - run->GetStartCharPos());
        drawer->SetYOffsetInLine(run_range->GetYOffsetInLine());
        InsertDrawerPiece(std::move(drawer));
        start_char = next_justify_opportunity;
        next_justify_opportunity =
            paragraph_->FindNextJustifyOpportunity(start_char);
      }
      if (start_char < end_char) {
        auto drawer = std::make_unique<DrawerPiece>(
            run, run_range->GetParent(), start_char - run->GetStartCharPos(),
            end_char - run->GetStartCharPos());
        drawer->SetYOffsetInLine(run_range->GetYOffsetInLine());
        InsertDrawerPiece(std::move(drawer));
      }
    }
  }
}
void TextLineImpl::CreateDrawerPiece() {
  if (!drawer_list_.empty()) drawer_list_.clear();
  auto align = paragraph_->GetParagraphStyleImpl().GetHorizontalAlign();
  for (const auto& line_range : range_lst_) {
    if (align == ParagraphHorizontalAlignment::kJustify) {
      SplitToWordDrawer(*line_range);
    } else {
      for (const auto& run_range : line_range->run_range_lst_) {
        auto drawer_piece = std::make_unique<DrawerPiece>(*run_range);
        InsertDrawerPiece(std::move(drawer_piece));
      }
    }
  }
}

void TextLineImpl::TrimTailSpace() {
  auto last = drawer_list_.rbegin();
  while (last != drawer_list_.rend()) {
    auto& drawer = *last;
    if (drawer->GetRun()->IsGhostRun()) {
      break;
    }
    auto start_char_index = drawer->GetStartCharPosInParagraph();
    auto last_char_index = drawer->GetEndCharPosInParagraph();
    auto content = paragraph_->GetContent();
    while (last_char_index > start_char_index) {
      if (!base::IsSpaceChar(content.GetUnicode(last_char_index - 1))) {
        break;
      }
      drawer->RemoveLastChar(1);
      last_char_index--;
    }
    if (last_char_index == start_char_index) {
      TTASSERT(drawer->GetStartCharPosInParagraph() ==
               drawer->GetEndCharPosInParagraph());
      ++last;
      continue;
    }
    break;
  }
  if (last != drawer_list_.rend()) {
    drawer_list_.erase(last.base(), drawer_list_.end());
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
  auto range_width = range->GetRangeWidth();
  if (range_width < space) return false;
  float total_width = 0;
  for (auto& drawer : drawer_list_) {
    if (drawer->GetParent() == range.get()) {
      total_width += drawer->GetWidthWithIndent();
    }
  }
  space = space + total_width - range_width;
  if (space <= 0 || drawer_list_.empty()) {
    return true;
  }
  const bool strip_from_start =
      paragraph_->GetResolvedWriteDirection() == WriteDirection::kRTL;
  size_t first_piece = 0;
  size_t last_piece = drawer_list_.size();
  uint32_t stripped_char_count = 0;
  while (space > 0 && first_piece < last_piece) {
    const size_t index = strip_from_start ? first_piece : last_piece - 1;
    auto& piece = drawer_list_[index];
    if (space > piece->GetWidthWithIndent() || !piece->GetRun()->IsTextRun()) {
      space -= piece->GetWidthWithIndent();
      if (!piece->GetRun()->IsGhostRun()) {
        stripped_char_count += piece->GetCharCount();
      }
      if (strip_from_start) {
        ++first_piece;
      } else {
        --last_piece;
      }
    } else {
      const auto piece_end_pos = piece->GetEndCharPosInParagraph();
      auto end_pos =
          piece->FindCharPosInParagraphByX(piece->GetWidthWithIndent() - space);
      stripped_char_count += piece_end_pos - end_pos;
      if (end_pos == piece->GetStartCharPosInParagraph()) {
        if (strip_from_start) {
          ++first_piece;
        } else {
          --last_piece;
        }
      } else {
        auto* piece_run = piece->GetRun();
        auto new_piece = std::make_unique<RunRange>(
            piece_run, piece->GetParent(),
            piece->GetStartCharPosInParagraph() - piece_run->GetStartCharPos(),
            end_pos - piece_run->GetStartCharPos());
        drawer_list_[index] = std::move(new_piece);
      }
      space = 0;
    }
  }
  if (strip_from_start) {
    drawer_list_.erase(drawer_list_.begin(),
                       drawer_list_.begin() + first_piece);
  } else {
    drawer_list_.erase(drawer_list_.begin() + last_piece, drawer_list_.end());
  }
  const bool strip_succeeded = FloatsLargerOrEqual(0, space);
  if (strip_succeeded) {
    const auto original_end_pos = GetEndCharPos();
    TTASSERT(stripped_char_count <= original_end_pos - GetStartCharPos());
    line_end_pos_ = paragraph_->CharPosToLayoutPosition(original_end_pos -
                                                        stripped_char_count);
  }
  return strip_succeeded;
}

void TextLineImpl::AppendGhostRun(std::unique_ptr<BaseRun> ghost_run) {
  auto drawer_piece = std::make_unique<RunRange>(
      ghost_run.get(), range_lst_.back().get(), 0, ghost_run->GetCharCount());
  WriteDirection para_dir = paragraph_->GetResolvedWriteDirection();
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
      paragraph_->GetParagraphStyleImpl().GetHorizontalAlign() !=
          ParagraphHorizontalAlignment::kJustify) {
    CreateDrawerPiece();
  }
  ApplyAlignment(h_align);
}

void TextLineImpl::ApplyAlignment() {
  ApplyAlignment(paragraph_->GetResolvedHorizontalAlignment());
}

void TextLineImpl::ApplyAlignment(ParagraphHorizontalAlignment h_align) {
  const auto align = paragraph_->ResolveHorizontalAlignment(h_align);
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
    auto line_container_ascent = GetLineBaseLine() - GetLineTop();
    auto line_container_descent = GetLineBottom() - GetLineBaseLine();
    for (; drawer_iter_begin != drawer_iter_end; ++drawer_iter_begin) {
      auto container_ascent = line_container_ascent;
      auto container_descent = line_container_descent;
      auto& drawer = *drawer_iter_begin;
      drawer->SetXOffset(start_offset);
      auto* run = drawer->GetRun();
      auto metrics = run->GetScaledMetrics();
      auto y_offset = GetContentBaseline();
      const bool use_line_box_vertical_alignment =
          run->IsObjectRun() && !run->IsGhostRun() &&
          paragraph_->GetParagraphStyleImpl()
                  .GetInlineVerticalAlignmentMode() ==
              InlineVerticalAlignmentMode::kLineBox;
      if (use_line_box_vertical_alignment) {
        y_offset = drawer->GetYOffsetInLine();
      } else {
        auto cva = CharacterVerticalAlignment::kBaseLine;
        if (run->GetLayoutStyle().HasStyleAttribute(
                Style::VerticalAlignmentFlag)) {
          cva = run->GetLayoutStyle().GetVerticalAlignment();
        } else if (paragraph_->GetDefaultStyleImpl().HasStyleAttribute(
                       Style::VerticalAlignmentFlag)) {
          cva = paragraph_->GetDefaultStyleImpl().GetVerticalAlignment();
        }
        auto element_ascent = metrics.GetMaxAscent();
        auto element_descent = metrics.GetMaxDescent();
        if (cva == CharacterVerticalAlignment::kTextTop ||
            cva == CharacterVerticalAlignment::kTextBottom) {
          container_ascent = GetContentBaseline() - GetContentTop();
          container_descent = GetContentBottom() - GetContentBaseline();
        }
        if (cva != CharacterVerticalAlignment::kBaseLine) {
          y_offset += LayoutMeasurer::CalcElementY(
              cva, container_ascent, container_descent, element_ascent,
              element_descent);
          if (cva == CharacterVerticalAlignment::kSuperScript) {
            auto base_metrics = run->GetMetrics();
            y_offset += base_metrics.GetSupOffset();
          }
          if (cva == CharacterVerticalAlignment::kSubScript) {
            auto base_metrics = run->GetMetrics();
            y_offset += base_metrics.GetSubOffset();
          }
        }
        if (drawer->GetRun()->GetType() == RunType::kInlineObject ||
            drawer->GetRun()->GetType() == RunType::kFloatObject) {
          auto paragraph = run->GetParagraph();
          if (paragraph) {
            auto& style_manager = paragraph->style_manager_;
            auto base_line_offset = style_manager->GetBaselineOffset(
                drawer->GetRun()->GetStartCharPos());
            y_offset += base_line_offset;
          }
        }
      }
      if (drawer->GetRun()->GetType() == RunType::kInlineObject ||
          drawer->GetRun()->GetType() == RunType::kFloatObject) {
        auto y = GetLineTop() + y_offset;
        drawer->GetRun()->GetRunDelegate()->SetOffset(
            start_offset, y + metrics.GetMaxAscent());
      }
      drawer->SetYOffsetInLine(y_offset);
      start_offset += drawer->GetWidthWithIndent() + word_spacing;
    }
  }
}

void TextLineImpl::ApplyDominateBaseline() {
  DominateBaselineHelper::ApplyDominateBaseline(this);
}

void TextLineImpl::ClearForRelayout() {
  range_lst_.clear();
  empty_ = true;
  layouted_ = false;
  max_ascent_ = max_descent_ = 0;
  top_extra_ = bottom_extra_ = 0;
  current_available_range_index_ = 0;
}
bool TextLineImpl::IsLastLineOfParagraph() const {
  return paragraph_->GetBoundaryTypeBefore(line_end_pos_) !=
         BoundaryType::kParagraph;
}

void TextLineImpl::StripByEllipsis(const char32_t* ellipsis) {
  auto& para_style = paragraph_->GetParagraphStyleImpl();
  if (ellipsis == nullptr) {
    ellipsis = para_style.GetEllipsis().c_str();
  }
  auto ellipsis_len = base::U32Strlen(ellipsis);
  if ((ellipsis != nullptr && ellipsis_len > 0) ||
      para_style.GetEllipsisDelegate() != nullptr) {
    if (paragraph_->GetResolvedHorizontalAlignment() !=
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
  if (drawer_list_.empty() || start_char_pos >= end_char_pos) {
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
  auto bounds = MergeRectsToBounds(
      GetCharRangeBounds(start_char_pos, end_char_pos, false));
  *left_p = bounds.GetLeft();
  *top_p = bounds.GetTop();
  *width_p = std::max(0.f, bounds.GetWidth());
  *height_p = std::max(0.f, bounds.GetHeight());
}

RectF TextLineImpl::MergeRectsToBounds(const std::vector<RectF>& rects) {
  if (rects.empty()) {
    return RectF::MakeEmpty();
  }
  float top = rects.front().GetTop();
  float bottom = rects.front().GetBottom();
  for (const auto& rect : rects) {
    top = std::min(top, rect.GetTop());
    bottom = std::max(bottom, rect.GetBottom());
  }
  return RectF::MakeLTRB(rects.front().GetLeft(), top, rects.back().GetRight(),
                         bottom);
}

std::vector<RectF> TextLineImpl::GetCharRangeBounds(CharPos start_char_pos,
                                                    CharPos end_char_pos,
                                                    bool tight) const {
  float y_base = tight ? 0.f : line_top_;
  std::vector<RectF> bounding_rects;
  float current_left{0}, current_top{0}, current_right{0}, current_bottom{0};
  bool bounds_available{false};
  float tight_left{0}, tight_right{0}, tight_ascent{0}, tight_descent{0};
  for (auto& drawer : drawer_list_) {
    if (drawer->GetRun()->IsGhostRun()) {
      continue;
    }
    auto drawer_start = drawer->GetStartCharPosInParagraph();
    auto drawer_end = drawer->GetEndCharPosInParagraph();
    auto range = Range::Intersect(Range::MakeLR(start_char_pos, end_char_pos),
                                  Range::MakeLR(drawer_start, drawer_end));
    if (range.Empty()) {
      if (bounds_available) {
        bounding_rects.emplace_back(RectF::MakeLTRB(
            current_left, current_top, current_right, current_bottom));
      }
      bounds_available = false;
      continue;
    }
    bool rtl = drawer->GetRun()->IsRtl();
    float drawer_top, drawer_bottom;

    if (tight) {
      GetDrawerPieceCharRangeTightAscentAndDescent(drawer.get(), start_char_pos,
                                                   end_char_pos, &tight_ascent,
                                                   &tight_descent);
      drawer_top = y_base + drawer->GetYOffsetInLine() + tight_ascent;
      drawer_bottom = y_base + drawer->GetYOffsetInLine() + tight_descent;
    } else {
      auto metrics = drawer->GetRun()->GetMetrics();
      drawer_top = y_base + drawer->GetYOffsetInLine() + metrics.GetMaxAscent();
      drawer_bottom =
          y_base + drawer->GetYOffsetInLine() + metrics.GetMaxDescent();
    }

    bool drawer_start_included = drawer_start == range.GetStart();
    bool drawer_end_included = drawer_end == range.GetEnd();
    bool left_reached = rtl ? drawer_end_included : drawer_start_included;
    bool right_reached = rtl ? drawer_start_included : drawer_end_included;

    uint32_t left_char_index = rtl ? range.GetEnd() : range.GetStart();
    uint32_t right_char_index = rtl ? range.GetStart() : range.GetEnd();

    if (!left_reached || !bounds_available) {
      if (bounds_available) {
        bounding_rects.emplace_back(RectF::MakeLTRB(
            current_left, current_top, current_right, current_bottom));
      }
      float left_width =
          (left_char_index == drawer_start)
              ? 0
              : drawer->GetWidthInRange(0, left_char_index - drawer_start);
      current_left = drawer->GetXOffset() +
                     (rtl ? (drawer->GetWidth() - left_width) : left_width);
      if (tight) {
        int char_offset = rtl ? -1 : 0;
        GetDrawerPieceCharTightLeftAndRight(drawer.get(),
                                            left_char_index + char_offset,
                                            &tight_left, &tight_right);
        current_left +=
            tight_left + drawer->GetRun()->GetPunctuationDrawOffset();
      }
      current_top = drawer_top;
      current_bottom = drawer_bottom;
    } else if (bounds_available) {
      current_top = std::min(current_top, drawer_top);
      current_bottom = std::max(current_bottom, drawer_bottom);
    }
    float right_width =
        (right_char_index == drawer_start)
            ? 0
            : drawer->GetWidthInRange(0, right_char_index - drawer_start);
    current_right = drawer->GetXOffset() +
                    (rtl ? (drawer->GetWidth() - right_width) : right_width);
    if (tight) {
      int char_offset = rtl ? 0 : -1;
      GetDrawerPieceCharTightLeftAndRight(drawer.get(),
                                          right_char_index + char_offset,
                                          &tight_left, &tight_right);

      float right_char_width = drawer->GetWidthInRange(
          right_char_index - drawer_start + char_offset, 1);
      current_right += tight_right - right_char_width +
                       drawer->GetRun()->GetPunctuationDrawOffset();
    }
    if (!right_reached) {
      bounding_rects.emplace_back(RectF::MakeLTRB(
          current_left, current_top, current_right, current_bottom));
    }
    bounds_available = right_reached;
  }
  if (bounds_available) {
    bounding_rects.emplace_back(RectF::MakeLTRB(current_left, current_top,
                                                current_right, current_bottom));
  }
  return bounding_rects;
}

void TextLineImpl::GetTightBoundingRectByCharRange(float bounding_rect[4],
                                                   CharPos start_char_pos,
                                                   CharPos end_char_pos) const {
  if (drawer_list_.empty() || start_char_pos >= end_char_pos ||
      start_char_pos >= GetEndCharPos() || end_char_pos <= GetStartCharPos()) {
    bounding_rect[0] = 0;
    bounding_rect[1] = 0;
    bounding_rect[2] = 0;
    bounding_rect[3] = 0;
    return;
  }
  start_char_pos = std::max(start_char_pos, GetStartCharPos());
  end_char_pos = std::min(end_char_pos, GetEndCharPos());
  TTASSERT(start_char_pos <= end_char_pos);
  auto bounds = MergeRectsToBounds(
      GetCharRangeBounds(start_char_pos, end_char_pos, true));
  bounding_rect[0] = bounds.GetLeft();
  bounding_rect[1] = bounds.GetTop() - max_ascent_ - top_extra_;
  bounding_rect[2] = bounds.GetRight();
  bounding_rect[3] = bounds.GetBottom() - max_ascent_ - top_extra_;
}

void TextLineImpl::GetDrawerPieceCharTightLeftAndRight(DrawerPiece* piece,
                                                       CharPos index,
                                                       float* left,
                                                       float* right) {
  auto* piece_run = piece->GetRun();
  index -= piece_run->GetStartCharPos();
  if (index >= piece_run->GetCharCount()) {
    *left = 0;
    *right = 0;
    return;
  }
  auto& shape_result = piece_run->shape_result_;
  auto glyph_index = shape_result.CharToGlyph(index);
  float font_size = piece_run->GetLayoutStyle().GetScaledTextSize();
  GlyphID glyph = shape_result.Glyphs(glyph_index);
  TypefaceRef font = shape_result.Font(glyph_index);
  float ltwh[4];
  font->GetWidthBound(ltwh, glyph, font_size);
  auto info = font->GetFontInfo(font_size);
  float skew_extra =
      piece_run->GetSkewExtraWidth() * ltwh[1] / info.GetAscent();
  *left = ltwh[0] - std::max(0.f, skew_extra);
  *right = ltwh[0] + ltwh[2] + std::max(0.f, -skew_extra);
}

void TextLineImpl::GetDrawerPieceCharRangeTightAscentAndDescent(
    DrawerPiece* piece, CharPos start, CharPos end, float* ascent,
    float* descent) {
  auto* piece_run = piece->GetRun();
  auto piece_char_start = std::max(piece->GetStartCharPosInParagraph(), start) -
                          piece_run->GetStartCharPos();
  auto piece_char_end = std::min(piece->GetEndCharPosInParagraph(), end) -
                        piece_run->GetStartCharPos();
  auto& shape_result = piece_run->shape_result_;
  auto char_count = shape_result.CharCount();
  auto glyph_count = shape_result.GlyphCount();
  piece_char_start = std::min(piece_char_start, char_count);
  piece_char_end = std::min(piece_char_end, char_count);
  auto glyph_start = shape_result.CharToGlyph(piece_char_start);
  auto glyph_end = std::max(shape_result.CharToGlyph(piece_char_end - 1) + 1,
                            shape_result.CharToGlyph(piece_char_end));
  glyph_start = std::min(glyph_start, glyph_count);
  glyph_end = std::min(glyph_end, glyph_count);
  if (glyph_start >= glyph_end) {
    *ascent = 0;
    *descent = 0;
    return;
  }
  float font_size = piece_run->GetLayoutStyle().GetScaledTextSize();
  TypefaceRef last_font = shape_result.Font(glyph_start);
  std::vector<GlyphID> glyphs;
  glyphs.emplace_back(shape_result.Glyphs(glyph_start));
  float current_ascent = std::numeric_limits<float>::max();
  float current_descent = -std::numeric_limits<float>::max();
  float content_bound[4];
  for (auto index = glyph_start + 1; index < glyph_end; index++) {
    auto& current_font = shape_result.Font(index);
    if (current_font != last_font) {
      last_font->GetWidthBounds(content_bound, glyphs.data(),
                                static_cast<uint32_t>(glyphs.size()),
                                font_size);
      current_ascent = std::min(current_ascent, content_bound[1]);
      current_descent = std::max(current_descent, content_bound[3]);
      last_font = current_font;
      glyphs.clear();
    }
    glyphs.emplace_back(shape_result.Glyphs(index));
  }
  if (!glyphs.empty()) {
    last_font->GetWidthBounds(content_bound, glyphs.data(),
                              static_cast<uint32_t>(glyphs.size()), font_size);
    current_ascent = std::min(current_ascent, content_bound[1]);
    current_descent = std::max(current_descent, content_bound[3]);
  }
  *ascent = current_ascent;
  *descent = current_descent;
}

uint32_t TextLineImpl::GetCharPosByCoordinateX(float x) const {
  RunRange* prev_drawer = nullptr;
  float prev_right = 0;
  for (auto& drawer : drawer_list_) {
    if (drawer->GetRun()->IsGhostRun()) {
      continue;
    }
    const auto drawer_left = drawer->GetXOffset();
    const auto drawer_width = drawer->GetWidth();
    const auto drawer_right = drawer_left + drawer_width;
    if (FloatsLargerOrEqual(x, drawer_left) &&
        FloatsLargerOrEqual(drawer_right, x)) {
      auto local_x = x - drawer_left;
      if (drawer->GetRun()->IsRtl()) {
        local_x = drawer_width - local_x;
      }
      return drawer->FindCharPosInParagraphByX(local_x) - GetStartCharPos();
    }
    if (FloatsLarger(drawer_left, x)) {
      if (prev_drawer == nullptr ||
          FloatsLarger(x - prev_right, drawer->GetXOffset() - x)) {
        return GetVisualLeftCharPos(drawer.get()) - GetStartCharPos();
      }
      return GetVisualRightCharPos(prev_drawer) - GetStartCharPos();
    }
    prev_drawer = drawer.get();
    prev_right = drawer_right;
  }
  if (prev_drawer != nullptr) {
    return GetVisualRightCharPos(prev_drawer) - GetStartCharPos();
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
