// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/font_info.h>
#include <textra/style.h>

#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/float_comparison.h"

using namespace ttoffice::tttext;

TEST(StyleTest, AttributeSetterGetterAndFlag) {
#define SET_ATTRIBUTE_AND_CHECK_RESULT(ATTRIBUTE_NAME, COMPARE)             \
  {                                                                         \
    /* Arrange */                                                           \
    Style style;                                                            \
    EXPECT_FALSE(style.HasStyleAttribute(Style::ATTRIBUTE_NAME##Flag));     \
    /* Act and Assert */                                                    \
    style.Set##ATTRIBUTE_NAME(Style::DefaultStyle().Get##ATTRIBUTE_NAME()); \
    EXPECT_TRUE(style.HasStyleAttribute(Style::ATTRIBUTE_NAME##Flag));      \
    EXPECT_TRUE(COMPARE(style.Get##ATTRIBUTE_NAME(),                        \
                        Style::DefaultStyle().Get##ATTRIBUTE_NAME()));      \
  }

  SET_ATTRIBUTE_AND_CHECK_RESULT(FontDescriptor, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(TextSize, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(TextScale, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(ForegroundColor, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(BackgroundColor, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationColor, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationType, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationStyle, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationThicknessMultiplier,
                                 ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationElementLength,
                                 ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationGapLength, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(DecorationSideMargin, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(Bold, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(Italic, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(VerticalAlignment, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(WordSpacing, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(LetterSpacing, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(ForegroundPainter, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(BackgroundPainter, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(WordBreak, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(BaselineOffset, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(TextSkew, ttoffice::FloatsEqual);
  {
    Style style;
    EXPECT_FALSE(style.HasStyleAttribute(Style::TextStrokeStyleFlag));
    style.SetTextStrokeValue(Style::DefaultStyle().GetTextStrokeValue());
    EXPECT_TRUE(style.HasStyleAttribute(Style::TextStrokeStyleFlag));
    EXPECT_TRUE(std::equal_to<>{}(style.GetTextStrokeValue(),
                                  Style::DefaultStyle().GetTextStrokeValue()));
    style.SetTextStrokeStyle(TTColor::RED, 25);
    EXPECT_TRUE(std::equal_to<>{}(style.GetTextStrokeColor().GetPlainColor(),
                                  TTColor::RED));
    EXPECT_TRUE(ttoffice::FloatsEqual(style.GetTextStrokeWidth(), 25.f));
  }
  // SET_ATTRIBUTE_AND_CHECK_RESULT(TextShadowList, /* no operator== */);
}

TEST(StyleTest, ChangingSomeAttributesResetsShapeStyle) {
  const auto check_shape_style_changed = [](auto change_attribute) {
    Style style;
    ShapeStyle old_shape_style = style.GetShapeStyle();
    change_attribute(style);
    EXPECT_FALSE(style.GetShapeStyle() == old_shape_style);
  };
  check_shape_style_changed([](auto& style) {
    style.SetFontDescriptor({{"test"}, FontStyle::Bold(), 0});
  });
  check_shape_style_changed([](auto& style) { style.SetTextSize(1.f); });
  // Note: Bold and Italic are currently ignored by Style.
  // check_shape_style_changed(
  //     [](auto& style) { style.SetBold(!style.GetBold()); });
  // check_shape_style_changed(
  //     [](auto& style) { style.SetItalic(!style.GetItalic()); });
}

TEST(StyleTest, Reset) {
  Style style;
  style.SetBackgroundColor(TTColor(TTColor::BLUE));
  EXPECT_TRUE(style.HasStyleAttribute(Style::BackgroundColorFlag));
  style.Reset();
  EXPECT_FALSE(style.HasStyleAttribute(Style::BackgroundColorFlag));
}

TEST(StyleTest, FontFeaturesDistinguishShapingAndReset) {
  Style style;
  auto original = style.GetShapeStyle();
  auto descriptor = style.GetFontDescriptor();
  descriptor.font_feature_settings_ = {{{'l', 'i', 'g', 'a'}, 0},
                                       {{'c', 'v', '0', '1'}, 2}};
  style.SetFontDescriptor(descriptor);
  auto disabled = style.GetShapeStyle();
  EXPECT_FALSE(original == disabled);
  EXPECT_NE(std::hash<ShapeStyle>()(original),
            std::hash<ShapeStyle>()(disabled));
  EXPECT_EQ(style.GetFontDescriptor().font_feature_settings_.at(1).value, 2);

  Style inherited = style;
  EXPECT_EQ(inherited.GetShapeStyle(), disabled);
  descriptor.font_feature_settings_[0].value = 1;
  inherited.SetFontDescriptor(descriptor);
  EXPECT_FALSE(inherited.GetShapeStyle() == disabled);
  EXPECT_EQ(style.GetFontDescriptor().font_feature_settings_.at(0).value, 0);

  descriptor.font_feature_settings_.clear();
  inherited.SetFontDescriptor(descriptor);
  EXPECT_EQ(inherited.GetShapeStyle(), original);
}

TEST(StyleTest, FontFeatureVectorUsesAllTagBytesAndValues) {
  const FontFeature liga{{'l', 'i', 'g', 'a'}, 0};
  const FontFeature ligb{{'l', 'i', 'g', 'b'}, 0};
  const FontFeature enabled{{'l', 'i', 'g', 'a'}, 1};
  EXPECT_TRUE(liga == liga);
  EXPECT_FALSE(liga == ligb);
  EXPECT_FALSE(liga == enabled);

  FontDescriptor descriptor;
  descriptor.font_feature_settings_ = {liga, enabled};
  auto copy = descriptor;
  EXPECT_EQ(descriptor, copy);
  EXPECT_EQ(FontDescriptor::Hasher()(descriptor),
            FontDescriptor::Hasher()(copy));
  copy.font_feature_settings_[0] = ligb;
  EXPECT_FALSE(descriptor == copy);
  EXPECT_NE(FontDescriptor::Hasher()(descriptor),
            FontDescriptor::Hasher()(copy));

  copy.font_feature_settings_ = {enabled, liga};
  EXPECT_FALSE(descriptor == copy);
}
