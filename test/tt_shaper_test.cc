// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "tt_shaper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/font_info.h>

#include <array>
#include <numeric>

#include "mocks.h"
#include "src/textlayout/shape_cache.h"
#include "test_utils.h"

using namespace ttoffice::tttext;
using namespace ::testing;

TEST(ShapeStyle, Constructor) {
  FontDescriptor font_descriptor;
  ShapeStyle style(font_descriptor, 10.f, true, true);
  EXPECT_EQ(style.GetFontDescriptor(), font_descriptor);
  EXPECT_FLOAT_EQ(style.GetFontSize(), 10.f);
  EXPECT_TRUE(style.FakeBold());
  EXPECT_TRUE(style.FakeItalic());
}

TEST(ShapeStyle, CopyAndAssignment) {
  ShapeStyle source(FontDescriptor(), 10.f, true, true);
  ShapeStyle copy_by_constructor(source);  // NOLINT(performance-*)
  EXPECT_EQ(copy_by_constructor, source);
  ShapeStyle copy_by_assignment;
  copy_by_assignment = source;
  EXPECT_EQ(copy_by_assignment, source);
}

TEST(ShapeStyle, SetFontDescriptor) {
  FontDescriptor old_font;
  ShapeStyle old_style(old_font, 10.f, false, false);
  EXPECT_EQ(old_style.GetFontDescriptor(), old_font);
  ShapeStyle new_style = old_style;

  FontDescriptor new_font;
  new_font.font_style_ = FontStyle::Bold();
  new_style.SetFontDescriptor(new_font);
  EXPECT_EQ(new_style.GetFontDescriptor(), new_font);

  EXPECT_FALSE(old_style == new_style);
}

TEST(ShapeKey, Constructor) {
  const std::u32string text = U"Hello world";
  const float font_size = 10.f;
  {
    ShapeStyle style(FontDescriptor(), font_size, false, false);
    ShapeKey key(text.c_str(), text.size(), &style, true);
    EXPECT_EQ(key.style_, style);
    EXPECT_EQ(key.text_, text);
    EXPECT_TRUE(key.rtl_);
  }
  {
    ShapeKey key(text.c_str(), text.size(), FontDescriptor(), font_size, true,
                 true, true);
    EXPECT_EQ(key.text_, text);
    EXPECT_EQ(key.style_, ShapeStyle(FontDescriptor(), font_size, true, true));
    EXPECT_TRUE(key.rtl_);
  }
}

TEST(ShapeKey, UsedAsMapKey) {
  const std::u32string text1 = U"Hello world";
  const std::u32string text2 = U"Different text";
  const ShapeStyle style1(FontDescriptor(), 10.f, false, false);
  const ShapeStyle style2(FontDescriptor(), 10.f, true, true);

  ShapeKey key1(text1.c_str(), text1.size(), &style1, false);
  ShapeKey key2(text1.c_str(), text1.size(), &style1, false);  // Same as key1
  ShapeKey key3(text2.c_str(), text2.size(), &style1, false);  // Different text
  ShapeKey key4(text1.c_str(), text1.size(), &style2,
                false);                                       // Different style
  ShapeKey key5(text1.c_str(), text1.size(), &style1, true);  // Different RTL

  std::unordered_map<ShapeKey, int> shape_map;
  shape_map[key1] = 1;
  shape_map[key2] = 2;  // overrides shape_map[key1]
  shape_map[key3] = 3;
  shape_map[key4] = 4;
  shape_map[key5] = 5;

  EXPECT_EQ(shape_map.size(), 4u);
  EXPECT_EQ(shape_map[key1], 2);
  EXPECT_EQ(shape_map[key2], 2);  // key2 is identical to key1
  EXPECT_EQ(shape_map[key3], 3);
  EXPECT_EQ(shape_map[key4], 4);
  EXPECT_EQ(shape_map[key5], 5);
}

TEST(ShapeResult, Constructor) {
  const uint32_t char_count = 10;
  ShapeResult result(char_count, false);
  EXPECT_EQ(result.CharCount(), 0u);
  EXPECT_EQ(result.GlyphCount(), 0u);
  EXPECT_FALSE(result.IsRTL());
}

TEST(ShapeResult, CopyConstructor) {
  const uint32_t char_count = 10;
  ShapeResult source(char_count, true);
  ShapeResult copy(source);
  EXPECT_EQ(copy.CharCount(), source.CharCount());
  EXPECT_EQ(copy.GlyphCount(), source.GlyphCount());
  EXPECT_EQ(copy.IsRTL(), source.IsRTL());
}

TEST(ShapeResult, AppendPlatformShapingResult) {
  TestShapingResultReader shaping_result(4);
  std::vector<std::array<float, 2>> advances = {
      {5.f, 0.f}, {10.f, 0.f}, {15.f, 0.f}, {20.f, 0.f}};
  shaping_result.glyphs_ = {1, 2, 3, 4};
  shaping_result.advances_ = advances;
  shaping_result.positions_ = {
      {0.f, 1.f}, {5.f, 2.f}, {15.f, 3.f}, {30.f, 4.f}};
  TypefaceRef typeface = nullptr;
  shaping_result.font_ = typeface;

  ShapeResult result(shaping_result.glyphs_.size(), false);
  result.AppendPlatformShapingResult(shaping_result);
  EXPECT_EQ(result.CharCount(), shaping_result.TextCount());
  for (uint32_t i = 0; i < result.CharCount(); i++) {
    EXPECT_EQ(result.Glyphs(i), shaping_result.glyphs_[i]);
    EXPECT_FLOAT_EQ(result.Advances(i)[0], shaping_result.advances_[i][0]);
    EXPECT_FLOAT_EQ(result.Advances(i)[1], shaping_result.advances_[i][1]);
    EXPECT_FLOAT_EQ(result.Positions(i)[0], shaping_result.positions_[i][0]);
    EXPECT_FLOAT_EQ(result.Positions(i)[1], shaping_result.positions_[i][1]);
    EXPECT_EQ(result.Font(i), shaping_result.font_);
    EXPECT_EQ(result.FontByCharId(i), typeface);
    EXPECT_EQ(result.CharToGlyph(i), i);
    EXPECT_EQ(result.GlyphToChar(i), i);
  }
}

TEST(ShapeResult, MeasureWidth) {
  ShapeResult result(0, false);
  EXPECT_FLOAT_EQ(result.MeasureWidth(0, 0, 0.0f), 0.0f);

  TestShapingResultReader shaping_result(4);
  shaping_result.glyphs_ = {1, 2, 3, 4};
  std::vector<std::array<float, 2>> advances = {
      {5.f, 0.f}, {10.f, 0.f}, {15.f, 0.f}, {20.f, 0.f}};
  shaping_result.advances_ = advances;
  TypefaceRef typeface = nullptr;
  shaping_result.font_ = typeface;
  result.AppendPlatformShapingResult(shaping_result);

  // Test MeasureWidth without letter spacing
  for (uint32_t idx = 1; idx <= advances.size(); idx++) {
    float expected_width = std::accumulate(
        advances.cbegin(), advances.cbegin() + idx, 0.f,
        [](float sum, auto coordinate) { return sum + coordinate[0]; });
    EXPECT_FLOAT_EQ(result.MeasureWidth(0, idx, 0.f), expected_width);
  }
  // Test MeasureWidth with letter spacing
  const float letter_spacing = 2.f;
  for (uint32_t idx = 1; idx <= advances.size(); idx++) {
    float expected_width =
        std::accumulate(
            advances.cbegin(), advances.cbegin() + idx, 0.f,
            [](float sum, auto coordinate) { return sum + coordinate[0]; }) +
        letter_spacing * idx;
    EXPECT_FLOAT_EQ(result.MeasureWidth(0, idx, letter_spacing),
                    expected_width);
  }
}

TEST(TTShaper, Constructor) {
  FontmgrCollection expected = TestUtils::getFontmgrCollection();
  TestShaper shaper(expected);
  const FontmgrCollection& actual = shaper.GetFontCollection();
  EXPECT_EQ(actual.getFontManagersCount(), expected.getFontManagersCount());
  EXPECT_EQ(actual.GetDefaultFontManager(), expected.GetDefaultFontManager());
  EXPECT_EQ(actual.GetAssetFontManager(), expected.GetAssetFontManager());
  EXPECT_EQ(actual.GetDynamicFontManager(), expected.GetDynamicFontManager());
  EXPECT_EQ(actual.GetTestFontManager(), expected.GetTestFontManager());
}

TEST(TTShaper, ShapeTextCallsOnShapeText) {
  FontmgrCollection font_collection = TestUtils::getFontmgrCollection();
  MockTTShaper mock_shaper(font_collection);
  const std::u32string text = U"test";
  ShapeStyle style(FontDescriptor(), 12.0f, false, false);
  const bool rtl = false;
  ShapeKey key(text.c_str(), text.length(), &style, rtl);

  // Test that ShapeText calls OnShapeText with the correct key
  EXPECT_CALL(mock_shaper, OnShapeText(key, _))
      .WillOnce(Invoke([&text](const ShapeKey& _, ShapeResult* result) {
        TestShapingResultReader shaping_result(text.length());
        result->AppendPlatformShapingResult(shaping_result);
      }));
  auto result = mock_shaper.ShapeText(text.c_str(), text.length(), &style, rtl);
  // Check the returned ShapeResult is set by OnShapeText
  EXPECT_EQ(result->CharCount(), text.length());
}
