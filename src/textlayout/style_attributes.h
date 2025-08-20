// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

/**
 * @brief Centralized header defining data structures for text styling
 * attributes used in Style, ParagraphStyle, and related classes.
 */

#ifndef SRC_TEXTLAYOUT_STYLE_ATTRIBUTES_H_
#define SRC_TEXTLAYOUT_STYLE_ATTRIBUTES_H_

#include <textra/layout_definition.h>

namespace ttoffice {
namespace tttext {

struct Indent {
  float start_{0};
  int32_t start_chars_{0};
  float end_{0};
  int32_t end_chars_{0};
  float first_line_{0};
  int32_t first_line_chars_{0};
  float hanging_{0};
  int32_t hanging_chars_{0};
};
struct Spacing {
  // priority:auto > line_percent > px
  /**
   * specing before or after paragraph, auto compute.
   */
  bool after_auto_spacing_{false};
  bool before_auto_spacing_{false};
  /**
   * specing before or after pararaph, in units of line pixel.
   */
  float after_px_{0};
  float before_px_{0};
  /**
   * specing before or after line, in units of line height percentage.
   */
  float after_line_percent_{0};
  float before_line_percent_{0};
  /**
   * line_height in px
   */
  float line_px_{0};
  /**
   * line_height in percent;
   */
  float line_percent_{1.f};
  /**
   * Line height mode:
   * kAuto: Multiple line height, line_percent_ takes effect
   * kAtLeast: At least, corresponds to Word's minimum value, line_px_ takes
   * effect kExact: Fixed line height, line_px_ takes effect
   */
  RulerType line_rule_{RulerType::kAuto};
  /**
   * Extra spacing before and after lines, in pixels
   */
  float line_space_before_px_{0};
  float line_space_after_px_{0};
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_STYLE_ATTRIBUTES_H_
