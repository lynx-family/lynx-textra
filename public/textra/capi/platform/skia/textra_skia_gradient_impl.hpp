#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_GRADIENT_IMPL_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_GRADIENT_IMPL_HPP_
#include <include/core/SkMatrix.h>
#include <include/effects/SkGradientShader.h>

#include <textra/capi/platform/skia/textra_skia_support.hpp>
namespace textra_native {
namespace skia_backend {
inline sk_sp<SkShader> MakeSkiaGradient(const Gradient& fill) {
  const auto count = static_cast<int>(fill.colors.size());
  switch (fill.type) {
    case GradientType::kLinear: {
      const SkPoint points[] = {{fill.start.x, fill.start.y},
                                {fill.end.x, fill.end.y}};
      return SkGradientShader::MakeLinear(points, fill.colors.data(),
                                          fill.positions.data(), count,
                                          SkTileMode::kClamp);
    }
    case GradientType::kRadial: {
      const bool has_zero = fill.radius_x == 0 || fill.radius_y == 0;
      const float aspect_ratio = has_zero ? 1 : fill.radius_x / fill.radius_y;
      SkMatrix matrix;
      matrix.setScale(1, 1 / aspect_ratio, fill.center.x, fill.center.y);
      return SkGradientShader::MakeRadial(
          {fill.center.x, fill.center.y}, std::max(fill.radius_x, 1.f),
          fill.colors.data(), fill.positions.data(), count, SkTileMode::kClamp,
          0, &matrix);
    }
    case GradientType::kConic: {
      SkMatrix matrix;
      matrix.setRotate(fill.start_angle - 90, fill.center.x, fill.center.y);
      return SkGradientShader::MakeSweep(
          fill.center.x, fill.center.y, fill.colors.data(),
          fill.positions.data(), count, SkTileMode::kClamp, 0, 360, 0, &matrix);
    }
  }
  return nullptr;
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_GRADIENT_IMPL_HPP_
