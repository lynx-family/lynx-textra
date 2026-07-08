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

TEST(StyleTest, GenericAttributeSetterGetter) {
  Style style;

  EXPECT_TRUE(style.SetAttribute(kTextSize, StyleValue(18.f)));
  StyleValue value;
  EXPECT_TRUE(style.GetAttribute(kTextSize, &value));
  EXPECT_EQ(value.type, kStyleValueFloat);
  EXPECT_TRUE(value.IsNumber());
  EXPECT_TRUE(ttoffice::FloatsEqual(value.Float(), 18.f));
  EXPECT_TRUE(style.HasStyleAttribute(Style::TextSizeFlag));

  StyleValue int_value(static_cast<int32_t>(20));
  EXPECT_TRUE(int_value.IsNumber());
  EXPECT_EQ(int_value.Int32(), 20);
  EXPECT_TRUE(ttoffice::FloatsEqual(int_value.Float(), 20.f));
  EXPECT_TRUE(style.SetAttribute(kTextSize, int_value));
  EXPECT_TRUE(ttoffice::FloatsEqual(style.GetTextSize(), 20.f));

  EXPECT_TRUE(style.SetAttribute(kForegroundColor, StyleValue(TTColor::RED)));
  value = style.GetAttribute(kForegroundColor);
  EXPECT_EQ(value.type, kStyleValueInt64);
  EXPECT_EQ(value.UInt32(), TTColor::RED);
  EXPECT_EQ(style.GetForegroundColor(), TTColor::RED);

  EXPECT_TRUE(style.SetAttribute(
      kDecorationStyle, StyleValue(static_cast<int32_t>(LineType::kDashed))));
  EXPECT_EQ(style.GetDecorationStyle(), LineType::kDashed);

  Painter painter;
  EXPECT_TRUE(style.SetAttribute(kForegroundPainter, StyleValue(&painter)));
  value = style.GetAttribute(kForegroundPainter);
  EXPECT_TRUE(value.IsPointer());
  EXPECT_EQ(value.Pointer(), reinterpret_cast<intptr_t>(&painter));
  EXPECT_EQ(style.GetForegroundPainter(), &painter);

  const uint64_t stroke_value = Style::DefaultStyle().GetTextStrokeValue();
  EXPECT_TRUE(style.SetAttribute(kTextStrokeStyle, StyleValue(stroke_value)));
  EXPECT_EQ(style.GetTextStrokeValue(), stroke_value);

  EXPECT_TRUE(style.SetAttribute(kBold, StyleValue(true)));
  value = style.GetAttribute(kBold);
  EXPECT_TRUE(value.IsBoolean());
  EXPECT_TRUE(value.Bool());
}

TEST(StyleTest, GenericAttributeCopiesObjectValue) {
  FontDescriptor font_descriptor{{"test"}, FontStyle::Bold(), 1};
  Style style;

  EXPECT_TRUE(style.SetAttribute(
      kFontDescriptor, StyleValue(&font_descriptor, sizeof(FontDescriptor))));
  font_descriptor.platform_font_ = 2;

  EXPECT_EQ(style.GetFontDescriptor().platform_font_, 1u);

  StyleValue value = style.GetAttribute(kFontDescriptor);
  EXPECT_EQ(value.type, kStyleValuePointer);
  EXPECT_EQ(value.length, static_cast<int64_t>(sizeof(FontDescriptor)));
  const auto* stored_font_descriptor =
      reinterpret_cast<const FontDescriptor*>(value.Pointer());
  EXPECT_EQ(*stored_font_descriptor, style.GetFontDescriptor());
}

TEST(StyleTest, GenericAttributeRejectsWrongValueType) {
  Style style;

  EXPECT_FALSE(style.SetAttribute(kTextSize, StyleValue(true)));
  EXPECT_FALSE(style.HasStyleAttribute(Style::TextSizeFlag));
  EXPECT_TRUE(ttoffice::FloatsEqual(style.GetTextSize(),
                                    Style::DefaultStyle().GetTextSize()));

  StyleValue value;
  EXPECT_FALSE(style.GetAttribute(kExtraBaselineOffset, &value));
  EXPECT_FALSE(value.IsValid());
  EXPECT_FALSE(style.GetAttribute(kTextSize, nullptr));
}

TEST(StyleTest, CopyKeepsDecorationOffsetFromGenericAttribute) {
  Style style;
  style.SetAttribute(kDecorationOffset, StyleValue(-4.f));

  Style copy = style;

  EXPECT_TRUE(copy.HasStyleAttribute(Style::DecorationOffsetFlag));
  EXPECT_TRUE(ttoffice::FloatsEqual(
      copy.GetAttribute(kDecorationOffset).Float(), -4.f));
}
