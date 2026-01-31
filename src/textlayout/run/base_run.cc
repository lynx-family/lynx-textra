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
LayoutMetrics BaseRun::CalculateLayoutMetrics(uint32_t start_char,
                                              uint32_t char_count,
                                              float font_size,
                                              bool align_with_bbox) const {
  LayoutMetrics layout_metrics;
  auto typeface = shape_result_.FontByCharId(start_char);

  if (align_with_bbox) {
    auto glyph_start = shape_result_.CharToGlyph(start_char);
    auto glyph_end = shape_result_.CharToGlyph(start_char + char_count);
    auto glyph_count = glyph_end - glyph_start;
    std::vector<GlyphID> glyph_data(glyph_count, 0);
    for (auto k = glyph_start; k < glyph_count; k++) {
      glyph_data[k - glyph_start] = shape_result_.Glyphs(k);
    }
    float rect_ltrb[4] = {0, 0, 0, 0};
    typeface->GetWidthBounds(rect_ltrb, glyph_data.data(),
                             static_cast<uint32_t>(glyph_data.size()),
                             font_size);
    layout_metrics.UpdateMaxAscent(rect_ltrb[1]);
    layout_metrics.UpdateMaxDescent(rect_ltrb[3]);
  } else {
    auto font_info = typeface->GetFontInfo(font_size);
    layout_metrics.UpdateMaxAscent(font_info.GetAscent());
    layout_metrics.UpdateMaxDescent(font_info.GetDescent());

    if (paragraph_->GetParagraphStyle().LineHeightOverride()) {
      if (paragraph_->GetParagraphStyle().HalfLeading()) {
        auto diff = layout_metrics.GetHeight() - font_size;
        layout_metrics.max_ascent_ += diff;
      } else {
        auto ratio = font_size / layout_metrics.GetHeight();
        layout_metrics.max_ascent_ *= ratio;
        layout_metrics.max_descent_ *= ratio;
      }
    }
  }
  return layout_metrics;
}
void BaseRun::Layout() {
  if (run_type_ == RunType::kFloatObject ||
      run_type_ == RunType::kInlineObject) {
    metrics_ = LayoutMetrics{delegate_->GetAscent(), delegate_->GetDescent()};
    scaled_metrics_ = metrics_;
  } else {
    auto align_with_bbox = paragraph_->GetParagraphStyle().EnableTextBounds();
    uint32_t start_char = 0u;
    auto current_char = start_char;
    auto end_char = GetCharCount();
    while (current_char <= end_char) {
      if (current_char == end_char ||
          shape_result_.FontByCharId(current_char) !=
              shape_result_.FontByCharId(start_char)) {
        auto metrics =
            CalculateLayoutMetrics(start_char, current_char - start_char,
                                   layout_style_.GetTextSize(), false);
        metrics_.UpdateMax(metrics);
        start_char = current_char;
      }
      current_char++;
    }
    if (FloatsEqual(layout_style_.GetTextScale(), 1.0f)) {
      if (align_with_bbox) {
        scaled_metrics_ = CalculateLayoutMetrics(
            0, GetCharCount(), layout_style_.GetTextSize(), true);
      } else {
        scaled_metrics_ = metrics_;
      }
    } else {
      scaled_metrics_ = CalculateLayoutMetrics(
          GetStartCharPos(), GetCharCount(), layout_style_.GetScaledTextSize(),
          align_with_bbox);
    }
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
