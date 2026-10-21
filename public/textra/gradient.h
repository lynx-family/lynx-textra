// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_GRADIENT_H_
#define PUBLIC_TEXTRA_GRADIENT_H_

#include <textra/macro.h>
#include <textra/tt_color.h>

#include <cstdint>
#include <vector>

namespace ttoffice {
namespace tttext {

struct GradientPoint {
  float x{0};
  float y{0};
};

struct GradientBounds {
  float left{0};
  float top{0};
  float width{0};
  float height{0};
};

enum class GradientType : uint8_t { kLinear = 1, kRadial, kConic };
enum class GradientUnit : uint8_t { kRatio, kLength, kAngle };

struct GradientLength {
  float value{0};
  GradientUnit unit{GradientUnit::kRatio};
  float Resolve(float length) const {
    return unit == GradientUnit::kRatio ? value * length : value;
  }
};

struct GradientStop {
  TTColor color;
  float position{0};
  // Ratio: 1 = 100%; length: layout units; angle: degrees (conic only).
  GradientUnit unit{GradientUnit::kRatio};
};

enum class LinearGradientDirection : uint8_t {
  kAngle,
  kToTop,
  kToBottom,
  kToLeft,
  kToRight,
  kToTopRight,
  kToTopLeft,
  kToBottomRight,
  kToBottomLeft,
};

struct LinearGradient {
  // CSS convention: 0 points up, 90 points right.
  float angle{180};
  LinearGradientDirection direction{LinearGradientDirection::kAngle};
};

enum class RadialGradientSize : uint8_t {
  kFarthestCorner,
  kFarthestSide,
  kClosestCorner,
  kClosestSide,
  kExplicit,
};

struct RadialGradient {
  GradientLength center_x{0.5f};
  GradientLength center_y{0.5f};
  RadialGradientSize size{RadialGradientSize::kFarthestCorner};
  bool ellipse{true};
  GradientLength radius_x{0.5f};
  GradientLength radius_y{0.5f};
};

struct ConicGradient {
  GradientLength center_x{0.5f};
  GradientLength center_y{0.5f};
  float start_angle{0};
};

// Geometry uses the caller's drawing coordinate space. Colors and positions are
// normalized; drawing consumes this value without resolving its bounds.
struct L_EXPORT ResolvedGradient {
  GradientType type{GradientType::kLinear};
  GradientBounds bounds;
  GradientPoint start;
  GradientPoint end;
  GradientPoint center;
  float radius_x{0};
  float radius_y{0};
  float start_angle{0};
  std::vector<uint32_t> colors;
  std::vector<float> positions;
};

// Callers finish CSS parsing before constructing a Gradient. Stops must contain
// at least two finite, nondecreasing positions after unit conversion. Duplicate
// positions are allowed. This optional helper is independent of text styles;
// callers choose bounds and explicitly apply the ResolvedGradient.
struct L_EXPORT Gradient {
  GradientType type{GradientType::kLinear};
  LinearGradient linear;
  RadialGradient radial;
  ConicGradient conic;
  std::vector<GradientStop> stops;

  ResolvedGradient Resolve(const GradientBounds& bounds) const;
};

}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_GRADIENT_H_
