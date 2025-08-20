// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/run/base_run.h"

#include <textra/layout_definition.h>
#include <textra/paragraph.h>

#include <memory>
#include <utility>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/float_comparison.h"
#include "src/textlayout/utils/tt_rectf.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

std::pair<RunType, const char*>* BaseRun::ControlRunList() {
  static std::pair<RunType, const char*> list[] = {
      {RunType::kNLFRun, base::NLF_STR()},
      {RunType::kCRLFRun, base::CRLF_STR()},
      {RunType::kCRRun, base::CR_STR()},
      {RunType::kLFRun, base::LF_STR()},
      {RunType::kTabRun, base::TAB_STR()},
      {RunType::kFFRun, base::FF_STR()}};
  return list;
}

const char** BaseRun::SpaceList() {
  static const char* list[] = {base::SPACE_STR(), base::ZWS_STR(),
                               base::NBS_STR()};
  return list;
}

float BaseRun::GetWidth(uint32_t char_start_in_run, uint32_t char_count) const {
  if (GetType() == RunType::kInlineObject ||
      GetType() == RunType::kFloatObject) {
    TTASSERT(delegate_ != nullptr);
    return delegate_->GetAdvance();
  }
  if (GetCharCount() == 0 || char_count == 0) return 0;
  TTASSERT(char_count <= GetEndCharPos() - char_start_in_run);
  TTASSERT(shape_result_.Valid());
  TTASSERT(char_start_in_run < GetCharCount() && char_count <= GetCharCount());
  auto letter_spacing = layout_style_.GetLetterSpacing();
  return shape_result_.MeasureWidth(char_start_in_run, char_count,
                                    letter_spacing);
}
float BaseRun::MeasureRunByWidth(uint32_t& break_pos_in_run,
                                 float max_width) const {
  TTASSERT(break_pos_in_run < GetCharCount());
  TTASSERT(shape_result_.Valid());
  auto width = 0.f;
  auto& idx = break_pos_in_run;
  uint32_t prev_glyph_id = -1;
  auto letter_spacing = layout_style_.GetLetterSpacing();
  while (idx < GetCharCount()) {
    TTASSERT(idx < shape_result_.CharCount());
    auto glyph_id = shape_result_.CharToGlyph(idx);
    TTASSERT(glyph_id < shape_result_.GlyphCount());
    if (glyph_id != prev_glyph_id) {
      const auto ww = shape_result_.Advances(glyph_id)[0] + letter_spacing;
      if (FloatsLarger(width + ww, max_width)) break;
      width += ww;
    }
    idx++;
    prev_glyph_id = glyph_id;
  }
  return width;
}
void BaseRun::Layout() {
  if (run_type_ == RunType::kFloatObject ||
      run_type_ == RunType::kInlineObject) {
    metrics_ = LayoutMetrics{delegate_->GetAscent(), delegate_->GetDescent()};
    return;
  }
  if (!IsTextRun() && !IsControlRun() && !IsGhostRun()) return;
  auto typeface = shape_result_.FontByCharId(0);
  FontInfo base_font_info = typeface->GetFontInfo(layout_style_.GetTextSize());
  for (auto k = 1u; k < GetCharCount(); k++) {
    auto new_typeface = shape_result_.FontByCharId(k);
    if (new_typeface != typeface) {
      auto info = new_typeface->GetFontInfo(layout_style_.GetTextSize());
      if (FloatsLarger(-info.GetAscent(), -base_font_info.GetAscent())) {
        base_font_info.SetAscent(info.GetAscent());
      }
      if (FloatsLarger(info.GetDescent(), base_font_info.GetDescent())) {
        base_font_info.SetDescent(info.GetDescent());
      }
      if (FloatsLarger(-info.GetLeading(), -base_font_info.GetLeading())) {
        base_font_info.SetLeading(info.GetLeading());
      }
      typeface = new_typeface;
    }
  }
  auto v_align = layout_style_.GetVerticalAlignment();
  auto line_height_override =
      paragraph_->GetParagraphStyle().LineHeightOverride();
  auto align_with_bbox = paragraph_->GetParagraphStyle().EnableTextBounds();

  if (v_align == CharacterVerticalAlignment::kSuperScript ||
      v_align == CharacterVerticalAlignment::kSubScript) {
    constexpr float offset_percent = 0.33f;
    auto font_info = typeface->GetFontInfo(layout_style_.GetScaledTextSize());
    auto mid_line = base_font_info.GetHeight() *
                    (0.5f + (v_align == CharacterVerticalAlignment::kSuperScript
                                 ? -offset_percent
                                 : offset_percent));
    auto baseline_offset =
        mid_line + (-font_info.GetAscent() - font_info.GetHeight() / 2) +
        base_font_info.GetAscent();
    paragraph_->style_manager_->SetBaselineOffset(
        baseline_offset, GetStartCharPos(), GetCharCount());
    metrics_.UpdateMax(font_info);
  } else {
    metrics_.max_ascent_ = base_font_info.GetAscent();
    metrics_.max_descent_ = base_font_info.GetDescent();
    if (line_height_override) {
      auto half_leading = paragraph_->GetParagraphStyle().HalfLeading();
      if (half_leading) {
        auto diff = base_font_info.GetHeight() - layout_style_.GetTextSize();
        metrics_.max_ascent_ += diff;
      } else {
        auto ratio = layout_style_.GetTextSize() / base_font_info.GetHeight();
        metrics_.max_ascent_ *= ratio;
        metrics_.max_descent_ *= ratio;
      }
    }
  }

  if (align_with_bbox) {
    TypefaceRef font = nullptr;
    RectF bounds;
    std::vector<GlyphID> glyphs;
    glyphs.reserve(GetCharCount());
    for (auto k = 0u; k < GetCharCount(); k++) {
      auto cur_font = shape_result_.FontByCharId(k);
      if (cur_font != font) {
        if (!glyphs.empty()) {
          float rect_ltrb[4] = {0, 0, 0, 0};
          cur_font->GetWidthBounds(rect_ltrb, glyphs.data(),
                                   static_cast<uint32_t>(glyphs.size()),
                                   layout_style_.GetTextSize());
          glyphs.clear();
          if (rect_ltrb[1] < bounds.GetTop()) bounds.SetTop(rect_ltrb[1]);
          if (rect_ltrb[3] > bounds.GetBottom()) bounds.SetBottom(rect_ltrb[3]);
        }
        font = cur_font;
      }
      glyphs.push_back(shape_result_.Glyphs(k));
    }
    if (!glyphs.empty()) {
      float rect_ltrb[4] = {0, 0, 0, 0};
      font->GetWidthBounds(rect_ltrb, glyphs.data(),
                           static_cast<uint32_t>(glyphs.size()),
                           layout_style_.GetTextSize());
      glyphs.clear();
      if (rect_ltrb[1] < bounds.GetTop()) bounds.SetTop(rect_ltrb[1]);
      if (rect_ltrb[3] > bounds.GetBottom()) bounds.SetBottom(rect_ltrb[3]);
    }
    auto diff = metrics_.GetHeight() - bounds.GetHeight();
    baseline_offset_ = metrics_.GetMaxAscent() + diff / 2 - bounds.GetTop();
  }
}
void BaseRun::UpdateRunContent() {
#ifdef TTTEXT_DEBUG
  if (!IsGhostRun() && GetCharCount() > 0) {
    run_content_ =
        paragraph_->GetContent(GetStartCharPos(), GetCharCount()).ToUTF32();
  }
#endif
}
}  // namespace tttext
}  // namespace ttoffice
