#ifndef EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_GRADIENT_IMPL_HPP_
#define EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_GRADIENT_IMPL_HPP_
#include <examples/capi/platform/textra_native_support.hpp>
#include <skity/effect/shader.hpp>
#include <skity/geometry/matrix.hpp>
namespace textra_native {
namespace skity_coretext_backend {
inline std::shared_ptr<skity::Shader> MakeSkityGradient(const Gradient& fill) {
  std::vector<skity::Vec4> colors;
  for (auto value : fill.colors) {
    Color c(value);
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
}  // namespace skity_coretext_backend
}  // namespace textra_native
#endif  // EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_GRADIENT_IMPL_HPP_
