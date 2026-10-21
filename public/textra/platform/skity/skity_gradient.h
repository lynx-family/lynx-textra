// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_GRADIENT_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_GRADIENT_H_

#include <textra/gradient.h>

#include <algorithm>
#include <skity/effect/shader.hpp>
#include <skity/geometry/matrix.hpp>

namespace ttoffice {
namespace tttext {
inline std::shared_ptr<skity::Shader> MakeSkityGradient(
    const ResolvedGradient& fill) {
  std::vector<skity::Vec4> colors;
  for (auto value : fill.colors) {
    TTColor c(value);
    colors.push_back({c.GetRedRatio(), c.GetGreenRatio(), c.GetBlueRatio(),
                      c.GetAlphaRatio()});
  }
  const auto count = static_cast<int>(colors.size());
  switch (fill.type) {
    case GradientType::kLinear: {
      const skity::Point points[2] = {{fill.start.x, fill.start.y, 0, 1},
                                      {fill.end.x, fill.end.y, 0, 1}};
      return skity::Shader::MakeLinear(points, colors.data(),
                                       fill.positions.data(), count);
    }
    case GradientType::kRadial: {
      const bool has_zero = fill.radius_x == 0 || fill.radius_y == 0;
      const float aspect_ratio = has_zero ? 1 : fill.radius_x / fill.radius_y;
      auto shader = skity::Shader::MakeRadial(
          {fill.center.x, fill.center.y, 0, 1}, std::max(fill.radius_x, 1.f),
          colors.data(), fill.positions.data(), count);
      if (aspect_ratio != 1) {
        shader->SetLocalMatrix(
            skity::Matrix::Translate(fill.center.x, fill.center.y) *
            skity::Matrix::Scale(1, 1 / aspect_ratio) *
            skity::Matrix::Translate(-fill.center.x, -fill.center.y));
      }
      return shader;
    }
    case GradientType::kConic: {
      auto shader = skity::Shader::MakeSweep(
          fill.center.x, fill.center.y, 0, 360, colors.data(),
          fill.positions.data(), count, skity::TileMode::kClamp, 0);
      shader->SetLocalMatrix(skity::Matrix::RotateDeg(
          fill.start_angle - 90, skity::Vec2(fill.center.x, fill.center.y)));
      return shader;
    }
  }
  return nullptr;
}
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_GRADIENT_H_
