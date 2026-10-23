// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/gradient.h>

#include <algorithm>
#include <cmath>

namespace ttoffice {
namespace tttext {
namespace {
constexpr float kPi = 3.14159265358979323846f;

uint32_t Interpolate(uint32_t a, uint32_t b, float t) {
  uint32_t color = 0;
  for (int shift = 0; shift < 32; shift += 8) {
    const float ca = (a >> shift) & 255;
    const float cb = (b >> shift) & 255;
    color |= static_cast<uint32_t>(std::lround(ca + (cb - ca) * t)) << shift;
  }
  return color;
}

void NormalizeStops(ResolvedGradient* fill) {
  auto& p = fill->positions;
  auto& c = fill->colors;
  if (p.back() < 0 || p.front() > 1) {
    const auto color = p.back() < 0 ? c.back() : c.front();
    c = {color, color};
    p = {0, 1};
    return;
  }
  if (p.front() < 0) {
    const auto i = static_cast<size_t>(
        std::lower_bound(p.begin(), p.end(), 0.f) - p.begin());
    c[i - 1] = Interpolate(c[i - 1], c[i], -p[i - 1] / (p[i] - p[i - 1]));
    p[i - 1] = 0;
    c.erase(c.begin(), c.begin() + i - 1);
    p.erase(p.begin(), p.begin() + i - 1);
  }
  if (p.back() > 1) {
    const auto i = static_cast<size_t>(
        std::upper_bound(p.begin(), p.end(), 1.f) - p.begin());
    c[i] = Interpolate(c[i - 1], c[i], (1 - p[i - 1]) / (p[i] - p[i - 1]));
    p[i] = 1;
    c.resize(i + 1);
    p.resize(i + 1);
  }
  if (p.size() == 1) {
    c = {c.front(), c.front()};
    p = {0, 1};
  }
}
}  // namespace

ResolvedGradient Gradient::Resolve(const GradientBounds& bounds) const {
  TTASSERT(stops.size() >= 2);
  ResolvedGradient fill;
  fill.type = type;
  fill.bounds = bounds;
  const float w = type == GradientType::kRadial ? bounds.width
                                                : std::max(bounds.width, 1.f);
  const float h = type == GradientType::kRadial ? bounds.height
                                                : std::max(bounds.height, 1.f);
  float stop_length = 1;
  switch (type) {
    case GradientType::kLinear: {
      float angle = linear.angle * kPi / 180;
      switch (linear.direction) {
        case LinearGradientDirection::kAngle:
          break;
        case LinearGradientDirection::kToTop:
          angle = 0;
          break;
        case LinearGradientDirection::kToBottom:
          angle = kPi;
          break;
        case LinearGradientDirection::kToLeft:
          angle = -kPi / 2;
          break;
        case LinearGradientDirection::kToRight:
          angle = kPi / 2;
          break;
        case LinearGradientDirection::kToTopRight:
          angle = std::atan2(h, w);
          break;
        case LinearGradientDirection::kToTopLeft:
          angle = std::atan2(-h, w);
          break;
        case LinearGradientDirection::kToBottomRight:
          angle = std::atan2(h, -w);
          break;
        case LinearGradientDirection::kToBottomLeft:
          angle = std::atan2(-h, -w);
          break;
      }
      const float dx = std::sin(angle);
      const float dy = -std::cos(angle);
      stop_length = std::abs(dx * w) + std::abs(dy * h);
      const float cx = bounds.left + w / 2;
      const float cy = bounds.top + h / 2;
      fill.start = {cx - dx * stop_length / 2, cy - dy * stop_length / 2};
      fill.end = {cx + dx * stop_length / 2, cy + dy * stop_length / 2};
      break;
    }
    case GradientType::kRadial: {
      const float cx = radial.center_x.Resolve(w);
      const float cy = radial.center_y.Resolve(h);
      fill.center = {bounds.left + cx, bounds.top + cy};
      if (radial.size == RadialGradientSize::kExplicit) {
        fill.radius_x = radial.radius_x.Resolve(w);
        fill.radius_y =
            radial.ellipse ? radial.radius_y.Resolve(h) : fill.radius_x;
        stop_length = std::min(fill.radius_x, fill.radius_y);
        break;
      }
      const bool closest = radial.size == RadialGradientSize::kClosestSide ||
                           radial.size == RadialGradientSize::kClosestCorner;
      const bool corner = radial.size == RadialGradientSize::kClosestCorner ||
                          radial.size == RadialGradientSize::kFarthestCorner;
      const float x = closest ? std::min(std::abs(cx), std::abs(w - cx))
                              : std::max(std::abs(cx), std::abs(w - cx));
      const float y = closest ? std::min(std::abs(cy), std::abs(h - cy))
                              : std::max(std::abs(cy), std::abs(h - cy));
      stop_length = closest ? std::min(x, y) : std::max(x, y);
      if (radial.ellipse) {
        fill.radius_x = x;
        fill.radius_y = y;
        if (corner) {
          const float aspect_ratio = x / y;
          if (aspect_ratio == 0 || std::isinf(aspect_ratio) ||
              std::isnan(aspect_ratio)) {
            fill.radius_x = fill.radius_y = 0;
          } else {
            fill.radius_x =
                std::sqrt(x * x + y * y * aspect_ratio * aspect_ratio);
            fill.radius_y = fill.radius_x / aspect_ratio;
          }
          stop_length = closest ? std::min(fill.radius_x, fill.radius_y)
                                : std::max(fill.radius_x, fill.radius_y);
        }
      } else {
        if (corner) stop_length = std::hypot(x, y);
        fill.radius_x = fill.radius_y = stop_length;
      }
      break;
    }
    case GradientType::kConic:
      fill.center = {bounds.left + conic.center_x.Resolve(w),
                     bounds.top + conic.center_y.Resolve(h)};
      fill.start_angle = conic.start_angle;
      stop_length = 360;
      break;
  }
  // Native renderers handle zero radii; keep the geometry and color stops.
  // Absolute stops use the minimum native radius as their zero-length
  // reference.
  if (stop_length == 0) stop_length = 1;
  fill.colors.reserve(stops.size());
  fill.positions.reserve(stops.size());
  for (const auto& stop : stops) {
    fill.colors.push_back(stop.color);
    fill.positions.push_back(stop.unit == GradientUnit::kRatio
                                 ? stop.position
                                 : stop.position / stop_length);
    TTASSERT(std::isfinite(fill.positions.back()));
    TTASSERT(fill.positions.size() == 1 ||
             fill.positions.back() >=
                 fill.positions[fill.positions.size() - 2]);
  }
  NormalizeStops(&fill);
  return fill;
}
}  // namespace tttext
}  // namespace ttoffice
