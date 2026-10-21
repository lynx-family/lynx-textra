// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/gradient.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/text_layout.h>

#include <cmath>

#include "mocks.h"
#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/style/style_manager.h"

using namespace ttoffice::tttext;
using namespace testing;

namespace {
std::shared_ptr<Gradient> Linear() {
  auto gradient = std::make_shared<Gradient>();
  gradient->linear.angle = 90;
  gradient->stops = {{TTColor::RED, 0}, {TTColor::BLUE, 1}};
  return gradient;
}

class Typeface : public ITypefaceHelper {
 public:
  Typeface() : ITypefaceHelper(10001) {}
  float GetHorizontalAdvance(GlyphID, float size) const override {
    return size;
  }
  void GetHorizontalAdvances(GlyphID*, uint32_t count, float* widths,
                             float size) const override {
    std::fill(widths, widths + count, size);
  }
  void GetWidthBound(float* rect, GlyphID, float size) const override {
    rect[0] = 0;
    rect[1] = -0.75f * size;
    rect[2] = size;
    rect[3] = size;
  }
  void GetWidthBounds(float* rect, GlyphID*, uint32_t count,
                      float size) override {
    rect[0] = 0;
    rect[1] = -0.75f * size;
    rect[2] = count * size;
    rect[3] = 0.25f * size;
  }
  const void* GetFontData() const override { return nullptr; }
  size_t GetFontDataSize() const override { return 0; }
  int GetFontIndex() const override { return 0; }
  uint16_t UnicharToGlyph(Unichar, uint32_t) const override { return 1; }
  void UnicharsToGlyphs(const Unichar*, uint32_t count,
                        GlyphID* glyphs) const override {
    std::fill(glyphs, glyphs + count, 1);
  }
  uint32_t GetUnitsPerEm() const override { return 1000; }

 protected:
  void OnCreateFontInfo(FontInfo* info, float size) const override {
    *info = FontInfo(-0.75f * size, 0.25f * size, size);
  }
};

class Reader : public PlatformShapingResultReader {
 public:
  Reader(uint32_t count, float advance, TypefaceRef font)
      : count_(count), advance_(advance), font_(std::move(font)) {}
  uint32_t GlyphCount() const override { return count_; }
  uint32_t TextCount() const override { return count_; }
  GlyphID ReadGlyphID(uint32_t i) const override { return 1; }
  float ReadAdvanceX(uint32_t i) const override { return advance_; }
  uint32_t ReadIndices(uint32_t i) const override { return i; }
  TypefaceRef ReadFontId(uint32_t i) const override { return font_; }

 private:
  uint32_t count_;
  float advance_;
  TypefaceRef font_;
};

class GradientParagraph : public ParagraphImpl {
 public:
  StyleManager& Styles() { return *style_manager_; }
};

class GradientLayoutTest : public Test {
 protected:
  void SetUp() override {
    auto font = std::make_shared<Typeface>();
    shaper_ =
        std::make_unique<NiceMock<MockTTShaper>>(FontmgrCollection(nullptr));
    ON_CALL(*shaper_, OnShapeText)
        .WillByDefault(
            Invoke([this, font](const ShapeKey& key, ShapeResult* result) {
              ++shape_count_;
              Reader reader(static_cast<uint32_t>(key.text_.length()),
                            key.style_.GetFontSize(), font);
              result->AppendPlatformShapingResult(reader);
            }));
    Style style;
    style.SetTextSize(10);
    style.SetForegroundGradient(fill_);
    paragraph_.GetParagraphStyle().SetDefaultStyle(style);
    paragraph_.AddTextRun(&style, "AAA BBB CCC DDD");
    ON_CALL(canvas_, CreatePainter).WillByDefault(Invoke([] {
      return std::make_unique<Painter>();
    }));
  }
  std::unique_ptr<LayoutRegion> Layout(float width) {
    TextLayout layout(std::move(shaper_));
    TTTextContext context;
    auto region = std::make_unique<LayoutRegion>(width, 1000);
    layout.Layout(&paragraph_, region.get(), context);
    return region;
  }
  std::vector<std::shared_ptr<const ResolvedGradient>> Draw(
      LayoutRegion* region) {
    std::vector<std::shared_ptr<const ResolvedGradient>> result;
    EXPECT_CALL(canvas_, DrawGlyphs)
        .WillRepeatedly(Invoke(
            [&](const ITypefaceHelper*, uint32_t, const uint16_t*, const char*,
                uint32_t, float, float, float*, float*, Painter* painter) {
              result.push_back(painter->GetFillGradient());
            }));
    LayoutDrawer drawer(&canvas_);
    drawer.DrawLayoutPage(region);
    Mock::VerifyAndClearExpectations(&canvas_);
    return result;
  }
  std::shared_ptr<const ResolvedGradient> fill_{
      std::make_shared<const ResolvedGradient>(
          Linear()->Resolve({10, 20, 150, 30}))};
  GradientParagraph paragraph_;
  std::unique_ptr<MockTTShaper> shaper_;
  NiceMock<MockCanvasHelper> canvas_;
  uint32_t shape_count_{0};
};
}  // namespace

TEST(Gradient, ResolvesLinearBoundsAndAbsoluteStops) {
  auto gradient = Linear();
  gradient->stops = {{TTColor::RED, 20, GradientUnit::kLength},
                     {TTColor::BLUE, 1}};
  auto fill = gradient->Resolve({10, 20, 100, 50});
  EXPECT_NEAR(fill.start.x, 10, 0.001);
  EXPECT_NEAR(fill.end.x, 110, 0.001);
  EXPECT_NEAR(fill.start.y, 45, 0.001);
  EXPECT_FLOAT_EQ(fill.positions[0], 0.2f);
  EXPECT_FLOAT_EQ(gradient->stops[0].position, 20);
  fill = gradient->Resolve({10, 20, 200, 50});
  EXPECT_FLOAT_EQ(fill.positions[0], 0.1f);
}

TEST(Gradient, CornerDirectionDependsOnAspectRatio) {
  auto gradient = Linear();
  gradient->linear.direction = LinearGradientDirection::kToTopRight;
  auto fill = gradient->Resolve({0, 0, 200, 100});
  EXPECT_NEAR((fill.end.x - fill.start.x) / (fill.start.y - fill.end.y), 0.5f,
              0.001);
}

TEST(Gradient, LinearAndConicUseNativeMinimumBounds) {
  auto gradient = Linear();
  auto fill = gradient->Resolve({10, 20, 0, 0});
  EXPECT_FLOAT_EQ(fill.start.x, 10);
  EXPECT_FLOAT_EQ(fill.end.x, 11);
  EXPECT_FLOAT_EQ(fill.start.y, 20.5f);
  gradient->type = GradientType::kConic;
  fill = gradient->Resolve({10, 20, 0, 0});
  EXPECT_FLOAT_EQ(fill.center.x, 10.5f);
  EXPECT_FLOAT_EQ(fill.center.y, 20.5f);
}

TEST(Gradient, ClipsOutOfRangeStopsByInterpolatingColor) {
  auto gradient = Linear();
  gradient->stops = {{TTColor::RED, -1}, {TTColor::BLUE, 1}};
  auto fill = gradient->Resolve({0, 0, 100, 50});
  EXPECT_EQ(fill.colors.front(), 0xff800080u);
  EXPECT_FLOAT_EQ(fill.positions.front(), 0);
  gradient->stops = {{TTColor::RED, -2}, {TTColor::BLUE, -1}};
  fill = gradient->Resolve({0, 0, 100, 50});
  EXPECT_EQ(fill.colors, (std::vector<uint32_t>{TTColor::BLUE, TTColor::BLUE}));
}

TEST(Gradient, PreservesHardStopsAndAlpha) {
  auto gradient = Linear();
  gradient->stops = {{0x80ff0000u, 0},
                     {TTColor::RED, 0.5f},
                     {TTColor::BLUE, 0.5f},
                     {TTColor::BLUE, 1}};
  auto fill = gradient->Resolve({0, 0, 100, 50});
  EXPECT_EQ(fill.colors.front(), 0x80ff0000u);
  EXPECT_FLOAT_EQ(fill.positions[1], fill.positions[2]);
}

TEST(Gradient, ResolvesRadialEllipseAndLengthStops) {
  auto gradient = Linear();
  gradient->type = GradientType::kRadial;
  gradient->radial.size = RadialGradientSize::kExplicit;
  gradient->radial.radius_x = {40, GradientUnit::kLength};
  gradient->radial.radius_y = {20, GradientUnit::kLength};
  gradient->stops[0] = {TTColor::RED, 10, GradientUnit::kLength};
  auto fill = gradient->Resolve({10, 20, 100, 60});
  EXPECT_FLOAT_EQ(fill.center.x, 60);
  EXPECT_FLOAT_EQ(fill.center.y, 50);
  EXPECT_FLOAT_EQ(fill.radius_x, 40);
  EXPECT_FLOAT_EQ(fill.radius_y, 20);
  EXPECT_FLOAT_EQ(fill.positions[0], 0.5f);
}

TEST(Gradient, PreservesConicStartAndAngularStops) {
  auto gradient = Linear();
  gradient->type = GradientType::kConic;
  gradient->conic.start_angle = 45;
  gradient->stops[0] = {TTColor::RED, 90, GradientUnit::kAngle};
  auto fill = gradient->Resolve({10, 20, 100, 60});
  EXPECT_FLOAT_EQ(fill.start_angle, 45);
  EXPECT_FLOAT_EQ(fill.positions[0], 0.25f);
}

TEST(Gradient, PreservesZeroRadiusAndClipsBoundaryStops) {
  auto gradient = Linear();
  gradient->type = GradientType::kRadial;
  gradient->radial.size = RadialGradientSize::kExplicit;
  gradient->radial.radius_x = {0, GradientUnit::kLength};
  gradient->radial.radius_y = {0, GradientUnit::kLength};
  auto fill = gradient->Resolve({0, 0, 100, 20});
  EXPECT_FLOAT_EQ(fill.radius_x, 0);
  EXPECT_FLOAT_EQ(fill.radius_y, 0);
  EXPECT_EQ(fill.colors, (std::vector<uint32_t>{TTColor::RED, TTColor::BLUE}));
  gradient->type = GradientType::kLinear;
  gradient->stops = {{TTColor::RED, -1}, {TTColor::BLUE, 0}};
  fill = gradient->Resolve({0, 0, 100, 20});
  EXPECT_EQ(fill.colors.size(), 2u);
  EXPECT_EQ(fill.colors.front(), TTColor::BLUE);
}

TEST(Gradient, RadialExtentsMatchLynxNativeRadiusRules) {
  auto gradient = Linear();
  gradient->type = GradientType::kRadial;
  const struct {
    bool ellipse;
    RadialGradientSize size;
    float x;
    float y;
    float radius_x;
    float radius_y;
  } cases[] = {
      {true, RadialGradientSize::kClosestSide, 10, 0, 10, 0},
      {true, RadialGradientSize::kClosestCorner, 0, 50, 0, 0},
      {true, RadialGradientSize::kFarthestCorner, 100, 50, 100 * std::sqrt(2.f),
       50 * std::sqrt(2.f)},
      {true, RadialGradientSize::kFarthestSide, 0, 0, 200, 100},
      {false, RadialGradientSize::kClosestSide, 0, 50, 0, 0},
      {false, RadialGradientSize::kFarthestCorner, 0, 0,
       std::hypot(200.f, 100.f), std::hypot(200.f, 100.f)},
  };
  for (const auto& item : cases) {
    gradient->radial.ellipse = item.ellipse;
    gradient->radial.size = item.size;
    gradient->radial.center_x = {item.x, GradientUnit::kLength};
    gradient->radial.center_y = {item.y, GradientUnit::kLength};
    const auto fill = gradient->Resolve({10, 20, 200, 100});
    EXPECT_FLOAT_EQ(fill.center.x, 10 + item.x);
    EXPECT_FLOAT_EQ(fill.center.y, 20 + item.y);
    EXPECT_FLOAT_EQ(fill.radius_x, item.radius_x);
    EXPECT_FLOAT_EQ(fill.radius_y, item.radius_y);
  }
}

TEST(GradientStyle, ColorDoesNotChangeResolvedGradientOrShaping) {
  Style style;
  auto fill = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({0, 0, 90, 10}));
  style.SetForegroundGradient(fill);
  const auto* shape = &style.GetShapeStyle();
  StyleManager manager;
  manager.ApplyStyleInRange(style.GetImpl(), 0, 9);
  style.Reset();
  style.SetForegroundColor(TTColor::GREEN);
  EXPECT_FALSE(style.HasStyleAttribute(Style::ForegroundGradientFlag));
  EXPECT_EQ(&style.GetShapeStyle(), shape);
  manager.ApplyStyleInRange(style.GetImpl(), 3, 3);
  StyleRange range;
  manager.GetStyleRange(&range, 0, Style::ForegroundGradientFlag, 9);
  EXPECT_EQ(range.GetRange(), tttext::Range(0, 9));
  EXPECT_EQ(range.GetStyle().GetForegroundGradient(), fill);
  EXPECT_EQ(manager.GetStyle(4).GetForegroundColor(), TTColor::GREEN);
}

TEST(GradientStyle, OwnsSavedResolvedValuesAndDistinguishesNull) {
  StyleManager manager;
  std::weak_ptr<const ResolvedGradient> weak;
  {
    Style style;
    auto fill = std::make_shared<const ResolvedGradient>(
        Linear()->Resolve({0, 0, 90, 10}));
    weak = fill;
    style.SetForegroundGradient(fill);
    manager.ApplyStyleInRange(style.GetImpl(), 0, 9);
  }
  ASSERT_FALSE(weak.expired());
  manager.SaveStyle();
  Style clear;
  clear.SetForegroundGradient(nullptr);
  manager.ApplyStyleInRange(clear.GetImpl(), 3, 3);
  EXPECT_FALSE(manager.GetStyle(4).GetForegroundGradient());
  EXPECT_EQ(manager.GetStyle(1).GetForegroundGradient(), weak.lock());
  EXPECT_EQ(manager.GetStyle(7).GetForegroundGradient(), weak.lock());
  manager.RestoreStyle();
  EXPECT_EQ(manager.GetStyle(4).GetForegroundGradient(), weak.lock());
}

TEST(GradientStyle, SameResolvedValueMergesAdjacentApplications) {
  StyleManager manager;
  Style style;
  auto fill = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({10, 20, 90, 10}));
  style.SetForegroundGradient(fill);
  manager.ApplyStyleInRange(style.GetImpl(), 0, 3);
  manager.ApplyStyleInRange(style.GetImpl(), 3, 3);
  StyleRange range;
  manager.GetStyleRange(&range, 0, Style::ForegroundGradientFlag, 9);
  EXPECT_EQ(range.GetRange(), tttext::Range(0, 6));
  EXPECT_EQ(range.GetStyle().GetForegroundGradient(), fill);
  EXPECT_FLOAT_EQ(fill->bounds.left, 10);
  EXPECT_FLOAT_EQ(fill->bounds.width, 90);
}

TEST(GradientStyle, DefaultFillAndExplicitNullAreIndependent) {
  StyleManager manager;
  Style style;
  auto fill = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({0, 0, 90, 10}));
  style.SetForegroundGradient(fill);
  manager.SetParagraphStyle(style.GetImpl());
  StyleRange range;
  manager.GetStyleRange(&range, 0, Style::ForegroundGradientFlag, 9);
  EXPECT_EQ(range.GetStyle().GetForegroundGradient(), fill);
  Style clear;
  clear.SetForegroundGradient(nullptr);
  manager.ApplyStyleInRange(clear.GetImpl(), 3, 3);
  EXPECT_FALSE(manager.GetStyle(4).GetForegroundGradient());
  EXPECT_EQ(manager.GetStyle(1).GetForegroundGradient(), fill);
  manager.ClearStyleInRange(kForegroundGradient, tttext::Range{3, 6});
  EXPECT_EQ(manager.GetStyle(4).GetForegroundGradient(), fill);
}

TEST_F(GradientLayoutTest, DrawUsesCallerResolvedValueWithoutLayoutChanges) {
  auto region = Layout(60);
  ASSERT_GT(region->GetLineCount(), 1u);
  const auto shapes = shape_count_;
  auto fills = Draw(region.get());
  ASSERT_FALSE(fills.empty());
  for (const auto& fill : fills) EXPECT_EQ(fill, fill_);
  auto* line = region->GetLine(1);
  line->ModifyHorizontalAlignment(ParagraphHorizontalAlignment::kRight);
  line->UpdateLineTop(line->GetLineTop() + 25);
  for (const auto& fill : Draw(region.get())) EXPECT_EQ(fill, fill_);
  EXPECT_FLOAT_EQ(fill_->bounds.left, 10);
  EXPECT_FLOAT_EQ(fill_->bounds.top, 20);
  EXPECT_FLOAT_EQ(fill_->bounds.width, 150);
  EXPECT_FLOAT_EQ(fill_->bounds.height, 30);
  EXPECT_EQ(shape_count_, shapes);
}

TEST_F(GradientLayoutTest,
       ExplicitUpdatesDisableClearAndRestoreRedrawImmediately) {
  auto region = Layout(200);
  const auto shapes = shape_count_;
  paragraph_.SaveStyle();
  Style child;
  auto changed = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({30, 0, 30, 10}));
  child.SetForegroundGradient(changed);
  paragraph_.ApplyStyleInRange(child, 3, 3);
  EXPECT_EQ(Draw(region.get()),
            (std::vector<std::shared_ptr<const ResolvedGradient>>{
                fill_, changed, fill_}));
  child.SetForegroundGradient(nullptr);
  paragraph_.ApplyStyleInRange(child, 3, 3);
  EXPECT_EQ(Draw(region.get()),
            (std::vector<std::shared_ptr<const ResolvedGradient>>{
                fill_, nullptr, fill_}));
  paragraph_.Styles().ClearStyleInRange(kForegroundGradient,
                                        tttext::Range{3, 6});
  for (const auto& fill : Draw(region.get())) EXPECT_EQ(fill, fill_);
  paragraph_.RestoreStyle();
  for (const auto& fill : Draw(region.get())) EXPECT_EQ(fill, fill_);
  EXPECT_EQ(shape_count_, shapes);
}

TEST_F(GradientLayoutTest,
       AddsResolvedGradientAfterSolidLayoutWithoutRelayout) {
  Style clear;
  clear.SetForegroundGradient(nullptr);
  paragraph_.ApplyStyleInRange(clear, 0, paragraph_.GetCharCount());
  auto region = Layout(200);
  const auto shapes = shape_count_;
  for (const auto& fill : Draw(region.get())) EXPECT_FALSE(fill);
  Style style;
  style.SetForegroundGradient(fill_);
  paragraph_.ApplyStyleInRange(style, 0, paragraph_.GetCharCount());
  for (const auto& fill : Draw(region.get())) EXPECT_EQ(fill, fill_);
  EXPECT_EQ(shape_count_, shapes);
}

TEST_F(GradientLayoutTest, CallerAppliesPerLineFillsForContinuousGradient) {
  auto region = Layout(60);
  ASSERT_GT(region->GetLineCount(), 1u);
  const auto shapes = shape_count_;
  std::vector<GradientBounds> rects;
  float width = 0;
  float top = 0;
  float bottom = 0;
  for (uint32_t i = 0; i < region->GetLineCount(); ++i) {
    auto* line = region->GetLine(i);
    float rect[4];
    line->GetBoundingRectForLine(rect);
    rects.push_back({rect[0], rect[1], rect[2], rect[3]});
    width += rect[2];
    top = std::min(top, rect[1] - line->GetLineBaseLine());
    bottom = std::max(bottom, rect[1] + rect[3] - line->GetLineBaseLine());
  }
  std::vector<std::shared_ptr<const ResolvedGradient>> prepared;
  float offset = 0;
  float previous_end = 0;
  auto gradient = Linear();
  for (uint32_t i = 0; i < region->GetLineCount(); ++i) {
    auto* line = region->GetLine(i);
    const auto& rect = rects[i];
    auto fill = std::make_shared<const ResolvedGradient>(
        gradient->Resolve({rect.left - offset, line->GetLineBaseLine() + top,
                           width, bottom - top}));
    Style style;
    style.SetForegroundGradient(fill);
    paragraph_.ApplyStyleInRange(style, line->GetStartCharPos(),
                                 line->GetCharCount());
    prepared.push_back(fill);
    EXPECT_NEAR((rect.left - fill->start.x) / width, previous_end, 0.001);
    previous_end = (rect.left + rect.width - fill->start.x) / width;
    offset += rect.width;
  }
  EXPECT_NEAR(previous_end, 1, 0.001);
  auto fills = Draw(region.get());
  ASSERT_FALSE(fills.empty());
  for (const auto& fill : fills) {
    EXPECT_NE(std::find(prepared.begin(), prepared.end(), fill),
              prepared.end());
  }
  EXPECT_EQ(shape_count_, shapes);
}

TEST_F(GradientLayoutTest, ChildSolidColorPreservesParentResolvedGradient) {
  Style child;
  child.SetForegroundColor(TTColor::GREEN);
  paragraph_.ApplyStyleInRange(child, 3, 3);
  auto region = Layout(200);
  auto fills = Draw(region.get());
  ASSERT_GT(fills.size(), 1u);
  for (const auto& fill : fills) EXPECT_EQ(fill, fill_);
}

TEST_F(GradientLayoutTest, CustomForegroundPainterTakesPrecedence) {
  Painter custom;
  custom.SetFillColor(TTColor::RED);
  Style style;
  style.SetForegroundPainter(&custom);
  paragraph_.ApplyStyleInRange(style, 0, paragraph_.GetCharCount());
  auto region = Layout(200);
  EXPECT_CALL(canvas_, DrawGlyphs)
      .WillRepeatedly(
          Invoke([&](const ITypefaceHelper*, uint32_t, const uint16_t*,
                     const char*, uint32_t, float, float, float*, float*,
                     Painter* painter) { EXPECT_EQ(painter, &custom); }));
  LayoutDrawer drawer(&canvas_);
  drawer.DrawLayoutPage(region.get());
}

TEST_F(GradientLayoutTest, NestedResolvedGradientsKeepParentGeometry) {
  Style child;
  auto gradient = Linear();
  gradient->linear.angle = 0;
  auto inner = std::make_shared<const ResolvedGradient>(
      gradient->Resolve({50, 20, 30, 30}));
  child.SetForegroundGradient(inner);
  paragraph_.ApplyStyleInRange(child, 4, 3);
  auto region = Layout(200);
  EXPECT_EQ(Draw(region.get()),
            (std::vector<std::shared_ptr<const ResolvedGradient>>{fill_, inner,
                                                                  fill_}));
  EXPECT_FLOAT_EQ(fill_->start.x, 10);
  EXPECT_FLOAT_EQ(fill_->end.x, 160);
}

TEST_F(GradientLayoutTest, PartialDrawUsesExplicitlyAppliedFill) {
  auto region = Layout(60);
  auto* line = region->GetLine(1);
  auto fill = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({-30, 10, 150, 10}));
  Style style;
  style.SetForegroundGradient(fill);
  paragraph_.ApplyStyleInRange(style, line->GetStartCharPos(),
                               line->GetCharCount());
  EXPECT_CALL(canvas_, DrawGlyphs)
      .WillRepeatedly(Invoke([&](const ITypefaceHelper*, uint32_t,
                                 const uint16_t*, const char*, uint32_t, float,
                                 float, float*, float*, Painter* painter) {
        EXPECT_EQ(painter->GetFillGradient(), fill);
      }));
  LayoutDrawer drawer(&canvas_);
  drawer.DrawTextLine(line, 0, line->GetCharCount());
}

TEST_F(GradientLayoutTest, EllipsisUsesOwnGradientDespiteStyleManagerOverride) {
  auto gradient = Linear();
  gradient->linear.angle = 0;
  auto inner = std::make_shared<const ResolvedGradient>(
      gradient->Resolve({0, 0, 60, 10}));
  Style style;
  style.SetForegroundGradient(inner);
  paragraph_.ApplyStyleInRange(style, 0, paragraph_.GetCharCount());
  paragraph_.GetParagraphStyle().SetMaxLines(1);
  paragraph_.GetParagraphStyle().SetEllipsis("...");
  paragraph_.GetParagraphStyle().SetEllipsisUsesDefaultForeground(true);
  Style default_style = paragraph_.GetParagraphStyle().GetDefaultStyle();
  default_style.SetForegroundGradient(std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({100, 20, 300, 30})));
  paragraph_.GetParagraphStyle().SetDefaultStyle(default_style);
  auto region = Layout(60);
  auto fills = Draw(region.get());
  ASSERT_GE(fills.size(), 2u);
  EXPECT_EQ(fills.front(), inner);
  EXPECT_EQ(fills.back(), fill_);
}

TEST_F(GradientLayoutTest, EllipsisKeepsOwnGradientAfterStyleManagerUpdate) {
  paragraph_.GetParagraphStyle().SetMaxLines(1);
  paragraph_.GetParagraphStyle().SetEllipsis("...");
  paragraph_.GetParagraphStyle().SetEllipsisUsesDefaultForeground(false);
  auto region = Layout(60);
  const auto shapes = shape_count_;
  auto before = Draw(region.get());
  ASSERT_FALSE(before.empty());
  EXPECT_EQ(before.back(), fill_);
  auto inner = std::make_shared<const ResolvedGradient>(
      Linear()->Resolve({20, 0, 200, 10}));
  Style style;
  style.SetForegroundGradient(inner);
  paragraph_.ApplyStyleInRange(style, 0, paragraph_.GetCharCount());
  auto fills = Draw(region.get());
  ASSERT_GE(fills.size(), 2u);
  EXPECT_EQ(fills.front(), inner);
  EXPECT_EQ(fills.back(), fill_);
  EXPECT_EQ(shape_count_, shapes);
}

TEST_F(GradientLayoutTest, EllipsisWithoutOwnGradientStaysSolid) {
  ParagraphImpl paragraph;
  Style plain;
  plain.SetTextSize(10);
  paragraph.GetParagraphStyle().SetDefaultStyle(plain);
  paragraph.GetParagraphStyle().SetMaxLines(1);
  paragraph.GetParagraphStyle().SetEllipsis("...");
  paragraph.AddTextRun(&plain, "AAA BBB CCC DDD");
  Style gradient;
  gradient.SetForegroundGradient(fill_);
  paragraph.ApplyStyleInRange(gradient, 0, paragraph.GetCharCount());
  TextLayout layout(std::move(shaper_));
  TTTextContext context;
  LayoutRegion region(60, 1000);
  layout.Layout(&paragraph, &region, context);
  auto fills = Draw(&region);
  ASSERT_GE(fills.size(), 2u);
  EXPECT_EQ(fills.front(), fill_);
  EXPECT_FALSE(fills.back());
}

TEST_F(GradientLayoutTest, ResetGradientDoesNotLeakIntoEllipsis) {
  ParagraphImpl paragraph;
  Style style;
  style.SetForegroundGradient(fill_);
  style.Reset();
  style.SetTextSize(10);
  ASSERT_FALSE(style.HasStyleAttribute(Style::ForegroundGradientFlag));
  paragraph.GetParagraphStyle().SetMaxLines(1);
  paragraph.GetParagraphStyle().SetEllipsis("...");
  paragraph.AddTextRun(&style, "AAA BBB CCC DDD");
  TextLayout layout(std::move(shaper_));
  TTTextContext context;
  LayoutRegion region(60, 1000);
  layout.Layout(&paragraph, &region, context);
  auto fills = Draw(&region);
  ASSERT_GE(fills.size(), 2u);
  for (const auto& fill : fills) EXPECT_FALSE(fill);
}
