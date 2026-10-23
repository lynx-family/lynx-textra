// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Cocoa/Cocoa.h>
#include <gtest/gtest.h>
#include <textra/platform/mac/mac_canvas_base.h>

using namespace ttoffice::tttext;

namespace {
class GradientCoreGraphicsTest : public testing::Test {
 protected:
  void SetUp() override {
    auto space = CGColorSpaceCreateDeviceRGB();
    context_ = CGBitmapContextCreate(pixels_, 160, 100, 8, 160 * 4, space,
                                     kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);
    CGContextTranslateCTM(context_, 0, 100);
    CGContextScaleCTM(context_, 1, -1);
  }
  void TearDown() override { CGContextRelease(context_); }
  const uint8_t* Pixel(size_t x, size_t y) const {
    return pixels_ + (y * 160 + x) * 4;
  }
  Gradient MakeGradient(GradientType type) {
    Gradient gradient;
    gradient.type = type;
    gradient.linear.angle = 90;
    gradient.stops = {{TTColor::RED, 0}, {TTColor::BLUE, 1}};
    return gradient;
  }
  CGContextRef context_;
  uint8_t pixels_[160 * 100 * 4]{};
};
}  // namespace

TEST_F(GradientCoreGraphicsTest, LinearUsesRegionCoordinates) {
  auto gradient = MakeGradient(GradientType::kLinear);
  DrawCoreGraphicsGradient(context_, gradient.Resolve({10, 10, 140, 80}));
  EXPECT_GT(Pixel(20, 50)[0], Pixel(20, 50)[2]);
  EXPECT_GT(Pixel(140, 50)[2], Pixel(140, 50)[0]);
}

TEST_F(GradientCoreGraphicsTest,
       LinearVerticalDirectionUsesTopLeftCoordinates) {
  auto gradient = MakeGradient(GradientType::kLinear);
  gradient.linear.angle = 180;
  DrawCoreGraphicsGradient(context_, gradient.Resolve({0, 0, 160, 100}));
  EXPECT_GT(Pixel(80, 10)[0], 220);
  EXPECT_GT(Pixel(80, 90)[2], 220);
}

TEST_F(GradientCoreGraphicsTest, EllipseUsesResolvedCenterAndRadii) {
  auto gradient = MakeGradient(GradientType::kRadial);
  gradient.radial.size = RadialGradientSize::kExplicit;
  gradient.radial.radius_x = {60, GradientUnit::kLength};
  gradient.radial.radius_y = {30, GradientUnit::kLength};
  DrawCoreGraphicsGradient(context_, gradient.Resolve({0, 0, 160, 100}));
  EXPECT_GT(Pixel(80, 50)[0], 230);
  EXPECT_GT(Pixel(139, 50)[2], 230);
  EXPECT_GT(Pixel(80, 79)[2], 230);
}

TEST_F(GradientCoreGraphicsTest, ConicStartsAtTopAndPreservesAlpha) {
  auto gradient = MakeGradient(GradientType::kConic);
  gradient.stops[0].color = 0x80ff0000;
  gradient.stops[1].color = 0x800000ff;
  DrawCoreGraphicsGradient(context_, gradient.Resolve({0, 0, 160, 100}));
  auto top = Pixel(81, 15);
  EXPECT_GT(top[0], top[2]);
  EXPECT_NEAR(top[3], 128, 1);
  auto left = Pixel(30, 50);
  EXPECT_GT(left[2], left[0]);
}

TEST_F(GradientCoreGraphicsTest, ConicCoversGlyphsOutsideReferenceBounds) {
  auto gradient = MakeGradient(GradientType::kConic);
  DrawCoreGraphicsGradient(context_, gradient.Resolve({40, 30, 80, 40}));
  EXPECT_NEAR(Pixel(10, 50)[3], 255, 1);
  EXPECT_NEAR(Pixel(150, 50)[3], 255, 1);
}

TEST_F(GradientCoreGraphicsTest, ConicFallbackPreservesColorAndAlpha) {
  auto gradient = MakeGradient(GradientType::kConic);
  gradient.stops[0].color = 0x40ff0000;
  auto space = CGColorSpaceCreateDeviceRGB();
  DrawCoreGraphicsConicGradientFallback(
      context_, gradient.Resolve({0, 0, 160, 100}), space);
  CGColorSpaceRelease(space);
  auto color = Pixel(80, 80);
  EXPECT_NEAR(color[0], color[2], 3);
  EXPECT_NEAR(color[3], 160, 2);
}

TEST_F(GradientCoreGraphicsTest, ConicMatchesLynxNativeDrawing) {
  auto gradient = MakeGradient(GradientType::kConic);
  gradient.conic.start_angle = 75;
  gradient.stops = {{0x40ff0000u, 0}, {TTColor::BLUE, 1}};
  DrawCoreGraphicsGradient(context_, gradient.Resolve({10, 20, 100, 60}));

  uint8_t reference_pixels[sizeof(pixels_)]{};
  auto space = CGColorSpaceCreateDeviceRGB();
  auto reference = CGBitmapContextCreate(reference_pixels, 160, 100, 8, 160 * 4,
                                         space, kCGImageAlphaPremultipliedLast);
  CGContextTranslateCTM(reference, 0, 100);
  CGContextScaleCTM(reference, 1, -1);
  const CGFloat red[] = {1, 0, 0, 64.0 / 255};
  const CGFloat blue[] = {0, 0, 1, 1};
  const CGColorRef colors[] = {CGColorCreate(space, red),
                               CGColorCreate(space, blue)};
  const void* color_values[] = {colors[0], colors[1]};
  auto color_array = CFArrayCreate(kCFAllocatorDefault, color_values, 2,
                                   &kCFTypeArrayCallBacks);
  const CGFloat positions[] = {0, 1};
  auto native = CGGradientCreateWithColors(space, color_array, positions);
  CGContextDrawConicGradient(reference, native, CGPointMake(60, 50),
                             (75 - 90) * M_PI / 180);
  int maximum_error = 0;
  for (size_t i = 0; i < sizeof(pixels_); ++i) {
    maximum_error = std::max(
        maximum_error, std::abs(int(pixels_[i]) - int(reference_pixels[i])));
  }
  EXPECT_LE(maximum_error, 1);
  CGGradientRelease(native);
  CFRelease(color_array);
  CGColorRelease(colors[0]);
  CGColorRelease(colors[1]);
  CGContextRelease(reference);
  CGColorSpaceRelease(space);
}

TEST_F(GradientCoreGraphicsTest, GradientPreservesSyntheticBoldCoverage) {
  auto font = CTFontCreateWithName(CFSTR("Helvetica"), 64, nullptr);
  TypefaceCoreText typeface(font, 1);
  const UniChar character = 'H';
  CGGlyph glyph;
  ASSERT_TRUE(CTFontGetGlyphsForCharacters(font, &character, &glyph, 1));
  CFRelease(font);
  float x = 0, y = 0;
  auto gradient = MakeGradient(GradientType::kLinear);
  Painter painter;
  painter.SetFillColor(TTColor::WHITE);
  painter.SetTextSize(64);
  painter.SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient.Resolve({0, 0, 160, 100})));
  IOSCanvasBase canvas(context_);
  auto coverage = [&] {
    uint32_t sum = 0;
    for (size_t i = 3; i < sizeof(pixels_); i += 4) sum += pixels_[i];
    return sum;
  };
  canvas.DrawGlyphs(&typeface, 1, &glyph, nullptr, 0, 15, 75, &x, &y, &painter);
  const auto regular = coverage();
  std::fill(std::begin(pixels_), std::end(pixels_), 0);
  painter.SetBold(true);
  canvas.DrawGlyphs(&typeface, 1, &glyph, nullptr, 0, 15, 75, &x, &y, &painter);
  EXPECT_GT(coverage(), regular * 1.1f);
}

TEST_F(GradientCoreGraphicsTest,
       SyntheticBoldItalicPreservesOffsetGlyphPositions) {
  auto font = CTFontCreateWithName(CFSTR("Helvetica"), 64, nullptr);
  TypefaceCoreText typeface(font, 1);
  const UniChar characters[2]{'H', 'O'};
  CGGlyph glyphs[2];
  ASSERT_TRUE(CTFontGetGlyphsForCharacters(font, characters, glyphs, 2));
  CFRelease(font);
  float x[2]{0, 65};
  auto gradient = MakeGradient(GradientType::kLinear);
  auto fill = std::make_shared<const ResolvedGradient>(
      gradient.Resolve({0, 0, 160, 100}));
  Painter painter;
  painter.SetFillColor(TTColor::WHITE);
  painter.SetTextSize(64);
  painter.SetBold(true);
  painter.SetItalic(true);
  IOSCanvasBase canvas(context_);
  for (float offset : {10.f, -10.f}) {
    SCOPED_TRACE(offset);
    float y[2]{0, offset};
    painter.SetFillGradient(nullptr);
    std::fill(std::begin(pixels_), std::end(pixels_), 0);
    canvas.DrawGlyphs(&typeface, 2, glyphs, nullptr, 0, 15, 75, x, y, &painter);
    std::vector<uint8_t> solid(std::begin(pixels_), std::end(pixels_));
    painter.SetFillGradient(fill);
    std::fill(std::begin(pixels_), std::end(pixels_), 0);
    canvas.DrawGlyphs(&typeface, 2, glyphs, nullptr, 0, 15, 75, x, y, &painter);
    uint32_t coverage = 0;
    int maximum_alpha_error = 0;
    for (size_t i = 3; i < sizeof(pixels_); i += 4) {
      coverage += solid[i];
      maximum_alpha_error = std::max(maximum_alpha_error,
                                     std::abs(int(pixels_[i]) - int(solid[i])));
    }
    EXPECT_GT(coverage, 0u);
    // Allow antialiasing differences between native text and the clip mask.
    EXPECT_LE(maximum_alpha_error, 20);
  }
}

TEST_F(GradientCoreGraphicsTest, GlyphFillAndSolidStrokeUseIndependentPaints) {
  auto font = CTFontCreateWithName(CFSTR("Helvetica"), 64, nullptr);
  TypefaceCoreText typeface(font, 1);
  const UniChar character = 'H';
  CGGlyph glyph;
  ASSERT_TRUE(CTFontGetGlyphsForCharacters(font, &character, &glyph, 1));
  CFRelease(font);
  uint16_t glyphs[2]{glyph, glyph};
  float x[2]{0, 90};
  float y[2]{0, 0};
  auto gradient = MakeGradient(GradientType::kLinear);
  Painter painter;
  painter.SetFillColor(TTColor::WHITE);
  painter.SetTextSize(64);
  painter.SetStrokeColor(TTColor::GREEN);
  painter.SetStrokeWidth(2);
  painter.SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient.Resolve({0, 0, 160, 100})));
  IOSCanvasBase canvas(context_);
  canvas.DrawGlyphs(&typeface, 2, glyphs, nullptr, 0, 15, 75, x, y, &painter);
  int red = 0, blue = 0, green = 0;
  for (size_t i = 0; i < sizeof(pixels_); i += 4) {
    if (pixels_[i + 3] < 128) continue;
    if (pixels_[i] > pixels_[i + 2] + 30) ++red;
    if (pixels_[i + 2] > pixels_[i] + 30) ++blue;
    if (pixels_[i + 1] > 200) ++green;
  }
  EXPECT_GT(red, 100);
  EXPECT_GT(blue, 100);
  EXPECT_GT(green, 100);
}
