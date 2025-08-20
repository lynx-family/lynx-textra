// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/paragraph_style.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <cstdint>
#include <memory>
#include <utility>

#include "mocks.h"
#include "src/textlayout/style_attributes.h"
#include "test_utils.h"

using namespace ::testing;

namespace ttoffice {
namespace tttext {
class TextLayoutTest : public ::testing::Test {
 public:
  std::unique_ptr<MockTTShaper> GetFixedSizeMockShaper() {
    // FontInfo always returns -0.75 ascent and 0.25 descent, multiplied by
    // font_size
    auto mock_typeface = std::make_shared<NiceMock<MockTypefaceHelper>>();
    ON_CALL(*mock_typeface, OnCreateFontInfo(_, _))
        .WillByDefault(Invoke([](FontInfo* info, float font_size) {
          *info = FontInfo(-0.75 * font_size, 0.25 * font_size, font_size);
        }));

    auto test_fontmgr = std::make_shared<TestFontMgr>(
        std::vector<std::shared_ptr<ITypefaceHelper>>{mock_typeface});
    auto mock_shaper = std::make_unique<NiceMock<MockTTShaper>>(
        FontmgrCollection{test_fontmgr});
    ON_CALL(*mock_shaper, OnShapeText(_, _))
        .WillByDefault(
            Invoke([mock_typeface](const ShapeKey& key, ShapeResult* result) {
              // Each character takes a fixed space (font_size x font_size)
              const size_t char_count = key.text_.size();
              TestShapingResultReader reader(char_count);
              for (size_t i = 0; i < char_count; ++i) {
                reader.glyphs_[i] = i;
                const float font_size = key.style_.GetFontSize();
                reader.advances_[i] = {font_size, font_size};
              }
              reader.font_ = mock_typeface;
              result->AppendPlatformShapingResult(reader);
            }));
    return mock_shaper;
  }

  // A helper method to set up a Paragraph with the specified content and lay it
  // out in LayoutRegions (with the specified with/height and modes), and
  // returns the layout results.
  std::pair<std::vector<LayoutResult>,
            std::vector<std::unique_ptr<LayoutRegion>>>
  LayoutContentWithRegionParams(float width, float height,
                                LayoutMode width_mode, LayoutMode height_mode,
                                const std::string& content) {
    TextLayout layout(GetFixedSizeMockShaper());
    TTTextContext context;
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(1.f);
    para->AddTextRun(&style, content.c_str());

    std::vector<LayoutResult> results;
    std::vector<std::unique_ptr<LayoutRegion>> regions;

    LayoutResult last_result = LayoutResult::kBreakPage;
    while (last_result == LayoutResult::kBreakPage) {
      std::unique_ptr<LayoutRegion> new_region = std::make_unique<LayoutRegion>(
          width, height, width_mode, height_mode);
      last_result = layout.Layout(para.get(), new_region.get(), context);
      results.push_back(last_result);
      regions.push_back(std::move(new_region));
      context.SetLayoutBottom(0);
    }
    return std::make_pair(std::move(results), std::move(regions));
  };
};

TEST_F(TextLayoutTest, DifferentLayoutModes) {
  const float page_width = 3.f;
  const float page_height = 2.f;
  {
    // AtMost mode
    {
      // 2 chars -> dimension 2 x 1
      auto [results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kAtMost, LayoutMode::kAtMost,
          "01");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results[0], LayoutResult::kNormal);
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 1u);
      EXPECT_FLOAT_EQ(regions[0]->GetLayoutedWidth(), 2.f);
      EXPECT_FLOAT_EQ(regions[0]->GetLayoutedHeight(), 1.f);
    }
    {
      // 5 chars -> dimension 3 x 2
      auto [results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kAtMost, LayoutMode::kAtMost,
          "01234");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results, std::vector{LayoutResult::kNormal});
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 2u);
      EXPECT_FLOAT_EQ(regions[0]->GetLayoutedWidth(), page_width);
      EXPECT_FLOAT_EQ(regions[0]->GetLayoutedHeight(), page_height);
    }
  }
  {
    // Definite mode
    {
      // 2 chars -> 1 page, 1 lines
      auto [results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kDefinite, LayoutMode::kDefinite,
          "01");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results[0], LayoutResult::kNormal);
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 1u);
    }
    {
      // 5 chars -> 1 page, 2 lines
      auto [results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kDefinite, LayoutMode::kDefinite,
          "01234");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results, std::vector{LayoutResult::kNormal});
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 2u);
    }
    {
      // 10 chars -> 2 regions, 6 chars on first page and 4 chars on second page
      auto [results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kDefinite, LayoutMode::kDefinite,
          "0123456789");
      EXPECT_EQ(results.size(), 2u);
      EXPECT_EQ(results,
                (std::vector{LayoutResult::kBreakPage, LayoutResult::kNormal}));
      EXPECT_EQ(regions.size(), 2u);
      EXPECT_EQ(regions[0]->GetLineCount(), 2u);
      EXPECT_EQ(regions[1]->GetLineCount(), 2u);
    }
  }
  // Note: LayoutMode::kIndefinite currently not supported
}

TEST_F(TextLayoutTest, ParagraphStyle_DefaultStyle) {
  const float text_size = 15.f;
  const TTColor text_color(TTColor::BLUE());
  ParagraphStyle para_style;
  Style style;
  style.SetTextSize(text_size);
  style.SetForegroundColor(text_color);
  para_style.SetDefaultStyle(style);
  EXPECT_EQ(para_style.GetDefaultStyle().GetTextSize(), text_size);
  EXPECT_EQ(para_style.GetDefaultStyle().GetForegroundColor(), text_color);
}

TEST_F(TextLayoutTest, ParagraphStyle_HorizontalAlignment) {
  const char* text = "Hello world!";  // 12 characters, each 1 x 1 size
  const float page_width = 10.5f;     // Line fits 10 characters + 0.5 spacing

  const auto layout_helper = [this, page_width,
                              text](ParagraphHorizontalAlignment alignment) {
    auto para = std::make_unique<ParagraphImpl>();
    ParagraphStyle para_style;
    Style style;
    style.SetTextSize(1.f);
    para_style.SetDefaultStyle(style);
    para_style.SetHorizontalAlign(alignment);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(nullptr, text);
    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, 10.f);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kLeft);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(second_line->GetLineLeft(), 0.f);
    EXPECT_LT(first_line->GetLineRight(), page_width);
    EXPECT_LT(second_line->GetLineRight(), page_width);
  }
  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kCenter);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(
        (first_line->GetLineLeft() + first_line->GetLineRight()) / 2,
        page_width / 2);
    EXPECT_FLOAT_EQ(
        (second_line->GetLineLeft() + second_line->GetLineRight()) / 2,
        page_width / 2);
    EXPECT_GT(first_line->GetLineLeft(), 0.f);
    EXPECT_GT(second_line->GetLineLeft(), 0.f);
    EXPECT_LT(first_line->GetLineRight(), page_width);
    EXPECT_LT(second_line->GetLineRight(), page_width);
  }
  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kRight);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_GT(first_line->GetLineLeft(), 0.f);
    EXPECT_GT(second_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(first_line->GetLineRight(), page_width);
    EXPECT_FLOAT_EQ(second_line->GetLineRight(), page_width);
  }
  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kJustify);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(second_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(first_line->GetLineRight(), page_width);
    // TODO: Fix implementation and uncomment the test below
    // (Also see ParagraphStyle_LastLineFollowHorizontalAlign)
    // EXPECT_LT(second_line->GetLineRight(), page_width);
  }
  // Note: kDistributed currently not implemented
}

TEST_F(TextLayoutTest, ParagraphStyle_VerticalAlignment) {
  // Note: vertical_alignment_ currently not used and always align by baseline.
}

TEST_F(TextLayoutTest, ParagraphStyle_NumPr) {
  // Note: num_pr_ currently not used
}

TEST_F(TextLayoutTest, ParagraphStyle_Tabs) {
  // Note: tabs_ currently not used
}

TEST_F(TextLayoutTest, ParagraphStyle_Indent) {
  const char* ltr_text = "abcd";  // 4 characters, each 1 x 1 size
  const float page_width = 3.6f;  // Line fits 3 characters + 0.3 spacing
  const float page_height = 10.0f;
  const float text_size = 1.1f;
  const float start_indent = 0.2f;
  const int start_indent_chars = 2;
  const float end_indent = 0.4f;
  const int end_indent_chars = 1;

  auto layout_helper = [this, ltr_text, page_width, page_height, text_size](
                           ParagraphStyle& para_style,
                           WriteDirection direction = WriteDirection::kLTR) {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(text_size);
    para_style.SetDefaultStyle(style);
    para_style.SetWriteDirection(direction);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(nullptr, ltr_text);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);

    return std::make_pair(std::move(para), std::move(region));
  };

  auto check_indents = [page_width](
                           const LayoutRegion& region, float first_line_start,
                           float first_line_end, float hanging_line_start,
                           float hanging_line_end) {
    auto start_char_rect = [](const TextLine& line) {
      float rect[4];
      line.GetCharBoundingRect(rect, line.GetStartCharPos());
      return RectF::MakeLTWH(rect[0], rect[1], rect[2], rect[3]);
    };
    auto end_char_rect = [](const TextLine& line) {
      float rect[4];
      line.GetCharBoundingRect(rect, line.GetEndCharPos());
      return RectF::MakeLTWH(rect[0], rect[1], rect[2], rect[3]);
    };

    EXPECT_EQ(region.GetLineCount(), 2u);
    EXPECT_FLOAT_EQ(region.GetLine(0)->GetStartIndent(), first_line_start);
    EXPECT_FLOAT_EQ(region.GetLine(1)->GetStartIndent(), hanging_line_start);
    EXPECT_FLOAT_EQ(region.GetLine(0)->GetEndIndent(), first_line_end);
    EXPECT_FLOAT_EQ(region.GetLine(1)->GetEndIndent(), hanging_line_end);
    EXPECT_FLOAT_EQ(start_char_rect(*region.GetLine(0)).GetLeft(),
                    first_line_start);
    EXPECT_FLOAT_EQ(start_char_rect(*region.GetLine(1)).GetLeft(),
                    hanging_line_start);
    EXPECT_LE(end_char_rect(*region.GetLine(0)).GetRight() + first_line_end,
              page_width);
    EXPECT_LE(end_char_rect(*region.GetLine(1)).GetRight() + hanging_line_end,
              page_width);
  };

  {
    SCOPED_TRACE("Testing Indent::start_");
    // Note: start_ applies to hanging lines only (is this intentional?)
    ParagraphStyle para_style;
    para_style.SetStartIndentInPx(start_indent);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, 0.f, start_indent, 0.f);
  }
  {
    SCOPED_TRACE("Testing Indent::start_chars_");
    // Note: start_chars_ applies to hanging lines only (is this intentional?)
    ParagraphStyle para_style;
    para_style.SetStartIndentInCharCnt(start_indent_chars);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, 0.f, start_indent_chars * text_size, 0.f);
  }
  {
    SCOPED_TRACE("Testing Indent::end_");
    ParagraphStyle para_style;
    para_style.SetEndIndentInPx(end_indent);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, end_indent, 0.f, end_indent);
  }
  {
    SCOPED_TRACE("Testing Indent::end_chars_");
    ParagraphStyle para_style;
    para_style.SetEndIndentInCharCnt(end_indent_chars);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, end_indent_chars * text_size, 0.f,
                  end_indent_chars * text_size);
  }
  {
    SCOPED_TRACE("Testing Indent::first_line_");
    ParagraphStyle para_style;
    para_style.SetFirstLineIndentInPx(start_indent);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, start_indent, 0.f, 0.f, 0.f);
  }
  {
    SCOPED_TRACE("Testing Indent::first_line_chars_");
    ParagraphStyle para_style;
    para_style.SetFirstLineIndentInCharCnt(start_indent_chars);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, start_indent_chars * text_size, 0.f, 0.f, 0.f);
  }
  {
    SCOPED_TRACE("Testing Indent::hanging_");
    ParagraphStyle para_style;
    para_style.SetHangingIndentInPx(start_indent);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, 0.f, start_indent, 0.f);
  }
  {
    SCOPED_TRACE("Testing Indent::hanging_chars_");
    ParagraphStyle para_style;
    para_style.SetHangingIndentInCharCnt(start_indent_chars);
    auto [_, region] = layout_helper(para_style);
    check_indents(*region, 0.f, 0.f, start_indent_chars * text_size, 0.f);
  }

  // TODO: add RTL text tests
}

TEST_F(TextLayoutTest, ParagraphStyle_Spacing) {
  const char* text = "This is a test paragraph spanning multiple lines of text";
  const float page_width = 10.0f;
  const float page_height = 20.0f;
  const float text_size = 1.0f;
  const float line_percent = 1.5f;

  auto layout_helper = [this, text, page_width, page_height,
                        text_size](const Spacing& spacing) {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(text_size);
    ParagraphStyle para_style;
    para_style.SetDefaultStyle(style);
    para_style.SetSpacing(spacing);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(nullptr, text);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);

    return std::make_pair(std::move(para), std::move(region));
  };

  {
    SCOPED_TRACE("Test default spacing");
    Spacing spacing;
    auto [_, region] = layout_helper(spacing);
    ASSERT_GT(region->GetLineCount(), 1u);
    float line0_y = region->GetLine(0)->GetLineBaseLine();
    float line1_y = region->GetLine(1)->GetLineBaseLine();
    EXPECT_FLOAT_EQ(line1_y - line0_y, text_size);
  }

  {
    SCOPED_TRACE("Test RulerType::kAuto and line_percent_");
    Spacing spacing;
    spacing.line_percent_ = line_percent;
    spacing.line_rule_ = RulerType::kAuto;
    auto [_, region] = layout_helper(spacing);
    ASSERT_GT(region->GetLineCount(), 1u);
    float line0_y = region->GetLine(0)->GetLineBaseLine();
    float line1_y = region->GetLine(1)->GetLineBaseLine();
    EXPECT_FLOAT_EQ(line1_y - line0_y, text_size * line_percent);
  }

  {
    SCOPED_TRACE("Test RulerType::kExact and line_px_");
    Spacing spacing;
    spacing.line_px_ = 2.f;
    spacing.line_rule_ = RulerType::kExact;
    auto [_, region] = layout_helper(spacing);
    ASSERT_GT(region->GetLineCount(), 1u);
    float line0_y = region->GetLine(0)->GetLineBaseLine();
    float line1_y = region->GetLine(1)->GetLineBaseLine();
    EXPECT_FLOAT_EQ(line1_y - line0_y, spacing.line_px_);
  }

  {
    SCOPED_TRACE("Test RulerType::kAtLeast with line_px_ > text_size");
    Spacing spacing;
    spacing.line_px_ = 2 * text_size;
    spacing.line_rule_ = RulerType::kAtLeast;
    auto [_, region] = layout_helper(spacing);
    ASSERT_GT(region->GetLineCount(), 1u);
    float line0_y = region->GetLine(0)->GetLineBaseLine();
    float line1_y = region->GetLine(1)->GetLineBaseLine();
    EXPECT_FLOAT_EQ(line1_y - line0_y, spacing.line_px_);
  }

  {
    SCOPED_TRACE("Test RulerType::kAtLeast with line_px_ < text_size");
    Spacing spacing;
    spacing.line_px_ = 0.5f * text_size;
    spacing.line_rule_ = RulerType::kAtLeast;
    auto [_, region] = layout_helper(spacing);
    ASSERT_GT(region->GetLineCount(), 1u);
    float line0_y = region->GetLine(0)->GetLineBaseLine();
    float line1_y = region->GetLine(1)->GetLineBaseLine();
    EXPECT_FLOAT_EQ(line1_y - line0_y, text_size);
  }
  // Note: Spacing::after_auto_spacing_ and before_auto_spacing_ are unused
  // Note: Spacing::before_px_ and after_px_ are unused
}

// WriteDirection decides the position of the ellipsis
TEST_F(TextLayoutTest, ParagraphStyle_WriteDirection) {
  const char* text = "The quick brown fox jumps over the lazy dog.";
  const float page_width = 12.0f;
  const float page_height = 3.0f;
  const float text_size = 1.0f;
  const char* ellipsis = "...";

  // Helper to create a paragraph with text and a ghost run
  auto layout_helper = [this, text, text_size, page_width, page_height,
                        ellipsis](WriteDirection direction) {
    auto para = std::make_unique<ParagraphImpl>();

    Style style;
    style.SetTextSize(text_size);
    ParagraphStyle para_style;
    para_style.SetDefaultStyle(style);
    para_style.SetWriteDirection(direction);
    para_style.SetMaxLines(1);
    para_style.SetEllipsis(ellipsis);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(&style, text);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  {
    // Test LTR direction
    auto [para, region] = layout_helper(WriteDirection::kLTR);
    EXPECT_EQ(region->GetLineCount(), 1u);
    auto* line = region->GetLine(0);
    float text_run_rect[4];
    line->GetCharBoundingRect(text_run_rect, line->GetStartCharPos());
    // Expected output: "The quick..."
    EXPECT_FLOAT_EQ(text_run_rect[0], 0.f);
  }
  {
    // Test RTL direction
    auto [_, region] = layout_helper(WriteDirection::kRTL);
    EXPECT_EQ(region->GetLineCount(), 1u);
    auto* line = region->GetLine(0);
    float text_run_rect[4];
    line->GetCharBoundingRect(text_run_rect, line->GetStartCharPos());
    // Expected output: "...The quick"
    EXPECT_FLOAT_EQ(text_run_rect[0], strlen(ellipsis) * text_size);
  }
}

TEST_F(TextLayoutTest, ParagraphStyle_Ellipsis) {
  // Currently there's no suitable public API for pure unit testing
  // Currently relying on ParagraphImageTest::TestSupSub image tests
}

TEST_F(TextLayoutTest, ParagraphStyle_MaxLines) {
  const char* text = "0123456";            //   7 characters
  const float page_width = 2.0f;           // ÷ 2 characters per line
  const uint32_t expected_line_count = 4;  // = 4 lines
  const float page_height = 10.0f;
  const float text_size = 1.0f;

  auto layout_helper = [this, text, page_width, page_height,
                        text_size](int max_lines) {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(text_size);
    ParagraphStyle para_style;
    para_style.SetDefaultStyle(style);
    para_style.SetMaxLines(max_lines);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(nullptr, text);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);

    return std::make_pair(std::move(para), std::move(region));
  };

  // Test with no max_lines limit (0 means unlimited)
  auto [_0, region0] = layout_helper(0);
  EXPECT_EQ(region0->GetLineCount(), expected_line_count);
  // Test with max_lines = 1
  auto [_1, region1] = layout_helper(1);
  EXPECT_EQ(region1->GetLineCount(), 1u);
  // Test with max_lines = 2
  auto [_2, region2] = layout_helper(2);
  EXPECT_EQ(region2->GetLineCount(), 2u);
  // Test with max_lines = total lines (should be same as unlimited)
  auto [_3, region3] = layout_helper(expected_line_count);
  EXPECT_EQ(region3->GetLineCount(), expected_line_count);
  // Test with max_lines > total lines (should be same as unlimited)
  auto [_4, region4] = layout_helper(expected_line_count + 5);
  EXPECT_EQ(region4->GetLineCount(), expected_line_count);
}

TEST_F(TextLayoutTest, ParagraphStyle_LineHeightOverride) {
  const char* text = "This is a test paragraph\nwith multiple lines of text";
  const float page_width = 20.0f;
  const float page_height = 20.0f;
  // Our mock shaper returns FontInfo with ascent = -0.75 and descent = 0.25
  // So the metrics height is: |ascent| + descent = 0.75 + 0.25 = 1.0
  const float text_size = 2.0f;
  const float height_from_font_info = 1.0f * text_size;

  auto layout_helper = [this, text, page_width, page_height,
                        text_size](bool line_height_override) {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(text_size);
    ParagraphStyle para_style;
    para_style.SetDefaultStyle(style);
    para_style.SetLineHeightOverride(line_height_override);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(&style, text);
    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  {
    // Test with line_height_override = false
    auto [_, region] = layout_helper(false);
    ASSERT_GT(region->GetLineCount(), 1u);
    EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), height_from_font_info);
  }
  {
    // Test with line_height_override = true
    auto [_, region] = layout_helper(true);
    ASSERT_GT(region->GetLineCount(), 1u);
    EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), text_size);
  }
}

TEST_F(TextLayoutTest, ParagraphStyle_OverflowWrap) {
  const char* long_word = "abcdefghijklmnopqrstuvwxyz";  // 26 characters
  const float page_width = 10.0f;  // Each line fits 10 characters
  const uint32_t line_count = 3;   // Expected line count if breaking word
  const float page_height = 20.0f;
  const float text_size = 1.0f;

  auto layout_helper = [this, long_word, page_width, page_height,
                        text_size](OverflowWrap overflow_wrap) {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetTextSize(text_size);
    ParagraphStyle para_style;
    para_style.SetDefaultStyle(style);
    para_style.SetOverflowWrap(overflow_wrap);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(nullptr, long_word);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  {
    // In Normal mode, long word should not break
    auto [_, region] = layout_helper(OverflowWrap::kNormal);
    EXPECT_EQ(region->GetLineCount(), 1u);
    auto* line = region->GetLine(0);
    EXPECT_GT(line->GetLineRight(), page_width);
  }

  {
    // In BreakWord mode, long word should be break into several lines
    auto [_, region] = layout_helper(OverflowWrap::kBreakWord);
    EXPECT_EQ(region->GetLineCount(), line_count);
    for (size_t i = 0; i < region->GetLineCount(); ++i) {
      EXPECT_LE(region->GetLine(i)->GetLineRight(), page_width);
    }
  }
  {
    // In Anywhere mode, long word should be break into several lines
    auto [_, region] = layout_helper(OverflowWrap::kAnywhere);
    EXPECT_EQ(region->GetLineCount(), line_count);
    for (size_t i = 0; i < region->GetLineCount(); ++i) {
      EXPECT_LE(region->GetLine(i)->GetLineRight(), page_width);
    }
  }
}
}  // namespace tttext
}  // namespace ttoffice
