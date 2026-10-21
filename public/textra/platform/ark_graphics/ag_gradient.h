// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_GRADIENT_H_
#define PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_GRADIENT_H_

#include <dlfcn.h>
#include <native_drawing/drawing_matrix.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_shader_effect.h>
#include <textra/gradient.h>

#include <algorithm>

namespace ttoffice {
namespace tttext {
inline OH_Drawing_ShaderEffect* MakeAGGradient(const ResolvedGradient& fill) {
  const auto count = static_cast<uint32_t>(fill.colors.size());
  if (fill.type == GradientType::kLinear) {
    auto* start = OH_Drawing_PointCreate(fill.start.x, fill.start.y);
    auto* end = OH_Drawing_PointCreate(fill.end.x, fill.end.y);
    auto* shader = OH_Drawing_ShaderEffectCreateLinearGradient(
        start, end, fill.colors.data(), fill.positions.data(), count, CLAMP);
    OH_Drawing_PointDestroy(start);
    OH_Drawing_PointDestroy(end);
    return shader;
  }
  if (fill.type == GradientType::kRadial) {
    OH_Drawing_Point2D center{fill.center.x, fill.center.y};
    const bool has_zero = fill.radius_x == 0 || fill.radius_y == 0;
    const float aspect_ratio = has_zero ? 1 : fill.radius_x / fill.radius_y;
    OH_Drawing_Matrix* matrix = nullptr;
    if (aspect_ratio != 1) {
      matrix = OH_Drawing_MatrixCreate();
      OH_Drawing_MatrixScale(matrix, 1, 1 / aspect_ratio, fill.center.x,
                             fill.center.y);
    }
    auto* shader = OH_Drawing_ShaderEffectCreateRadialGradientWithLocalMatrix(
        &center, std::max(fill.radius_x, 1.f), fill.colors.data(),
        fill.positions.data(), count, CLAMP, matrix);
    if (matrix) OH_Drawing_MatrixDestroy(matrix);
    return shader;
  }
  auto* center = OH_Drawing_PointCreate(fill.center.x, fill.center.y);
  using SweepWithLocalMatrix =
      OH_Drawing_ShaderEffect* (*)(const OH_Drawing_Point*, const uint32_t*,
                                   const float*, uint32_t, OH_Drawing_TileMode,
                                   const OH_Drawing_Matrix*);
  static const auto sweep_with_matrix = reinterpret_cast<SweepWithLocalMatrix>(
      dlsym(RTLD_DEFAULT,
            "OH_Drawing_ShaderEffectCreateSweepGradientWithLocalMatrix"));
  if (sweep_with_matrix && fill.start_angle != 90) {
    auto* matrix = OH_Drawing_MatrixCreate();
    OH_Drawing_MatrixPreRotate(matrix, fill.start_angle - 90, fill.center.x,
                               fill.center.y);
    auto* shader =
        sweep_with_matrix(center, fill.colors.data(), fill.positions.data(),
                          count, CLAMP, matrix);
    OH_Drawing_MatrixDestroy(matrix);
    OH_Drawing_PointDestroy(center);
    return shader;
  }
  // Match Lynx text: use the original sweep when no local matrix is applied.
  auto* shader = OH_Drawing_ShaderEffectCreateSweepGradient(
      center, fill.colors.data(), fill.positions.data(), count, CLAMP);
  OH_Drawing_PointDestroy(center);
  return shader;
}
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_ARK_GRAPHICS_AG_GRADIENT_H_
