// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_TEXT_LAYOUT_IMPL_H_
#define SRC_TEXTLAYOUT_TEXT_LAYOUT_IMPL_H_

#include <textra/layout_definition.h>

#include <memory>

#include "src/textlayout/internal/line_range.h"
#include "src/textlayout/run/base_run.h"

namespace ttoffice {
namespace tttext {
class Paragraph;
class ParagraphImpl;
class LayoutRegion;
class TTTextContext;
class TTShaper;
class TextLine;
class TextLineImpl;
class LayoutPosition;
class TextLayoutImpl {
 public:
  static LayoutResult LayoutEx(Paragraph* para, LayoutRegion* page,
                               TTTextContext& context, TTShaper* shaper);

  static std::unique_ptr<TextLineImpl> ProcessNewLine(ParagraphImpl* para,
                                                      LayoutRegion* page,
                                                      TTTextContext& context);

  static float ProcessLineGap(LayoutRegion* page, TextLine* line,
                              TTTextContext& context);

  static void FinishLineLayout(LayoutRegion* page,
                               std::unique_ptr<TextLineImpl> line,
                               TTTextContext& context, LayoutResult* result);

  static LayoutPosition ProcessSpecialRun(const ParagraphImpl& paragraph,
                                          const LayoutPosition& position,
                                          LayoutRegion* page, TextLine* line,
                                          TTTextContext& context,
                                          LayoutResult* result);

  static LayoutPosition ProcessBreakableRunList(const ParagraphImpl& paragraph,
                                                const LayoutPosition& position,
                                                LayoutRegion* page,
                                                TextLineImpl* line,
                                                TTTextContext& context,
                                                LayoutResult* result);

  static bool CheckLineNeedRelayout(LayoutRegion* region, TextLineImpl* line,
                                    float new_height, float& next_line_top);

  static LayoutPosition AddBreakableRunsToLine(const ParagraphImpl& paragraph,
                                               const LayoutPosition& position,
                                               TextLineImpl* line,
                                               LayoutRegion* region,
                                               TTTextContext& context,
                                               LayoutResult* result);

  static float TryAddRun(LayoutMetrics line_metrics, const BaseRun* run);

  static float AddWordListToRunRange(LineRange* range,
                                     const ParagraphImpl& paragraph,
                                     const LayoutPosition& start_pos,
                                     const LayoutPosition& end_pos,
                                     LayoutMetrics* metrics);

  static LayoutPosition FindBreakPosInWord(const ParagraphImpl& paragraph,
                                           const LayoutPosition& position,
                                           float* max_width,
                                           LayoutResult* result);

  static LayoutPosition BreakWordForWidth(
      const ParagraphImpl& paragraph, const LayoutPosition& position,
      const LayoutPosition& greedy_break_pos, float max_width,
      OverflowWrap overflow_wrap = OverflowWrap::kBreakWord);

  static LayoutPosition ForceBreakWord(const ParagraphImpl& paragraph,
                                       const LayoutPosition& position,
                                       const LayoutPosition& greedy_break_pos,
                                       OverflowWrap overflow_wrap);
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_TEXT_LAYOUT_IMPL_H_
