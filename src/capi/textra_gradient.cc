// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_internal.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL textra_resolved_gradient_create(
    const textra_resolved_gradient_options_t* options,
    textra_resolved_gradient_t** out_gradient) {
  if (!out_gradient) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_gradient = nullptr;
  if (!ValidStruct(options)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto type = TEXTRA_READ(options, type,
                          textra_gradient_type_t{TEXTRA_GRADIENT_TYPE_LINEAR});
  if (type < TEXTRA_GRADIENT_TYPE_LINEAR || type > TEXTRA_GRADIENT_TYPE_CONIC)
    return TEXTRA_STATUS_UNSUPPORTED;
  auto stops = TEXTRA_READ(options, stops,
                           static_cast<const textra_gradient_stop_t*>(nullptr));
  auto count = TEXTRA_READ(options, stop_count, uint32_t{0});
  if (!stops || count < 2) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto value = std::make_shared<tt::ResolvedGradient>();
  value->type = static_cast<tt::GradientType>(type);
  auto start = TEXTRA_READ(options, start, textra_point_t{});
  auto end = TEXTRA_READ(options, end, textra_point_t{});
  auto center = TEXTRA_READ(options, center, textra_point_t{});
  value->start = {start.x, start.y};
  value->end = {end.x, end.y};
  value->center = {center.x, center.y};
  value->radius_x = TEXTRA_READ(options, radius_x, 0.0f);
  value->radius_y = TEXTRA_READ(options, radius_y, 0.0f);
  value->start_angle = TEXTRA_READ(options, start_angle, 0.0f);
  if (!Finite(start.x) || !Finite(start.y) || !Finite(end.x) ||
      !Finite(end.y) || !Finite(center.x) || !Finite(center.y) ||
      !Nonnegative(value->radius_x) || !Nonnegative(value->radius_y) ||
      !Finite(value->start_angle))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (type == TEXTRA_GRADIENT_TYPE_RADIAL &&
      (value->radius_x == 0 || value->radius_y == 0))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (type == TEXTRA_GRADIENT_TYPE_LINEAR && start.x == end.x &&
      start.y == end.y)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  for (uint32_t i = 0; i < count; ++i) {
    if (!Finite(stops[i].position) || stops[i].position < 0 ||
        stops[i].position > 1 ||
        (i && stops[i].position < stops[i - 1].position))
      return TEXTRA_STATUS_INVALID_ARGUMENT;
    value->colors.push_back(stops[i].argb);
    value->positions.push_back(stops[i].position);
  }
  auto result = std::make_unique<textra_resolved_gradient>();
  result->impl = std::move(value);
  *out_gradient = result.release();
  return TEXTRA_STATUS_OK;
}
