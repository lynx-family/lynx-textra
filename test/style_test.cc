// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/font_info.h>
#include <textra/style.h>

#include "tt_shaper.h"
#include "utils/float_comparison.h"

using namespace ttoffice::tttext;

TEST(StyleTest, AttributeSetterGetterAndFlag) {
#define SET_ATTRIBUTE_AND_CHECK_RESULT(ATTRIBUTE_NAME, COMPARE)             \
  {                                                                         \
    /* Arrange */                                                           \
    Style style;                                                            \
    EXPECT_FALSE(style.Has##ATTRIBUTE_NAME());                              \
    /* Act and Assert */                                                    \
    style.Set##ATTRIBUTE_NAME(Style::DefaultStyle().Get##ATTRIBUTE_NAME()); \
    EXPECT_TRUE(style.Has##ATTRIBUTE_NAME());                               \
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
  SET_ATTRIBUTE_AND_CHECK_RESULT(Bold, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(Italic, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(VerticalAlignment, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(WordSpacing, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(LetterSpacing, ttoffice::FloatsEqual);
  SET_ATTRIBUTE_AND_CHECK_RESULT(ForegroundPainter, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(BackgroundPainter, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(WordBreak, std::equal_to<>{});
  SET_ATTRIBUTE_AND_CHECK_RESULT(BaselineOffset, ttoffice::FloatsEqual);
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
  style.SetBackgroundColor(TTColor(TTColor::BLUE()));
  EXPECT_TRUE(style.HasBackgroundColor());
  style.Reset();
  EXPECT_FALSE(style.HasBackgroundColor());
}
