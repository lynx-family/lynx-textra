// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_definition.h>
#include <textra/layout_region.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <algorithm>

#include "src/textlayout/internal/line_range.h"
#include "src/textlayout/layout_measurer.h"
#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/object_run.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/text_line_impl.h"
namespace ttoffice {
namespace tttext {
LayoutRegion::LayoutRegion(float width, float height, LayoutMode width_mode,
                           LayoutMode height_mode, LayoutPageListener* listener)
    : width_mode_(width_mode),
      height_mode_(height_mode),
      line_pitch_(0),
      full_filled_(false),
      layouted_width_(0),
      layouted_bottom_(0),
      listener_(listener) {
  size_.first = std::clamp(width, LAYOUT_MIN_UNITS, LAYOUT_MAX_UNITS);
  size_.second = std::clamp(height, LAYOUT_MIN_UNITS, LAYOUT_MAX_UNITS);
}
LayoutRegion::~LayoutRegion() = default;

std::vector<std::array<float, 2>> LayoutRegion::GetRangeList(float* top,
                                                             float range_height,
                                                             float start_indent,
                                                             float end_indent) {
  std::vector<std::array<float, 2>> range_list;
  float range_start = 0 + start_indent;
  float range_end = size_.first - end_indent;
  range_list.emplace_back(std::array<float, 2>{range_start, range_end});
  *top = *top + 1;
  return range_list;
}
std::pair<LayoutResult, bool> LayoutRegion::ProcessFloatObject(
    const TTTextContext& context, const BaseRun& run, int char_x,
    float line_y) {
  return {LayoutResult::kNormal, false};
}
LayoutResult LayoutRegion::AddLine(std::unique_ptr<TextLine> line,
                                   const TTTextContext& context) {
  auto* line_impl = TTDYNAMIC_CAST<TextLineImpl*>(line.get());
  UpdateLayoutedSize(line.get(), context);
  if (paragraph_list_.empty() ||
      line_impl->GetParagraph() != paragraph_list_.back()) {
    paragraph_list_.emplace_back(line->GetParagraph());
  }
  line_lst_.emplace_back(std::move(line));
  LayoutResult result = LayoutResult::kNormal;
  if (listener_ != nullptr) {
    listener_->OnLineLayouted(this, GetLineCount() - 1,
                              line_lst_.back()->IsLastLineOfParagraph(),
                              GetPageHeight() - layouted_bottom_, &result);
  }
  return result;
}
void LayoutRegion::UpdateLayoutedSize(TextLine* line,
                                      const TTTextContext& context) {
  float line_left = line->GetLineLeft() - line->GetStartIndent();
  float line_right = line->GetLineRight() + line->GetEndIndent();
  layouted_width_ = std::max(layouted_width_, line_right - line_left);
  layouted_bottom_ = std::fmax(layouted_bottom_, line->GetLineBottom());
}
}  // namespace tttext
}  // namespace ttoffice
