// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_RUN_GHOST_RUN_H_
#define SRC_TEXTLAYOUT_RUN_GHOST_RUN_H_

#include "src/textlayout/run/base_run.h"
namespace ttoffice {
namespace tttext {
class TTShaper;
class GhostRun : public BaseRun {
  friend ParagraphImpl;

 public:
  GhostRun(ParagraphImpl* paragraph, const Style& style, uint32_t start_char,
           const char32_t* content, uint32_t length)
      : BaseRun(paragraph, style, start_char, start_char + length,
                RunType::kGhostRun) {
    auto* shaper = paragraph->shaper_;
    boundary_type_ = BoundaryType::kLineBreakable;
    // need ensure the attribute of layout style has default value
    ghost_content_ = TTString{content, length};
    if (!layout_style_.HasFontDescriptor()) {
      layout_style_.SetFontDescriptor(
          Style::DefaultStyle().GetFontDescriptor());
    }
    if (!layout_style_.HasTextSize()) {
      layout_style_.SetTextSize(Style::DefaultStyle().GetTextSize());
    }
    if (!layout_style_.HasTextScale()) {
      layout_style_.SetTextScale(Style::DefaultStyle().GetTextScale());
    }
    if (!layout_style_.HasForegroundColor()) {
      layout_style_.SetForegroundColor(
          Style::DefaultStyle().GetForegroundColor());
    }
    auto result = shaper->ShapeText(content, length, &GetShapeStyle(), false);
    shape_result_.InitWithShapeResult(result, 0, length);
  }
  ~GhostRun() override = default;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_RUN_GHOST_RUN_H_
