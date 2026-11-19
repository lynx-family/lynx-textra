// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/capi/textra_internal.h"

namespace textra_capi {
Delegate::~Delegate() {
  if (adopted && callbacks.destroy) callbacks.destroy(userdata);
}
textra_status_t Delegate::Measure() {
  textra_run_metrics_t value{sizeof(value), 0, 0, 0};
  auto status = callbacks.measure(userdata, &value);
  if (status) return status;
  if (!Finite(value.ascent) || value.ascent > 0 ||
      !Nonnegative(value.descent) || !Nonnegative(value.advance))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  metrics = value;
  return TEXTRA_STATUS_OK;
}
void Delegate::Draw(tt::ICanvasHelper*, float x, float y) {
  if (!operation || operation->error) return;
  if (!callbacks.draw) {
    Fail(TEXTRA_STATUS_UNSUPPORTED);
    return;
  }
  Fail(callbacks.draw(userdata, operation->canvas, x, y));
}

namespace {
struct Ranges {
  std::vector<std::array<float, 2>> values;
  textra_status_t error = 0;
};
textra_status_t TEXTRA_CALL WriteRanges(void* data,
                                        const textra_horizontal_range_t* ranges,
                                        uint32_t count) {
  auto* state = static_cast<Ranges*>(data);
  auto status = [&]() -> textra_status_t {
    if (state->error) return state->error;
    if (!ranges && count) return TEXTRA_STATUS_INVALID_ARGUMENT;
    for (uint32_t i = 0; i < count; ++i) {
      auto r = ranges[i];
      if (!Finite(r.start) || !Finite(r.end) || r.start >= r.end ||
          (!state->values.empty() && r.start < state->values.back()[1]))
        return TEXTRA_STATUS_INVALID_ARGUMENT;
      state->values.push_back({r.start, r.end});
    }
    return TEXTRA_STATUS_OK;
  }();
  if (status) state->error = status;
  return status;
}
}  // namespace
std::vector<std::array<float, 2>> Region::GetRangeList(float* top, float height,
                                                       float start, float end) {
  const float left = start, right = GetPageWidth() - end;
  if (!owner->callbacks.get_ranges || (operation && operation->error))
    return {{left, std::max(left + 0.001f, right)}};
  const float original = *top;
  Ranges ranges;
  auto status = owner->callbacks.get_ranges(owner->userdata, top, height, left,
                                            right, WriteRanges, &ranges);
  if (!status) status = ranges.error;
  if (!status && (!Finite(*top) || *top < original))
    status = TEXTRA_STATUS_INVALID_ARGUMENT;
  if (status) {
    Fail(status);
    *top = original;
    return {{left, std::max(left + 0.001f, right)}};
  }
  if (ranges.values.empty()) {
    // The legacy engine requires at least one interval. Move the line beyond
    // this region rather than handing it an empty vector and dereferencing it.
    *top = std::max(*top, GetPageHeight() + std::max(1.0f, height));
    return {{left, std::max(left + 0.001f, right)}};
  }
  return std::move(ranges.values);
}
void PageListener::OnLineLayouted(tt::LayoutRegion*, uint32_t index, bool last,
                                  float height, tt::LayoutResult* result) {
  if (operation && operation->error) {
    *result = tt::LayoutResult::kBreakPage;
    return;
  }
  if (!owner->callbacks.on_line) return;
  textra_line_layout_event_t event{sizeof(event), index,
                                   static_cast<textra_bool_t>(last), height};
  textra_layout_flow_t flow = TEXTRA_LAYOUT_FLOW_NORMAL;
  auto status = owner->callbacks.on_line(owner->userdata, owner, &event, &flow);
  if (!status && flow > TEXTRA_LAYOUT_FLOW_BREAK_LINE)
    status = TEXTRA_STATUS_UNSUPPORTED;
  if (status) {
    Fail(status);
    *result = tt::LayoutResult::kBreakPage;
    return;
  }
  *result = static_cast<tt::LayoutResult>(flow);
}
uint32_t DrawerListener::FetchThemeColor(tt::ThemeCategory category,
                                         const tt::TTColor& color) {
  if (!owner->callbacks.resolve_color || (operation && operation->error))
    return color.GetPlainColor();
  const auto mapped = static_cast<textra_theme_category_t>(category);
  uint32_t result = color.GetPlainColor();
  Fail(owner->callbacks.resolve_color(owner->userdata, mapped,
                                      color.GetPlainColor(), &result));
  return result;
}
}  // namespace textra_capi
