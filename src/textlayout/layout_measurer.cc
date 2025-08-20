// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/layout_measurer.h"

#include <textra/layout_definition.h>
#include <textra/paragraph_style.h>
#include <textra/text_line.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "src/textlayout/internal/line_range.h"
#include "src/textlayout/internal/run_range.h"
#include "src/textlayout/run/object_run.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/style_attributes.h"
#include "src/textlayout/text_line_impl.h"
#include "src/textlayout/utils/log_util.h"

namespace ttoffice {
namespace tttext {
LayoutMeasurer::LayoutMeasurer() = default;
LayoutMeasurer::~LayoutMeasurer() = default;
float LayoutMeasurer::CalcRunHeightByLinePitch(const Spacing& spacing,
                                               float run_height,
                                               float line_pitch) {
  float desire_line_height = run_height;
  bool snap_to_grid = line_pitch > 0;
  float single_line_height = GetLineSpacingRatio("") * desire_line_height;
  switch (spacing.line_rule_) {
    case RulerType::kAtLeast:
      if (snap_to_grid) {
        single_line_height =
            GetLinePitchSpacing(single_line_height, line_pitch, 1.0f);
      }
      desire_line_height = std::max(single_line_height, spacing.line_px_);
      break;
    case RulerType::kExact:
      desire_line_height = spacing.line_px_;
      break;
    case RulerType::kAuto:
      if (snap_to_grid) {
        desire_line_height = GetLinePitchSpacing(single_line_height, line_pitch,
                                                 spacing.line_percent_);
      } else {
        desire_line_height = spacing.line_percent_ * single_line_height;
      }
      break;
  }
  return desire_line_height;
}
void LayoutMeasurer::ApplyLineHAlignment(TextLine* line) {
  auto* line_impl = TTDYNAMIC_CAST<TextLineImpl*>(line);
  auto& paragraph = *(line_impl->GetParagraph());
  const auto* paragraph_style = &paragraph.GetParagraphStyle();
  auto last_line =
      // !paragraph_style->IsLastLineFollowHorizontalAlignment() &&
      !line->IsLastLineOfParagraph();
  for (auto idx = 0u; idx < line_impl->GetRangeCount(); idx++) {
    auto* range = line_impl->GetLineRange(idx);
    ApplyHorizontalAlignment(paragraph_style->GetHorizontalAlign(), range,
                             last_line);
  }
}
void LayoutMeasurer::ApplyLineVAlignment(TextLine* line) {
  //  for (size_t idx = 0; idx < line->GetRangeCount(); idx++) {
  //    auto &range = *line->GetLineRange(idx);
  //    for (size_t ii = 0; ii < range.GetWordCount(); ii++) {
  //      auto *run_range = range.GetWord(ii);
  //      auto &layout_metrics = run_range->metrics_;
  //      if (run_range->GetType() == RunType::kFloatObject ||
  //          run_range->GetRun(0)->IsControlRun())
  //        continue;
  //      TTASSERT(run_range->Visible());
  //      // Because a run may have different vertical alignment methods
  //      internally
  //      // So vertical alignment offset is added during draw time
  //      auto y_offset = line->GetMaxAscent();
  //      run_range->SetYOffsetInLine(run_range->GetType() ==
  //      RunType::kInlineObject
  //                                      ? y_offset +
  //                                      layout_metrics.GetMaxAscent() :
  //                                      y_offset);
  //    }
  //  }
}

void LayoutMeasurer::ApplyHorizontalAlignment(
    ParagraphHorizontalAlignment h_align, LineRange* range, bool last_line) {
  float space = range->x_max_ - range->x_min_ - range->GetContentWidth();
  // Due to aligning with Android's treatment of right punctuation width as
  // half-width characters, space may be negative, no horizontal alignment
  // needed for negative cases
  if (space <= 0) {
    return;
  }
  switch (h_align) {
    case ParagraphHorizontalAlignment::kLeft:
      break;
    case ParagraphHorizontalAlignment::kCenter:
      // for (int k = 0; k < range->GetLogicWordCount(); k++) {
      //   auto *rr = range->GetLogicWord(k);
      //   rr->SetXOffset(rr->GetXOffset() + space / 2);
      // }
      break;
    case ParagraphHorizontalAlignment::kRight:
      // for (int k = 0; k < range->GetLogicWordCount(); k++) {
      //   auto *rr = range->GetLogicWord(k);
      //   rr->SetXOffset(rr->GetXOffset() + space);
      // }
      break;
    case ParagraphHorizontalAlignment::kJustify:
      if (last_line) break;
    case ParagraphHorizontalAlignment::kDistributed:
      // range->DistributeSpace(space);
      break;
  }
}
float LayoutMeasurer::GetLineSpacingRatio(const std::string& font_family) {
  // return 1.238f;
  // pptx:0.83,docx:1.17
  return 1.0f;
}
float LayoutMeasurer::GetLinePitchSpacing(float single_line_spacing,
                                          float line_pitch, float multi) {
  float desire_line_spacing = std::round(line_pitch * multi);
  while (desire_line_spacing < single_line_spacing) {
    desire_line_spacing += line_pitch;
  }
  return desire_line_spacing;
}
float LayoutMeasurer::CalcElementY(CharacterVerticalAlignment v_align,
                                   float line_content_top,
                                   float line_content_bottom,
                                   float line_baseline,
                                   const LayoutMetrics& metrics) {
  float y = 0;
  auto element_height = metrics.GetHeight();
  switch (v_align) {
    case CharacterVerticalAlignment::kBaseLine:
      y = line_baseline;
      break;
    case CharacterVerticalAlignment::kSuperScript:
    case CharacterVerticalAlignment::kSubScript:
      // TODO(hfuttyh) need to implement
      y = line_baseline;
      break;
    case CharacterVerticalAlignment::kTop:
      y = line_content_top - metrics.GetMaxAscent();
      break;
    case CharacterVerticalAlignment::kBottom:
      y = line_content_bottom - metrics.GetMaxDescent();
      break;
    case CharacterVerticalAlignment::kMiddle:
      y = line_content_top +
          (line_content_bottom - line_content_top - element_height) / 2 -
          metrics.GetMaxAscent();
      break;
  }
  return y;
}
}  // namespace tttext
}  // namespace ttoffice
