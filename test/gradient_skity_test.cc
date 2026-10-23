// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/platform/skity/skity_canvas_helper.h>

#include <skity/graphic/bitmap.hpp>

using namespace ttoffice::tttext;

namespace {
class GradientSkityTest : public testing::Test {
 protected:
  GradientSkityTest()
      : bitmap_(160, 100),
        canvas_(skity::Canvas::MakeSoftwareCanvas(&bitmap_)),
        helper_(canvas_.get()),
        painter_(helper_.CreatePainter()) {
    canvas_->DrawColor(0);
    gradient_.linear.angle = 90;
    gradient_.stops = {{TTColor::RED, 0}, {TTColor::BLUE, 1}};
    painter_->SetFillColor(TTColor::WHITE);
  }
  void Fill() {
    painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
        gradient_.Resolve({0, 0, 160, 100})));
    helper_.DrawRect(0, 0, 160, 100, painter_.get());
    canvas_->Flush();
  }
  TTColor Pixel(uint32_t x, uint32_t y) { return bitmap_.GetPixel(x, y); }
  skity::Bitmap bitmap_;
  std::unique_ptr<skity::Canvas> canvas_;
  SkityCanvasHelper helper_;
  std::unique_ptr<Painter> painter_;
  Gradient gradient_;
};
}  // namespace

TEST_F(GradientSkityTest, LinearAndFollowingSolidDoNotShareShader) {
  Fill();
  EXPECT_GT(Pixel(20, 50).GetRed(), Pixel(20, 50).GetBlue());
  EXPECT_GT(Pixel(140, 50).GetBlue(), Pixel(140, 50).GetRed());
  painter_->SetFillGradient(nullptr);
  painter_->SetFillColor(TTColor::GREEN);
  helper_.DrawRect(0, 0, 40, 100, painter_.get());
  canvas_->Flush();
  EXPECT_EQ(Pixel(20, 50), TTColor::GREEN);
}

TEST_F(GradientSkityTest, RectFillAndSolidStrokeUseIndependentPaints) {
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  painter_->SetStrokeColor(TTColor::GREEN);
  painter_->SetStrokeWidth(4);
  helper_.DrawRect(10, 10, 150, 90, painter_.get());
  canvas_->Flush();
  EXPECT_EQ(Pixel(10, 50), TTColor::GREEN);
  EXPECT_GT(Pixel(20, 50).GetRed(), Pixel(20, 50).GetBlue());
  EXPECT_GT(Pixel(140, 50).GetBlue(), Pixel(140, 50).GetRed());
}

TEST_F(GradientSkityTest, FillGradientDoesNotAffectLineStroke) {
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  painter_->SetStrokeWidth(4);
  for (uint32_t color : {TTColor::GREEN, 0x8000ff00u, TTColor::UNDEFINED}) {
    SCOPED_TRACE(color);
    canvas_->Clear(0);
    painter_->SetStrokeColor(color);
    helper_.DrawLine(10, 50, 150, 50, painter_.get());
    canvas_->Flush();
    EXPECT_EQ(Pixel(20, 50), color);
    EXPECT_EQ(Pixel(140, 50), color);
  }
}

TEST_F(GradientSkityTest, CustomNativePaintKeepsPriority) {
  auto font = skity::Typeface::MakeFromFile(
      FONT_ROOT "Inter/Inter-VariableFont_opsz,wght.ttf");
  ASSERT_TRUE(font);
  skity::textlayout::SkityTypefaceHelper typeface(font);
  uint16_t glyph = font->UnicharToGlyph('H');
  float x = 0, y = 0;
  painter_->SetTextSize(64);
  painter_->SetStrokeColor(TTColor::GREEN);
  painter_->SetStrokeWidth(4);
  painter_->SetShadowList({{TTColor::GREEN, {0, 10}, 2}});
  auto fill = std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100}));
  auto* painter = static_cast<SkityPainter*>(painter_.get());
  for (auto style : {skity::Paint::kFill_Style, skity::Paint::kStroke_Style}) {
    SCOPED_TRACE(style);
    for (bool shader : {false, true}) {
      SCOPED_TRACE(shader);
      painter->platform_painter_ = std::make_unique<skity::Paint>();
      painter->platform_painter_->SetColor(TTColor::BLUE);
      painter->platform_painter_->SetStyle(style);
      painter->platform_painter_->SetStrokeWidth(3);
      if (shader)
        painter->platform_painter_->SetShader(MakeSkityGradient(*fill));
      painter->SetFillGradient(nullptr);
      canvas_->Clear(0);
      helper_.DrawGlyphs(&typeface, 1, &glyph, nullptr, 0, 15, 75, &x, &y,
                         painter);
      canvas_->Flush();
      auto* pixels = bitmap_.GetPixelAddr();
      const auto size = bitmap_.RowBytes() * bitmap_.Height();
      std::vector<uint8_t> reference(pixels, pixels + size);
      ASSERT_TRUE(std::any_of(reference.begin(), reference.end(),
                              [](uint8_t value) { return value != 0; }));
      painter->SetFillGradient(fill);
      canvas_->Clear(0);
      helper_.DrawGlyphs(&typeface, 1, &glyph, nullptr, 0, 15, 75, &x, &y,
                         painter);
      canvas_->Flush();
      EXPECT_TRUE(std::equal(reference.begin(), reference.end(), pixels));
      EXPECT_EQ(painter->platform_painter_->GetStyle(), style);
    }
  }
}

TEST_F(GradientSkityTest, RadialUsesEllipseMatrix) {
  gradient_.type = GradientType::kRadial;
  gradient_.radial.size = RadialGradientSize::kExplicit;
  gradient_.radial.radius_x = {60, GradientUnit::kLength};
  gradient_.radial.radius_y = {30, GradientUnit::kLength};
  Fill();
  EXPECT_GT(Pixel(80, 50).GetRed(), 230);
  EXPECT_GT(Pixel(139, 50).GetBlue(), 230);
  EXPECT_GT(Pixel(80, 79).GetBlue(), 230);
}

TEST_F(GradientSkityTest, RadialUsesNativeMinimumRadius) {
  gradient_.type = GradientType::kRadial;
  gradient_.radial.size = RadialGradientSize::kExplicit;
  gradient_.radial.center_y = {50.5f, GradientUnit::kLength};
  for (float radius : {0.f, 0.5f}) {
    gradient_.radial.radius_x = {radius, GradientUnit::kLength};
    gradient_.radial.radius_y = {radius, GradientUnit::kLength};
    canvas_->DrawColor(0);
    Fill();
    EXPECT_NEAR(Pixel(80, 50).GetRed(), 128, 3);
    EXPECT_NEAR(Pixel(80, 50).GetBlue(), 128, 3);
  }
}

TEST_F(GradientSkityTest, ConicUsesTopOriginAndStopAlpha) {
  gradient_.type = GradientType::kConic;
  gradient_.stops[0].color = 0x80ff0000;
  gradient_.stops[1].color = 0x800000ff;
  Fill();
  EXPECT_GT(Pixel(81, 15).GetRed(), Pixel(81, 15).GetBlue());
  EXPECT_GT(Pixel(30, 50).GetBlue(), Pixel(30, 50).GetRed());
  EXPECT_NEAR(Pixel(81, 15).GetAlpha(), 128, 1);
}

TEST_F(GradientSkityTest, ConicInterpolatesColorBeforeApplyingAlpha) {
  gradient_.type = GradientType::kConic;
  gradient_.stops[0].color = 0x40ff0000;
  Fill();
  auto color = Pixel(80, 80);
  EXPECT_NEAR(color.GetRed(), color.GetBlue(), 3);
  EXPECT_NEAR(color.GetAlpha(), 160, 2);
}

TEST_F(GradientSkityTest, GlyphOriginDoesNotMoveGradientAndStrokeStaysSolid) {
  auto font = skity::Typeface::MakeFromFile(
      FONT_ROOT "Inter/Inter-VariableFont_opsz,wght.ttf");
  ASSERT_TRUE(font);
  skity::textlayout::SkityTypefaceHelper typeface(font);
  uint16_t glyphs[2]{font->UnicharToGlyph('H'), font->UnicharToGlyph('H')};
  float x[2]{0, 90}, y[2]{0, 0};
  painter_->SetTextSize(64);
  painter_->SetStrokeColor(TTColor::GREEN);
  painter_->SetStrokeWidth(2);
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  helper_.DrawGlyphs(&typeface, 2, glyphs, nullptr, 0, 15, 75, x, y,
                     painter_.get());
  canvas_->Flush();
  int red = 0, blue = 0, green = 0;
  for (uint32_t y = 0; y < 100; ++y) {
    for (uint32_t x = 0; x < 160; ++x) {
      const auto c = Pixel(x, y);
      if (c.GetAlpha() < 128) continue;
      if (c.GetRed() > c.GetBlue() + 30) ++red;
      if (c.GetBlue() > c.GetRed() + 30) ++blue;
      if (c.GetGreen() > 200) ++green;
    }
  }
  EXPECT_GT(red, 100);
  EXPECT_GT(blue, 100);
  EXPECT_GT(green, 100);
}
