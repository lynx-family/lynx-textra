// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_LAYOUT_MEASURER_H_
#define SRC_TEXTLAYOUT_LAYOUT_MEASURER_H_

#include <memory>
#include <string>

#include "src/textlayout/run/base_run.h"
#include "src/textlayout/style_attributes.h"

namespace ttoffice {
namespace tttext {
class Painter;
class ParagraphImpl;
class FontInfo;
class RunRange;
class TextLine;
class LineRange;
class ParagraphStyle;
class LayoutDrawer;
enum class ParagraphHorizontalAlignment : uint8_t;
enum class ParagraphVerticalAlignment : uint8_t;

class LayoutMeasurer {
  friend LayoutDrawer;

 public:
  LayoutMeasurer();
  virtual ~LayoutMeasurer();

 public:
  static float CalcRunHeightByLinePitch(const Spacing& spacing,
                                        float run_height, float line_pitch);
  static void ApplyLineHAlignment(TextLine* line);
  static void ApplyLineVAlignment(TextLine* line);
  static void ApplyHorizontalAlignment(ParagraphHorizontalAlignment h_align,
                                       LineRange* range, bool last_line);
  static float GetLineSpacingRatio(const std::string& font_family);
  static float GetLinePitchSpacing(float single_line_spacing, float line_pitch,
                                   float multi);
  /**
   * Calculate element baseline offset relative to line top
   * @param v_align
   * @param line_height > 0
   * @param line_baseline > 0
   * @param element_ascent < 0
   * @param element_descent > 0
   * @return
   */
  static float CalcElementY(CharacterVerticalAlignment v_align,
                            float line_content_top, float line_content_bottom,
                            float line_baseline, const LayoutMetrics& metrics);
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_LAYOUT_MEASURER_H_
