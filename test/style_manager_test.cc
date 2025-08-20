// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#define GTEST

#include "src/textlayout/style/style_manager.h"

#include <gtest/gtest.h>

#include <tuple>

namespace ttoffice {
namespace tttext {

TEST(StyleRange, BasicTest) {
  Range range(5, 10);
  Style style;
  style.SetTextScale(2.f);
  StyleRange style_range(range, style);

  EXPECT_EQ(style_range.GetRange(), range);
  EXPECT_EQ(style_range.GetStyle().GetTextScale(), 2.f);
}

TEST(AttributesRangeList, Constructor) {
  {
    // Test default constructor
    AttributesRangeList range_list;
    EXPECT_EQ(range_list.GetAttrValue(0), AttributesRangeList::Undefined());
    EXPECT_EQ(range_list.GetAttributeRange(0).second,
              AttributesRangeList::Undefined());
  }
  {
    // Test copy constructor
    Range range(5, 10);
    AttributesRangeList::ValueType value = 100u;
    AttributesRangeList range_list;
    range_list.SetRangeValue(range, value);

    AttributesRangeList range_list_copy(range_list);
    auto [actual_range, actual_value] =
        range_list_copy.GetAttributeRange(range.GetStart());
    EXPECT_EQ(actual_range, range);
    EXPECT_EQ(actual_value, value);
  }
}

TEST(AttributesRangeList, EnbaleAndDisableMergeRange) {
  AttributesRangeList range_list;
  AttributesRangeList::ValueType value = 100u;
  // Test with SetMergeRange(true)
  range_list.SetMergeRange(true);
  range_list.SetRangeValue(Range{1, 3}, value);
  range_list.SetRangeValue(Range{3, 5}, value);
  EXPECT_EQ(range_list.GetAttributeRange(1).first, (Range{1u, 5u}));
  EXPECT_EQ(range_list.GetAttributeRange(1).second, value);
  // Test with SetMergeRange(false)
  range_list.Clear();
  range_list.SetMergeRange(false);
  range_list.SetRangeValue(Range{1, 3}, 100u);
  range_list.SetRangeValue(Range{3, 5}, 200u);
  EXPECT_EQ(range_list.GetAttributeRange(1).first, (Range{1, 3}));
  EXPECT_EQ(range_list.GetAttributeRange(1).second, 100u);
  EXPECT_EQ(range_list.GetAttributeRange(3).first, (Range{3, 5}));
  EXPECT_EQ(range_list.GetAttributeRange(3).second, 200u);
}

TEST(AttributesRangeList, GetAttributeValueAndRange) {
  AttributesRangeList range_list;
  auto value1 = 100u;
  auto value2 = 200u;
  range_list.SetRangeValue(Range{1, 3}, value1);
  range_list.SetRangeValue(Range{5, 7}, value2);

  EXPECT_EQ(range_list.GetAttrValue(0), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(1), value1);
  EXPECT_EQ(range_list.GetAttrValue(2), value1);
  EXPECT_EQ(range_list.GetAttrValue(3), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(4), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(5), value2);
  EXPECT_EQ(range_list.GetAttrValue(6), value2);
  EXPECT_EQ(range_list.GetAttrValue(7), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(8), AttributesRangeList::Undefined());

  EXPECT_EQ(range_list.GetAttributeRange(0).first, (Range{0, 1}));
  EXPECT_EQ(range_list.GetAttributeRange(1).first, (Range{1, 3}));
  EXPECT_EQ(range_list.GetAttributeRange(2).first, (Range{1, 3}));
  EXPECT_EQ(range_list.GetAttributeRange(3).first, (Range{3, 5}));
  EXPECT_EQ(range_list.GetAttributeRange(4).first, (Range{3, 5}));
  EXPECT_EQ(range_list.GetAttributeRange(5).first, (Range{5, 7}));
  EXPECT_EQ(range_list.GetAttributeRange(6).first, (Range{5, 7}));
  EXPECT_EQ(range_list.GetAttributeRange(7).first,
            (Range{7, Range::MaxIndex()}));
  EXPECT_EQ(range_list.GetAttributeRange(8).first,
            (Range{7, Range::MaxIndex()}));
}

TEST(AttributesRangeList, ClearRangeValue) {
  AttributesRangeList::ValueType value1 = 100u;
  AttributesRangeList::ValueType value2 = 200u;
  auto init_range_list = [value1, value2]() -> AttributesRangeList {
    AttributesRangeList range_list;
    range_list.SetRangeValue(Range{1, 3}, value1);
    range_list.SetRangeValue(Range{5, 7}, value2);
    return AttributesRangeList(range_list);
  };
  {
    // Test clearing an existing range
    AttributesRangeList range_list = init_range_list();
    range_list.ClearRangeValue(Range{1, 3});
    EXPECT_EQ(range_list.GetAttrValue(1), AttributesRangeList::Undefined());
    EXPECT_EQ(range_list.GetAttrValue(2), AttributesRangeList::Undefined());
    EXPECT_EQ(range_list.GetAttrValue(5), value2);
    EXPECT_EQ(range_list.GetAttrValue(6), value2);
  }
  {
    // Test clearing a range that is fully contained within an existing range
    AttributesRangeList range_list = init_range_list();
    range_list.ClearRangeValue(Range{2, 3});
    EXPECT_EQ(range_list.GetAttrValue(1), value1);
    EXPECT_EQ(range_list.GetAttrValue(2), AttributesRangeList::Undefined());
  }
  {
    // Test clearing a range that overlaps with two existing ranges
    AttributesRangeList range_list = init_range_list();
    range_list.ClearRangeValue(Range{2, 6});
    EXPECT_EQ(range_list.GetAttrValue(1), value1);
    EXPECT_EQ(range_list.GetAttrValue(2), AttributesRangeList::Undefined());
    EXPECT_EQ(range_list.GetAttrValue(5), AttributesRangeList::Undefined());
    EXPECT_EQ(range_list.GetAttrValue(6), value2);
  }
  {
    // Test clearing a range that doesn't exist
    AttributesRangeList range_list = init_range_list();
    range_list.ClearRangeValue(Range{10, 15});
    EXPECT_EQ(range_list.GetAttrValue(1), value1);
    EXPECT_EQ(range_list.GetAttrValue(6), value2);
  }
}

TEST(AttributesRangeList, Clear) {
  AttributesRangeList range_list;
  range_list.SetRangeValue(Range{1, 3}, 100u);
  range_list.SetRangeValue(Range{5, 7}, 200u);
  EXPECT_EQ(range_list.GetAttrValue(1), 100u);
  EXPECT_EQ(range_list.GetAttrValue(5), 200u);

  range_list.Clear();
  EXPECT_EQ(range_list.GetAttrValue(1), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(2), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(5), AttributesRangeList::Undefined());
  EXPECT_EQ(range_list.GetAttrValue(6), AttributesRangeList::Undefined());
}

TEST(AttributesRangeList, CopyConstructorAndAssignmentOperator) {
  AttributesRangeList original;
  AttributesRangeList::ValueType value = 100u;
  original.SetRangeValue(Range{1, 3}, 100u);
  original.SetMergeRange(false);
  // Test copy constructor
  AttributesRangeList copy(original);
  EXPECT_EQ(copy.GetAttrValue(1), value);
  EXPECT_EQ(copy.GetAttrValue(2), value);
  EXPECT_EQ(copy.GetAttributeRange(1).first, (Range{1, 3}));
  // Test assignment operator
  AttributesRangeList assigned;
  assigned = original;
  EXPECT_EQ(assigned.GetAttrValue(1), value);
  EXPECT_EQ(assigned.GetAttrValue(2), value);
  EXPECT_EQ(assigned.GetAttributeRange(1).first, (Range{1, 3}));
}

TEST(AttributesRangeListTest, SetRangeValue) {
  AttributesRangeList range_list;
  auto int_value = 3;
  range_list.SetRangeValue(Range{1, 3}, int_value);
  range_list.SetRangeValue(Range{5, 7}, int_value);
  EXPECT_EQ(range_list.range_list_.size(), 2u);
  range_list.SetRangeValue(Range{3, 5}, int_value);
  EXPECT_EQ(range_list.range_list_.size(), 1u);
  auto result = range_list.range_list_[0].first == Range{1, 7};
  ASSERT_TRUE(result);

  range_list.Clear();
  range_list.SetRangeValue(Range{1, 2}, int_value);
  range_list.SetRangeValue(Range{5, 6}, int_value);
  EXPECT_EQ(range_list.range_list_.size(), 2u);
  range_list.SetRangeValue(Range{3, 5}, int_value);
  EXPECT_EQ(range_list.range_list_.size(), 2u);
  result = range_list.range_list_[0].first == Range{1, 2};
  EXPECT_TRUE(result);
  result = range_list.range_list_[1].first == Range{3, 6};
  EXPECT_TRUE(result);

  range_list.Clear();
  auto int_value2 = 5;
  range_list.SetRangeValue(Range{1, 2}, int_value);
  range_list.SetRangeValue(Range{5, 6}, int_value2);
  EXPECT_EQ(range_list.range_list_.size(), 2u);
  range_list.SetRangeValue(Range{3, 5}, int_value);
  EXPECT_EQ(range_list.range_list_.size(), 3u);
  result = range_list.range_list_[0].first == Range{1, 2};
  EXPECT_TRUE(result);
  result = range_list.range_list_[1].first == Range{3, 5};
  EXPECT_TRUE(result);
  result = range_list.range_list_[2].first == Range{5, 6};
  EXPECT_TRUE(result);

  range_list.Clear();
}

TEST(StyleManager, CopyConstructor) {
  StyleManager original;
  TTColor color(TTColor::BLUE());
  original.SetForegroundColor(color, 1, 5);
  StyleManager copy(original);
  EXPECT_EQ(copy.GetForegroundColor(2), color);
}

TEST(StyleManager, SetParagraphStyle) {
  StyleManager manager;
  TTColor fg_color(TTColor::RED());
  Style style;
  style.SetForegroundColor(fg_color);
  manager.SetParagraphStyle(style);
  EXPECT_EQ(manager.GetForegroundColor(0), fg_color);
}

TEST(StyleManager, AttributeGettersAndSetters) {
  Style default_style = Style::DefaultStyle();
  StyleManager manager;
  manager.SetParagraphStyle(default_style);
  // Test ForegroundColor
  const TTColor fg_color(TTColor::RED());
  manager.SetForegroundColor(fg_color, 1, 5);
  EXPECT_EQ(manager.GetForegroundColor(0), default_style.GetForegroundColor());
  EXPECT_EQ(manager.GetForegroundColor(1), fg_color);
  EXPECT_EQ(manager.GetForegroundColor(5), fg_color);
  EXPECT_EQ(manager.GetForegroundColor(6), default_style.GetForegroundColor());
  // Test BackgroundColor
  const TTColor bg_color(TTColor::GREEN());
  manager.SetBackgroundColor(bg_color, 1, 5);
  EXPECT_EQ(manager.GetBackgroundColor(0), default_style.GetBackgroundColor());
  EXPECT_EQ(manager.GetBackgroundColor(1), bg_color);
  EXPECT_EQ(manager.GetBackgroundColor(5), bg_color);
  EXPECT_EQ(manager.GetBackgroundColor(6), default_style.GetBackgroundColor());
  // Test DecorationColor
  const TTColor dec_color(TTColor::BLUE());
  manager.SetDecorationColor(dec_color, 1, 5);
  EXPECT_EQ(manager.GetDecorationColor(0), default_style.GetDecorationColor());
  EXPECT_EQ(manager.GetDecorationColor(1), dec_color);
  EXPECT_EQ(manager.GetDecorationColor(5), dec_color);
  EXPECT_EQ(manager.GetDecorationColor(6), default_style.GetDecorationColor());
  // Test DecorationType
  const DecorationType dec_type = DecorationType::kUnderLine;
  manager.SetDecorationType(dec_type, 1, 5);
  EXPECT_EQ(manager.GetDecorationType(0), default_style.GetDecorationType());
  EXPECT_EQ(manager.GetDecorationType(1), dec_type);
  EXPECT_EQ(manager.GetDecorationType(5), dec_type);
  EXPECT_EQ(manager.GetDecorationType(6), default_style.GetDecorationType());
  // Test DecorationThicknessMultiplier
  const float thickness = 2.5f;
  manager.SetDecorationThicknessMultiplier(thickness, 1, 5);
  EXPECT_EQ(manager.GetDecorationThicknessMultiplier(0),
            default_style.GetDecorationThicknessMultiplier());
  EXPECT_EQ(manager.GetDecorationThicknessMultiplier(1), thickness);
  EXPECT_EQ(manager.GetDecorationThicknessMultiplier(5), thickness);
  EXPECT_EQ(manager.GetDecorationThicknessMultiplier(6),
            default_style.GetDecorationThicknessMultiplier());
  // Test ForegroundPainter
  Painter fg_painter;
  manager.SetForegroundPainter(&fg_painter, 1, 5);
  EXPECT_EQ(manager.GetForegroundPainter(0),
            default_style.GetForegroundPainter());
  EXPECT_EQ(manager.GetForegroundPainter(1), &fg_painter);
  EXPECT_EQ(manager.GetForegroundPainter(5), &fg_painter);
  EXPECT_EQ(manager.GetForegroundPainter(6),
            default_style.GetForegroundPainter());
  // Test BackgroundPainter
  Painter bg_painter;
  manager.SetBackgroundPainter(&bg_painter, 1, 5);
  EXPECT_EQ(manager.GetBackgroundPainter(0),
            default_style.GetBackgroundPainter());
  EXPECT_EQ(manager.GetBackgroundPainter(1), &bg_painter);
  EXPECT_EQ(manager.GetBackgroundPainter(5), &bg_painter);
  EXPECT_EQ(manager.GetBackgroundPainter(6),
            default_style.GetBackgroundPainter());
  // Test WordBreak
  const WordBreakType word_break = WordBreakType::kBreakAll;
  manager.SetWordBreak(word_break, 1, 5);
  EXPECT_EQ(manager.GetWordBreak(0), default_style.GetWordBreak());
  EXPECT_EQ(manager.GetWordBreak(1), word_break);
  EXPECT_EQ(manager.GetWordBreak(5), word_break);
  EXPECT_EQ(manager.GetWordBreak(6), default_style.GetWordBreak());
  // Test BaselineOffset
  const float baseline_offset = 1.5f;
  manager.SetBaselineOffset(baseline_offset, 1, 5);
  EXPECT_EQ(manager.GetBaselineOffset(0), default_style.GetBaselineOffset());
  EXPECT_EQ(manager.GetBaselineOffset(1), baseline_offset);
  EXPECT_EQ(manager.GetBaselineOffset(5), baseline_offset);
  EXPECT_EQ(manager.GetBaselineOffset(6), default_style.GetBaselineOffset());
  // Test TextShadowList
  TextShadow shadow;
  StyleManager::TextShadowList expected_shadow_list{shadow};
  manager.SetTextShadowList(expected_shadow_list, 1, 5);
  EXPECT_EQ(manager.GetTextShadowList(0).size(), 0u);
  const auto& shadow_list = manager.GetTextShadowList(1);
  ASSERT_EQ(shadow_list.size(), 1u);
  EXPECT_EQ(shadow_list[0].color_, shadow.color_);
  EXPECT_EQ(shadow_list[0].offset_[0], shadow.offset_[0]);
  EXPECT_EQ(shadow_list[0].offset_[1], shadow.offset_[1]);
  EXPECT_EQ(shadow_list[0].blur_radius_, shadow.blur_radius_);
}

TEST(StyleManager, GetStyle) {
  StyleManager manager;
  TTColor fg_color(TTColor::RED());
  TTColor bg_color(TTColor::GREEN());
  LineType line_type = LineType::kDashed;
  manager.SetForegroundColor(fg_color, 1, 5);
  manager.SetBackgroundColor(bg_color, 1, 5);
  manager.SetDecorationStyle(line_type, 1, 5);

  Style style = manager.GetStyle(3);
  EXPECT_EQ(style.GetForegroundColor(), fg_color);
  EXPECT_EQ(style.GetBackgroundColor(), bg_color);
  EXPECT_EQ(style.GetDecorationStyle(), line_type);
}

TEST(StyleManager, ApplyStyleInRange) {
  const TTColor color(TTColor::RED());
  Style style;
  style.SetForegroundColor(color);
  {
    // Test normal valid range
    StyleManager manager;
    manager.ApplyStyleInRange(style, 5, 5);
    EXPECT_EQ(manager.GetForegroundColor(4), Style().GetForegroundColor());
    EXPECT_EQ(manager.GetForegroundColor(5), color);
    EXPECT_EQ(manager.GetForegroundColor(9), color);
    EXPECT_EQ(manager.GetForegroundColor(10), Style().GetForegroundColor());
  }
  {
    // Test boundary conditions
    StyleManager manager;
    manager.ApplyStyleInRange(style, 0, 3);
    EXPECT_EQ(manager.GetForegroundColor(0), color);
    EXPECT_EQ(manager.GetForegroundColor(2), color);
    EXPECT_EQ(manager.GetForegroundColor(3), Style().GetForegroundColor());
  }
  {
    // Apply at a very large index (should handle gracefully)
    StyleManager manager;
    const uint32_t large_index = Range::MaxIndex() - 10;
    manager.ApplyStyleInRange(style, large_index, 20);
    EXPECT_EQ(manager.GetForegroundColor(large_index), color);
    EXPECT_EQ(manager.GetForegroundColor(Range::MaxIndex() - 1), color);
    EXPECT_EQ(manager.GetForegroundColor(Range::MaxIndex()),
              Style().GetForegroundColor());
    EXPECT_EQ(manager.GetForegroundColor(0), Style().GetForegroundColor());
  }
  {
    // Test overlapping ranges
    StyleManager manager;
    const TTColor color2(TTColor::GREEN());
    Style style2;
    style2.SetForegroundColor(color2);
    manager.ApplyStyleInRange(style, 5, 7);
    manager.ApplyStyleInRange(style2, 7, 5);

    // Verify the overlapped region has the new style
    EXPECT_EQ(manager.GetForegroundColor(5), color);
    EXPECT_EQ(manager.GetForegroundColor(6), color);
    EXPECT_EQ(manager.GetForegroundColor(7), color2);
    EXPECT_EQ(manager.GetForegroundColor(11), color2);
    EXPECT_EQ(manager.GetForegroundColor(12), Style().GetForegroundColor());
  }
  {
    // Test multiple attributes
    //            0   1   2   3   4   5   6
    // Background |   |<---Red--->)   |   |
    // Thickness  |   |   |<---2.0--->)   |
    // Decoration |<------Underline------>)
    StyleManager manager;
    const TTColor bg_color(TTColor::RED());
    const float thickness = 2.0f;
    const DecorationType dec_type = DecorationType::kUnderLine;
    Style style1;
    style1.SetBackgroundColor(bg_color);
    manager.ApplyStyleInRange(style1, 1, 3);
    Style style2;
    style2.SetDecorationThicknessMultiplier(thickness);
    manager.ApplyStyleInRange(style2, 2, 3);
    Style style3;
    style3.SetDecorationType(dec_type);
    manager.ApplyStyleInRange(style3, 0, 6);

    const std::vector<std::tuple<TTColor, float, DecorationType>>
        expected_attributes = {
            {TTColor(), 1.f, DecorationType::kUnderLine},
            {bg_color, 1.f, DecorationType::kUnderLine},
            {bg_color, thickness, DecorationType::kUnderLine},
            {bg_color, thickness, DecorationType::kUnderLine},
            {TTColor(), thickness, DecorationType::kUnderLine},
            {TTColor(), 1.f, DecorationType::kUnderLine},
            {TTColor(), 1.f, DecorationType::kNone},
        };
    for (size_t i = 0; i < expected_attributes.size(); i++) {
      auto [color, thickness, dec_type] = expected_attributes[i];
      EXPECT_EQ(manager.GetBackgroundColor(i), color);
      EXPECT_EQ(manager.GetDecorationThicknessMultiplier(i), thickness);
      EXPECT_EQ(manager.GetDecorationType(i), dec_type);
    }
  }
}

TEST(StyleManager, ClearStyleInRange) {
  StyleManager manager;
  const AttributeType fg_attr = AttributeType::kForegroundColor;
  const TTColor fg_color(TTColor::RED());
  const Range range{2, 5};
  const TTColor default_color = Style::DefaultStyle().GetForegroundColor();
  // Clear when StyleManager is empty
  Range clear_range{};
  clear_range = range;
  manager.ClearStyleInRange(fg_attr, clear_range);
  // Clear an existing range
  manager.SetForegroundColor(fg_color, range.GetStart(), range.GetLength());
  clear_range = range;
  manager.ClearStyleInRange(fg_attr, clear_range);
  EXPECT_EQ(manager.GetForegroundColor(range.GetStart()), default_color);
  EXPECT_EQ(manager.GetForegroundColor(range.GetEnd() - 1), default_color);
  // Clear a superset of an existing range
  manager.SetForegroundColor(fg_color, range.GetStart(), range.GetLength());
  clear_range = Range{1, 6};
  manager.ClearStyleInRange(fg_attr, clear_range);
  EXPECT_EQ(manager.GetForegroundColor(range.GetStart()), default_color);
  EXPECT_EQ(manager.GetForegroundColor(range.GetEnd() - 1), default_color);
  // Clear a subset of an existing range
  manager.SetForegroundColor(fg_color, range.GetStart(), range.GetLength());
  clear_range = Range{3, 5};
  manager.ClearStyleInRange(fg_attr, clear_range);
  EXPECT_EQ(manager.GetForegroundColor(range.GetStart()), fg_color);
  EXPECT_EQ(manager.GetForegroundColor(range.GetEnd() - 1), default_color);
  // Clear a non-existing range
  manager.SetForegroundColor(fg_color, range.GetStart(), range.GetLength());
  clear_range = Range{6, 10};
  manager.ClearStyleInRange(fg_attr, clear_range);
  EXPECT_EQ(manager.GetForegroundColor(range.GetStart()), fg_color);
  EXPECT_EQ(manager.GetForegroundColor(range.GetEnd() - 1), fg_color);
}

TEST(StyleManager, GetStyleRange) {
  //            0   1   2   3   4   5   6   7   8
  // Foreground |   |<---Red--->)   |   |   |   |   // range1
  // Thickness  |   |   |   |   |   |<---2.0--->) . // range2
  // Decoration |   |   |<----Underline---->)   | . // range3
  StyleManager manager;

  // Test single attribute range
  const Range range1{1, 4};
  const TTColor fg_color(TTColor::RED());
  manager.SetForegroundColor(fg_color, range1.GetStart(), range1.GetLength());
  StyleRange style_range;
  manager.GetStyleRange(&style_range, range1.GetStart());
  EXPECT_EQ(style_range.GetRange(), range1);
  EXPECT_EQ(style_range.GetStyle().GetForegroundColor(), fg_color);
  // Test two non-overlapping ranges
  const Range range2{5, 8};
  const float thickness = 2.f;
  manager.SetDecorationThicknessMultiplier(thickness, range2.GetStart(),
                                           range2.GetLength());
  manager.GetStyleRange(&style_range, range2.GetStart());
  EXPECT_EQ(style_range.GetRange(), range2);
  EXPECT_EQ(style_range.GetStyle().GetDecorationThicknessMultiplier(),
            thickness);
  EXPECT_NE(style_range.GetStyle().GetForegroundColor(), fg_color);
  // Test multiple attributes with overlapping ranges
  const Range range3{2, 7};
  const DecorationType underline = DecorationType::kUnderLine;
  manager.SetDecorationType(underline, range3.GetStart(), range3.GetLength());
  manager.GetStyleRange(&style_range, range3.GetStart());
  EXPECT_EQ(style_range.GetRange(),
            (Range{range3.GetStart(), range1.GetEnd()}));
  EXPECT_EQ(style_range.GetStyle().GetForegroundColor(), fg_color);
  EXPECT_EQ(style_range.GetStyle().GetDecorationType(), underline);
  EXPECT_NE(style_range.GetStyle().GetDecorationThicknessMultiplier(),
            thickness);
}

TEST(StyleManager, ExtraFloatAttributes) {
  StyleManager manager;
  AttributeType float_attr = AttributeType::kExtraBaselineOffset;
  EXPECT_FLOAT_EQ((manager.GetExtraFloatAttributes(float_attr, 0)), 0.f);

  const float offset = 0.5f;
  const Range range{1, 4};
  manager.SetExtraFloatAttributesInRange(float_attr, offset, range.GetStart(),
                                         range.GetLength());
  EXPECT_FLOAT_EQ(manager.GetExtraFloatAttributes(float_attr, range.GetStart()),
                  offset);
  EXPECT_FLOAT_EQ(
      manager.GetExtraFloatAttributes(float_attr, range.GetEnd() - 1), offset);
  EXPECT_FLOAT_EQ(manager.GetExtraFloatAttributes(float_attr, range.GetEnd()),
                  0.f);
  manager.ClearExtraAttributes(float_attr);
  EXPECT_FLOAT_EQ(manager.GetExtraFloatAttributes(float_attr, range.GetEnd()),
                  0.f);
}
}  // namespace tttext
}  // namespace ttoffice
