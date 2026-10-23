// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/platform/skia/skia_canvas_helper.h>

#include "include/core/SkBitmap.h"
#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_mac_ct.h"

using namespace ttoffice::tttext;

namespace {
class GradientSkiaTest : public testing::Test {
 protected:
  void SetUp() override {
    bitmap_.allocN32Pixels(160, 100);
    bitmap_.eraseColor(SK_ColorTRANSPARENT);
    canvas_ = std::make_unique<SkCanvas>(bitmap_);
    helper_ = std::make_unique<SkiaCanvasHelper>(canvas_.get());
    painter_ = helper_->CreatePainter();
    painter_->SetFillColor(TTColor::WHITE);
    gradient_.linear.angle = 90;
    gradient_.stops = {{TTColor::RED, 0}, {TTColor::BLUE, 1}};
  }
  void Fill(const GradientBounds& bounds = {0, 0, 160, 100}) {
    painter_->SetFillGradient(
        std::make_shared<const ResolvedGradient>(gradient_.Resolve(bounds)));
    helper_->DrawRect(0, 0, 160, 100, painter_.get());
  }
  std::unique_ptr<SkiaTypefaceHelper> MakeTypeface() {
    auto manager = SkFontMgr_New_CoreText(nullptr);
    auto font = manager->makeFromFile(FONT_ROOT
                                      "Inter/Inter-VariableFont_opsz,wght.ttf");
    return std::make_unique<SkiaTypefaceHelper>(std::move(font));
  }
  uint64_t Coverage() const {
    uint64_t sum = 0;
    for (int y = 0; y < 100; ++y)
      for (int x = 0; x < 160; ++x) sum += SkColorGetA(bitmap_.getColor(x, y));
    return sum;
  }
  SkColor Pixel(int x, int y) const { return bitmap_.getColor(x, y); }
  SkBitmap bitmap_;
  std::unique_ptr<SkCanvas> canvas_;
  std::unique_ptr<SkiaCanvasHelper> helper_;
  std::unique_ptr<Painter> painter_;
  Gradient gradient_;
};
}  // namespace

TEST_F(GradientSkiaTest, LinearUsesBoundsAndFollowingSolidClearsShader) {
  Fill({10, 0, 140, 100});
  EXPECT_GT(SkColorGetR(Pixel(20, 50)), SkColorGetB(Pixel(20, 50)));
  EXPECT_GT(SkColorGetB(Pixel(140, 50)), SkColorGetR(Pixel(140, 50)));
  painter_->SetFillGradient(nullptr);
  painter_->SetFillColor(TTColor::GREEN);
  helper_->DrawRect(0, 0, 40, 100, painter_.get());
  EXPECT_EQ(Pixel(20, 50), TTColor::GREEN);
}

TEST_F(GradientSkiaTest, RadialUsesEllipseMatrix) {
  gradient_.type = GradientType::kRadial;
  gradient_.radial.size = RadialGradientSize::kExplicit;
  gradient_.radial.radius_x = {60, GradientUnit::kLength};
  gradient_.radial.radius_y = {30, GradientUnit::kLength};
  Fill();
  EXPECT_GT(SkColorGetR(Pixel(80, 50)), 230);
  EXPECT_GT(SkColorGetB(Pixel(139, 50)), 230);
  EXPECT_GT(SkColorGetB(Pixel(80, 79)), 230);
}

TEST_F(GradientSkiaTest, RadialUsesNativeMinimumRadius) {
  gradient_.type = GradientType::kRadial;
  gradient_.radial.size = RadialGradientSize::kExplicit;
  gradient_.radial.center_y = {50.5f, GradientUnit::kLength};
  for (float radius : {0.f, 0.5f}) {
    gradient_.radial.radius_x = {radius, GradientUnit::kLength};
    gradient_.radial.radius_y = {radius, GradientUnit::kLength};
    bitmap_.eraseColor(SK_ColorTRANSPARENT);
    Fill();
    EXPECT_NEAR(SkColorGetR(Pixel(80, 50)), 128, 3);
    EXPECT_NEAR(SkColorGetB(Pixel(80, 50)), 128, 3);
  }
}

TEST_F(GradientSkiaTest, ConicUsesTopOriginAndPreservesAlpha) {
  gradient_.type = GradientType::kConic;
  gradient_.stops = {{0x80ff0000u, 0}, {0x800000ffu, 1}};
  Fill();
  EXPECT_GT(SkColorGetR(Pixel(81, 15)), SkColorGetB(Pixel(81, 15)));
  EXPECT_GT(SkColorGetB(Pixel(30, 50)), SkColorGetR(Pixel(30, 50)));
  EXPECT_NEAR(SkColorGetA(Pixel(81, 15)), 128, 1);
}

TEST_F(GradientSkiaTest, ConicRotatesStartAngleAroundCenter) {
  gradient_.type = GradientType::kConic;
  gradient_.conic.start_angle = 90;
  Fill();
  EXPECT_GT(SkColorGetR(Pixel(140, 51)), 240);
  EXPECT_GT(SkColorGetB(Pixel(80, 10)), SkColorGetR(Pixel(80, 10)));
}

TEST_F(GradientSkiaTest, ConicInterpolatesColorBeforeApplyingAlpha) {
  gradient_.type = GradientType::kConic;
  gradient_.stops[0].color = 0x40ff0000u;
  Fill();
  EXPECT_NEAR(SkColorGetR(Pixel(80, 80)), SkColorGetB(Pixel(80, 80)), 3);
  EXPECT_NEAR(SkColorGetA(Pixel(80, 80)), 160, 2);
}

TEST_F(GradientSkiaTest, PreservesHardStopsAndIgnoresSolidFillAlpha) {
  gradient_.stops = {{TTColor::RED, 0},
                     {TTColor::RED, 0.5f},
                     {TTColor::BLUE, 0.5f},
                     {TTColor::BLUE, 1}};
  painter_->SetFillColor(0);
  Fill();
  EXPECT_EQ(Pixel(79, 50), TTColor::RED);
  EXPECT_EQ(Pixel(80, 50), TTColor::BLUE);
}

TEST_F(GradientSkiaTest, RectFillAndSolidStrokeUseIndependentPaints) {
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  painter_->SetStrokeColor(TTColor::GREEN);
  painter_->SetStrokeWidth(4);
  helper_->DrawRect(10, 10, 150, 90, painter_.get());
  EXPECT_EQ(Pixel(10, 50), TTColor::GREEN);
  EXPECT_GT(SkColorGetR(Pixel(20, 50)), SkColorGetB(Pixel(20, 50)));
  EXPECT_GT(SkColorGetB(Pixel(140, 50)), SkColorGetR(Pixel(140, 50)));
  painter_->SetFillGradient(nullptr);
  painter_->SetFillColor(TTColor::UNDEFINED);
  painter_->SetStrokeColor(TTColor::RED);
  helper_->DrawRect(30, 10, 130, 90, painter_.get());
  EXPECT_EQ(Pixel(30, 50), TTColor::RED);
}

TEST_F(GradientSkiaTest, GlyphOriginDoesNotMoveGradientAndStrokeStaysSolid) {
  auto typeface = MakeTypeface();
  uint16_t glyphs[2]{typeface->UnicharToGlyph('H'),
                     typeface->UnicharToGlyph('H')};
  float x[2]{0, 90}, y[2]{0, 0};
  painter_->SetTextSize(64);
  painter_->SetStrokeColor(TTColor::GREEN);
  painter_->SetStrokeWidth(2);
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  helper_->DrawGlyphs(typeface.get(), 2, glyphs, nullptr, 0, 15, 75, x, y,
                      painter_.get());
  int red = 0, blue = 0, green = 0;
  for (int y = 0; y < 100; ++y) {
    for (int x = 0; x < 160; ++x) {
      const auto color = Pixel(x, y);
      if (SkColorGetA(color) < 128) continue;
      if (SkColorGetR(color) > SkColorGetB(color) + 30) ++red;
      if (SkColorGetB(color) > SkColorGetR(color) + 30) ++blue;
      if (SkColorGetG(color) > 200) ++green;
    }
  }
  EXPECT_GT(red, 100);
  EXPECT_GT(blue, 100);
  EXPECT_GT(green, 100);
}

TEST_F(GradientSkiaTest, GradientPreservesSyntheticBoldCoverage) {
  auto typeface = MakeTypeface();
  uint16_t glyph = typeface->UnicharToGlyph('H');
  float x = 0, y = 0;
  painter_->SetTextSize(64);
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  helper_->DrawGlyphs(typeface.get(), 1, &glyph, nullptr, 0, 15, 75, &x, &y,
                      painter_.get());
  const auto regular = Coverage();
  bitmap_.eraseColor(SK_ColorTRANSPARENT);
  painter_->SetBold(true);
  helper_->DrawGlyphs(typeface.get(), 1, &glyph, nullptr, 0, 15, 75, &x, &y,
                      painter_.get());
  EXPECT_GT(Coverage(), regular * 1.05f);
}

TEST_F(GradientSkiaTest, SimpleTextUsesGradient) {
  auto typeface = MakeTypeface();
  painter_->SetTextSize(64);
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({15, 0, 90, 100})));
  helper_->DrawText(typeface.get(), "HH", 2, 15, 75, painter_.get());
  int red = 0, blue = 0;
  for (int y = 0; y < 100; ++y) {
    for (int x = 0; x < 160; ++x) {
      const auto color = Pixel(x, y);
      if (SkColorGetA(color) < 128) continue;
      if (SkColorGetR(color) > SkColorGetB(color) + 30) ++red;
      if (SkColorGetB(color) > SkColorGetR(color) + 30) ++blue;
    }
  }
  EXPECT_GT(red, 100);
  EXPECT_GT(blue, 100);
}

TEST_F(GradientSkiaTest, ShadowUsesItsOwnSolidColor) {
  auto typeface = MakeTypeface();
  uint16_t glyph = typeface->UnicharToGlyph('H');
  float x = 0, y = 0;
  painter_->SetTextSize(64);
  painter_->SetShadowList({{TTColor::GREEN, {0, 0}, 4}});
  painter_->SetFillGradient(std::make_shared<const ResolvedGradient>(
      gradient_.Resolve({0, 0, 160, 100})));
  helper_->DrawGlyphs(typeface.get(), 1, &glyph, nullptr, 0, 15, 75, &x, &y,
                      painter_.get());
  int green = 0;
  for (int y = 0; y < 100; ++y) {
    for (int x = 0; x < 160; ++x) {
      const auto color = Pixel(x, y);
      if (SkColorGetA(color) > 16 &&
          SkColorGetG(color) > SkColorGetR(color) + 30 &&
          SkColorGetG(color) > SkColorGetB(color) + 30)
        ++green;
    }
  }
  EXPECT_GT(green, 100);
}

TEST_F(GradientSkiaTest, CustomNativePaintKeepsPriority) {
  auto* painter = static_cast<SkiaPainter*>(painter_.get());
  painter->sk_paint_ = std::make_unique<SkPaint>();
  painter->sk_paint_->setColor(TTColor::GREEN);
  painter->SetStrokeColor(TTColor::RED);
  painter->SetStrokeWidth(4);
  Fill();
  EXPECT_EQ(Pixel(10, 50), TTColor::GREEN);
  EXPECT_EQ(Pixel(140, 50), TTColor::GREEN);
  EXPECT_EQ(painter->sk_paint_->getShader(), nullptr);
}
