// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_SUPPORT_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_SUPPORT_HPP_
#include <textra/capi/textra_platform.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <new>
#include <vector>
namespace textra_native {
namespace skia_backend {
inline float Channel(uint32_t color, int shift) {
  return static_cast<float>((color >> shift) & 255) / 255.f;
}
constexpr uint32_t kUndefined = 0u;
enum class GradientType : uint8_t {
  kLinear = TEXTRA_GRADIENT_TYPE_LINEAR,
  kRadial = TEXTRA_GRADIENT_TYPE_RADIAL,
  kConic = TEXTRA_GRADIENT_TYPE_CONIC
};
struct Point {
  float x, y;
};
struct Bounds {
  float left, top, width, height;
};
struct Color {
  uint32_t value;
  explicit Color(uint32_t x) : value(x) {}
  float GetRedRatio() const { return Channel(value, 16); }
  float GetGreenRatio() const { return Channel(value, 8); }
  float GetBlueRatio() const { return Channel(value, 0); }
  float GetAlphaRatio() const { return Channel(value, 24); }
};
struct Gradient {
  GradientType type;
  Bounds bounds;
  Point start, end, center;
  float radius_x, radius_y, start_angle;
  std::vector<uint32_t> colors;
  std::vector<float> positions;
  explicit Gradient(const textra_backend_gradient_view_t& g)
      : type(static_cast<GradientType>(g.type)),
        bounds{g.bounds.left, g.bounds.top, g.bounds.right, g.bounds.bottom},
        start{g.start.x, g.start.y},
        end{g.end.x, g.end.y},
        center{g.center.x, g.center.y},
        radius_x(g.radius_x),
        radius_y(g.radius_y),
        start_angle(g.start_angle) {
    if (g.stop_count) {
      colors.assign(g.colors, g.colors + g.stop_count);
      positions.assign(g.positions, g.positions + g.stop_count);
    }
  }
};
/* Initialize in place. Until success, options still describe the caller's
 * native input. A non-null callback table marks an already initialized owned
 * record and prevents acquiring another reference or wrapping it twice.
 * On success the caller owns one adapter reference until adopted by an engine
 * owner. userdata and unknown trailing fields remain unchanged.
 */
inline textra_status_t InitializeManager(
    textra_font_manager_options_t* options,
    const textra_backend_font_manager_api_t& api,
    const textra_backend_typeface_api_t& face,
    textra_status_t(TEXTRA_CALL* create)(void*, void*, void**)) {
  if (!options || options->struct_size < sizeof(*options) ||
      options->manager_callbacks || options->typeface_callbacks)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  void* object = nullptr;
  auto status = create(options->userdata, options->font_manager, &object);
  if (!status && !object) status = TEXTRA_STATUS_NOT_FOUND;
  if (status) {
    if (object) api.destroy(options->userdata, object);
    return status;
  }
  options->font_manager = object;
  options->manager_callbacks = &api;
  options->typeface_callbacks = &face;
  return TEXTRA_STATUS_OK;
}
inline textra_status_t InitializeTypeface(
    textra_typeface_options_t* options, uint32_t backend,
    const textra_backend_typeface_api_t& api,
    textra_status_t(TEXTRA_CALL* import)(void*, const void*, void**)) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      options->callbacks)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  void* object = nullptr;
  auto status = import(options->userdata, options->native_font, &object);
  if (!status && !object) status = TEXTRA_STATUS_NOT_FOUND;
  if (status) {
    if (object) api.destroy(options->userdata, object);
    return status;
  }
  options->backend = backend;
  options->native_font = object;
  options->callbacks = &api;
  return TEXTRA_STATUS_OK;
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_SUPPORT_HPP_
