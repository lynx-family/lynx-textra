// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_PLATFORM_IOS_COREGRAPHICS_GRADIENT_H_
#define PUBLIC_TEXTRA_PLATFORM_IOS_COREGRAPHICS_GRADIENT_H_

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <textra/gradient.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace ttoffice {
namespace tttext {
inline void ClipGradientGlyphs(CGContextRef context, CTFontRef font,
                               const CGGlyph* glyphs, const CGPoint* positions,
                               size_t count, CGAffineTransform text_matrix,
                               float bold_width) {
  if (bold_width == 0) {
    CGContextSetTextDrawingMode(context, kCGTextClip);
    CTFontDrawGlyphs(font, glyphs, positions, count, context);
    return;
  }
  auto path = CGPathCreateMutable();
  for (size_t i = 0; i < count; ++i) {
    auto glyph = CTFontCreatePathForGlyph(font, glyphs[i], nullptr);
    // Spaces have advance but no outline.
    if (!glyph) continue;
    // Transform the glyph origin as well as its outline, like CTFontDrawGlyphs.
    auto transform = CGAffineTransformConcat(
        CGAffineTransformMakeTranslation(positions[i].x, positions[i].y),
        text_matrix);
    CGPathAddPath(path, &transform, glyph);
    CGPathRelease(glyph);
  }
  auto stroke = CGPathCreateCopyByStrokingPath(
      path, nullptr, bold_width, kCGLineCapButt, kCGLineJoinMiter, 10);
  CGContextAddPath(context, path);
  CGContextAddPath(context, stroke);
  CGContextClip(context);
  CGPathRelease(stroke);
  CGPathRelease(path);
}

// Compatibility for systems predating the native conic-gradient API.
inline void DrawCoreGraphicsConicGradientFallback(CGContextRef context,
                                                  const ResolvedGradient& fill,
                                                  CGColorSpaceRef color_space) {
  // Glyph overhang and ellipsis can extend beyond the reference bounds.
  // Sample the clipped drawing area without changing the gradient geometry.
  const auto rect = CGContextGetClipBoundingBox(context);
  const auto matrix = CGContextGetCTM(context);
  const double scale =
      std::max(std::hypot(matrix.a, matrix.b), std::hypot(matrix.c, matrix.d));
  const size_t width = static_cast<size_t>(std::ceil(rect.size.width * scale));
  const size_t height =
      static_cast<size_t>(std::ceil(rect.size.height * scale));
  if (width == 0 || height == 0) {
    return;
  }
  std::vector<uint8_t> pixels(width * height * 4);
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      const float dx = rect.origin.x + (x + 0.5f) / scale - fill.center.x;
      const float dy = rect.origin.y + (y + 0.5f) / scale - fill.center.y;
      float angle = std::atan2(dy, dx) * 180 / 3.14159265358979323846f + 90 -
                    fill.start_angle;
      angle = std::fmod(std::fmod(angle, 360.f) + 360.f, 360.f);
      const float position = angle / 360;
      auto next = std::upper_bound(fill.positions.begin(), fill.positions.end(),
                                   position);
      const size_t hi = static_cast<size_t>(next - fill.positions.begin());
      uint32_t color;
      if (hi == 0)
        color = fill.colors.front();
      else if (hi == fill.colors.size())
        color = fill.colors.back();
      else {
        const float t = (position - fill.positions[hi - 1]) /
                        (fill.positions[hi] - fill.positions[hi - 1]);
        color = 0;
        for (int shift = 0; shift < 32; shift += 8) {
          const float a = (fill.colors[hi - 1] >> shift) & 255;
          const float b = (fill.colors[hi] >> shift) & 255;
          color |= static_cast<uint32_t>(std::lround(a + (b - a) * t)) << shift;
        }
      }
      const size_t i = (y * width + x) * 4;
      const uint32_t alpha = color >> 24;
      pixels[i] = static_cast<uint8_t>(((color >> 16) & 255) * alpha / 255);
      pixels[i + 1] = static_cast<uint8_t>(((color >> 8) & 255) * alpha / 255);
      pixels[i + 2] = static_cast<uint8_t>((color & 255) * alpha / 255);
      pixels[i + 3] = static_cast<uint8_t>(alpha);
    }
  }
  auto bitmap =
      CGBitmapContextCreate(pixels.data(), width, height, 8, width * 4,
                            color_space, kCGImageAlphaPremultipliedLast);
  auto image = CGBitmapContextCreateImage(bitmap);
  CGContextSaveGState(context);
  CGContextTranslateCTM(context, rect.origin.x,
                        rect.origin.y + rect.size.height);
  CGContextScaleCTM(context, 1, -1);
  CGContextDrawImage(
      context, CGRectMake(0, 0, rect.size.width, rect.size.height), image);
  CGContextRestoreGState(context);
  CGImageRelease(image);
  CGContextRelease(bitmap);
}

// The caller establishes the glyph clipping path in region-local coordinates.
inline void DrawCoreGraphicsGradient(CGContextRef context,
                                     const ResolvedGradient& fill) {
  auto color_space = CGColorSpaceCreateDeviceRGB();
  std::vector<CGFloat> colors;
  std::vector<CGFloat> locations(fill.positions.begin(), fill.positions.end());
  for (auto value : fill.colors) {
    TTColor color(value);
    colors.insert(colors.end(), {color.GetRedRatio(), color.GetGreenRatio(),
                                 color.GetBlueRatio(), color.GetAlphaRatio()});
  }
  auto gradient = CGGradientCreateWithColorComponents(
      color_space, colors.data(), locations.data(), locations.size());
  const auto options =
      kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation;
  if (fill.type == GradientType::kLinear) {
    CGContextDrawLinearGradient(context, gradient,
                                CGPointMake(fill.start.x, fill.start.y),
                                CGPointMake(fill.end.x, fill.end.y), options);
  } else if (fill.type == GradientType::kConic) {
    if (__builtin_available(macOS 14.0, iOS 17.0, *)) {
      CGContextDrawConicGradient(
          context, gradient, CGPointMake(fill.center.x, fill.center.y),
          (fill.start_angle - 90) * 3.14159265358979323846 / 180);
    } else {
      DrawCoreGraphicsConicGradientFallback(context, fill, color_space);
    }
  } else {
    const bool has_zero = fill.radius_x == 0 || fill.radius_y == 0;
    const float aspect_ratio = has_zero ? 1 : fill.radius_x / fill.radius_y;
    CGContextSaveGState(context);
    if (aspect_ratio != 1) {
      CGContextTranslateCTM(context, fill.center.x, fill.center.y);
      CGContextScaleCTM(context, 1, 1 / aspect_ratio);
      CGContextTranslateCTM(context, -fill.center.x, -fill.center.y);
    }
    const auto center = CGPointMake(fill.center.x, fill.center.y);
    CGContextDrawRadialGradient(context, gradient, center, 0, center,
                                has_zero ? 0 : fill.radius_x, options);
    CGContextRestoreGState(context);
  }
  CGGradientRelease(gradient);
  CGColorSpaceRelease(color_space);
}
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_IOS_COREGRAPHICS_GRADIENT_H_
