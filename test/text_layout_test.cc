// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/paragraph_style.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>

#include "mocks.h"
#include "src/textlayout/style_attributes.h"
#include "test_utils.h"

using namespace ::testing;

namespace ttoffice {
namespace tttext {

class MockInlineObject : public RunDelegate {
 public:
  MockInlineObject(float width, float ascent, float descent)
      : width_(width), ascent_(ascent), descent_(descent) {}

  float GetAscent() const override { return ascent_; }
  float GetDescent() const override { return descent_; }
  float GetAdvance() const override { return width_; }

 private:
  float width_;
  float ascent_;
  float descent_;
};

class HeightSensitiveLayoutRegion : public LayoutRegion {
 public:
  HeightSensitiveLayoutRegion() : LayoutRegion(100.f, 200.f) {}

  std::vector<std::array<float, 2>> GetRangeList(float* top, float range_height,
                                                 float start_indent,
                                                 float end_indent) override {
    ++range_query_count_;
    // The fallback after repeated queries keeps a broken implementation from
    // hanging the test while still exposing a non-converging relayout loop.
    const bool use_tall_range = range_height >= 40.f || range_query_count_ > 6;
    return {{start_indent, (use_tall_range ? 80.f : 100.f) - end_indent}};
  }

  int GetRangeQueryCount() const { return range_query_count_; }

 private:
  int range_query_count_ = 0;
};

class TextLayoutTest : public ::testing::Test {
 public:
  ::TypefaceRef CreateFixedSizeMockTypeface() {
    auto mock_typeface = std::make_shared<NiceMock<MockTypefaceHelper>>();
    testing::Mock::AllowLeak(mock_typeface.get());
    ON_CALL(*mock_typeface, OnCreateFontInfo(_, _))
        .WillByDefault(Invoke([](FontInfo* info, float font_size) {
          *info = FontInfo(-0.75 * font_size, 0.25 * font_size, font_size);
        }));
    ON_CALL(*mock_typeface, GetWidthBounds(_, _, _, _))
        .WillByDefault(Invoke([](float* rect_ltrb, GlyphID[],
                                 uint32_t glyph_count, float font_size) {
          if (glyph_count == 0) {
            rect_ltrb[0] = 0.f;
            rect_ltrb[1] = 0.f;
            rect_ltrb[2] = 0.f;
            rect_ltrb[3] = 0.f;
            return;
          }
          rect_ltrb[0] = 0.f;
          rect_ltrb[1] = -0.75f * font_size;
          rect_ltrb[2] = font_size * static_cast<float>(glyph_count);
          rect_ltrb[3] = 0.25f * font_size;
        }));
    ON_CALL(*mock_typeface, GetWidthBound(_, _, _))
        .WillByDefault(Invoke([](float* rect_ltwh, GlyphID, float font_size) {
          rect_ltwh[0] = 0.f;
          rect_ltwh[1] = -0.75f * font_size;
          rect_ltwh[2] = font_size;
          rect_ltwh[3] = font_size;
        }));
    return mock_typeface;
  }

  std::unique_ptr<MockTTShaper> CreateFixedSizeMockShaper(
      const ::TypefaceRef& typeface) {
    auto test_fontmgr = std::make_shared<TestFontMgr>(
        std::vector<std::shared_ptr<ITypefaceHelper>>{typeface});
    auto mock_shaper = std::make_unique<NiceMock<MockTTShaper>>(
        FontmgrCollection{test_fontmgr});
    ON_CALL(*mock_shaper, OnShapeText(_, _))
        .WillByDefault(
            Invoke([typeface](const ShapeKey& key, ShapeResult* result) {
              const size_t char_count = key.text_.size();
              TestShapingResultReader reader(char_count);
              for (size_t i = 0; i < char_count; ++i) {
                reader.glyphs_[i] = i;
                const float font_size = key.style_.GetFontSize();
                reader.advances_[i] = {font_size, font_size};
              }
              reader.font_ = typeface;
              result->AppendPlatformShapingResult(reader);
            }));
    return mock_shaper;
  }

  std::unique_ptr<MockTTShaper> GetFixedSizeMockShaper() {
    return CreateFixedSizeMockShaper(CreateFixedSizeMockTypeface());
  }

  // A helper method to set up a Paragraph with the specified content and lay it
  // out in LayoutRegions (with the specified with/height and modes), and
  // returns the layout results.
  std::tuple<std::unique_ptr<ParagraphImpl>, std::vector<LayoutResult>,
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
    return std::make_tuple(std::move(para), std::move(results),
                           std::move(regions));
  };
};

TEST_F(TextLayoutTest, DifferentLayoutModes) {
  const float page_width = 3.f;
  const float page_height = 2.f;
  {
    // AtMost mode
    {
      // 2 chars -> dimension 2 x 1
      auto [para, results, regions] = LayoutContentWithRegionParams(
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
      auto [para, results, regions] = LayoutContentWithRegionParams(
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
      auto [para, results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kDefinite, LayoutMode::kDefinite,
          "01");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results[0], LayoutResult::kNormal);
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 1u);
    }
    {
      // 5 chars -> 1 page, 2 lines
      auto [para, results, regions] = LayoutContentWithRegionParams(
          page_width, page_height, LayoutMode::kDefinite, LayoutMode::kDefinite,
          "01234");
      EXPECT_EQ(results.size(), 1u);
      EXPECT_EQ(results, std::vector{LayoutResult::kNormal});
      EXPECT_EQ(regions.size(), 1u);
      EXPECT_EQ(regions[0]->GetLineCount(), 2u);
    }
    {
      // 10 chars -> 2 regions, 6 chars on first page and 4 chars on second page
      auto [para, results, regions] = LayoutContentWithRegionParams(
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
  const TTColor text_color(TTColor::BLUE);
  ParagraphStyle para_style;
  Style style;
  style.SetTextSize(text_size);
  style.SetForegroundColor(text_color);
  para_style.SetDefaultStyle(style);
  EXPECT_EQ(para_style.GetDefaultStyle().GetTextSize(), text_size);
  EXPECT_EQ(para_style.GetDefaultStyle().GetForegroundColor(), text_color);
}

TEST_F(TextLayoutTest, DominantBaselineTopAlignsSmallerRun) {
  const auto layout_helper = [this](DominantBaseline baseline) {
    TextLayout layout(GetFixedSizeMockShaper());
    TTTextContext context;
    auto para = std::make_unique<ParagraphImpl>();
    ParagraphStyle para_style;
    Style default_style;
    default_style.SetTextSize(10.f);
    para_style.SetDefaultStyle(default_style);
    para_style.SetDominantBaseline(baseline);
    para->SetParagraphStyle(&para_style);
    Style large;
    large.SetTextSize(10.f);
    Style small;
    small.SetTextSize(5.f);
    para->AddTextRun(&large, "A");
    para->AddTextRun(&small, "B");
    auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  auto [para_default, region_default] =
      layout_helper(DominantBaseline::kAlphabetic);
  auto* line_default = region_default->GetLine(0);
  float rect_default[4]{};
  line_default->GetBoundingRectByCharRange(rect_default, 1, 2);

  auto [para_top, region_top] = layout_helper(DominantBaseline::kTop);
  auto* line_top = region_top->GetLine(0);
  float rect_top[4]{};
  line_top->GetBoundingRectByCharRange(rect_top, 1, 2);

  EXPECT_GT(rect_default[1], line_default->GetLineTop());
  EXPECT_FLOAT_EQ(rect_top[1], line_top->GetLineTop());
}

TEST_F(TextLayoutTest, GetCharBoundingRectIncludesItalicExtraWidth) {
  const float text_size = 10.f;
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(text_size);
  style.SetItalic(true);
  para->AddTextRun(&style, "A");

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  float rect[4]{};
  region->GetLine(0)->GetCharBoundingRect(rect, 0);

  const auto expected_extra_width =
      std::abs(FAKE_ITALIC_SKEW) * 0.75f * text_size;
  EXPECT_FLOAT_EQ(rect[2], text_size + expected_extra_width);
}

TEST_F(TextLayoutTest, GetCharBoundingRectIncludesTextSkewExtraWidth) {
  const float text_size = 10.f;
  const float text_skew = 0.2f;
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(text_size);
  style.SetTextSkew(text_skew);
  para->AddTextRun(&style, "A");

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  float rect[4]{};
  region->GetLine(0)->GetCharBoundingRect(rect, 0);

  const auto expected_extra_width = text_skew * 0.75f * text_size;
  EXPECT_FLOAT_EQ(rect[2], text_size + expected_extra_width);
}

TEST_F(TextLayoutTest, ParagraphStyle_HorizontalAlignment) {
  const char* text = "Hello world!";  // 12 characters, each 1 x 1 size
  const float page_width = 10.5f;     // Line fits 10 characters + 0.5 spacing

  const auto layout_helper =
      [this, page_width, text](ParagraphHorizontalAlignment alignment,
                               WriteDirection direction = WriteDirection::kLTR,
                               const char* content = nullptr,
                               bool use_real_shaper = false) {
        auto para = std::make_unique<ParagraphImpl>();
        ParagraphStyle para_style;
        Style style;
        style.SetTextSize(1.f);
        para_style.SetDefaultStyle(style);
        para_style.SetHorizontalAlign(alignment);
        para_style.SetWriteDirection(direction);
        para->SetParagraphStyle(&para_style);
        para->AddTextRun(nullptr, content == nullptr ? text : content);
        TTTextContext context;
        std::unique_ptr<TTShaper> shaper;
        if (use_real_shaper) {
          shaper = TestUtils::getRealShaper();
        } else {
          shaper = GetFixedSizeMockShaper();
        }
        TextLayout layout(std::move(shaper));
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
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kStart,
                                     WriteDirection::kLTR);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(second_line->GetLineLeft(), 0.f);
  }
  {
    auto [_, region] =
        layout_helper(ParagraphHorizontalAlignment::kEnd, WriteDirection::kLTR);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineRight(), page_width);
    EXPECT_FLOAT_EQ(second_line->GetLineRight(), page_width);
  }
  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kStart,
                                     WriteDirection::kRTL);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineRight(), page_width);
    EXPECT_FLOAT_EQ(second_line->GetLineRight(), page_width);
  }
  {
    auto [_, region] =
        layout_helper(ParagraphHorizontalAlignment::kEnd, WriteDirection::kRTL);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(second_line->GetLineLeft(), 0.f);
  }
  {
    const char* auto_rtl_text = "123 \xD7\x90\xD7\x91";
    auto [_, region] =
        layout_helper(ParagraphHorizontalAlignment::kStart,
                      WriteDirection::kAuto, auto_rtl_text, true);
    EXPECT_EQ(region->GetLineCount(), 1u);
    EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineRight(), page_width);
  }
  {
    const char* auto_rtl_text = "123 \xD7\x90\xD7\x91";
    auto [_, region] =
        layout_helper(ParagraphHorizontalAlignment::kEnd, WriteDirection::kAuto,
                      auto_rtl_text, true);
    EXPECT_EQ(region->GetLineCount(), 1u);
    EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineLeft(), 0.f);
  }
  {
    auto [_, region] = layout_helper(ParagraphHorizontalAlignment::kJustify);
    EXPECT_EQ(region->GetLineCount(), 2u);
    auto* first_line = region->GetLine(0);
    auto* second_line = region->GetLine(1);
    EXPECT_FLOAT_EQ(first_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(second_line->GetLineLeft(), 0.f);
    EXPECT_FLOAT_EQ(first_line->GetLineRight(),
                    5);  // remove trailing space white
    // TODO: Fix implementation and uncomment the test below
    // (Also see ParagraphStyle_LastLineFollowHorizontalAlign)
    // EXPECT_LT(second_line->GetLineRight(), page_width);
  }
  // Note: kDistributed currently not implemented
}

TEST_F(TextLayoutTest, GetCharPosByCoordinateX_RTL) {
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(1.f);
  ParagraphStyle para_style;
  para_style.SetDefaultStyle(style);
  para_style.SetWriteDirection(WriteDirection::kRTL);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&style, "abcd");

  TTTextContext context;
  TextLayout layout(
      TestUtils::CreateFixedBidiTestShaper({3, 2, 1, 0}, {1, 1, 1, 1}));
  auto region = std::make_unique<LayoutRegion>(10.f, 10.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  const auto left = line->GetLineLeft();
  const auto right = line->GetLineRight();

  EXPECT_EQ(line->GetCharPosByCoordinateX(left - 0.5f), 4u);
  EXPECT_EQ(line->GetCharPosByCoordinateX(left + 0.5f), 3u);
  EXPECT_EQ(line->GetCharPosByCoordinateX(left + 1.5f), 2u);
  EXPECT_EQ(line->GetCharPosByCoordinateX(left + 2.5f), 1u);
  EXPECT_EQ(line->GetCharPosByCoordinateX(left + 3.5f), 0u);
  EXPECT_EQ(line->GetCharPosByCoordinateX(right + 0.5f), 0u);
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

TEST_F(TextLayoutTest, InlineObjectRespectsAtLeastLineHeight) {
  constexpr float kObjectWidth = 20.f;
  constexpr float kObjectAscent = -6.f;
  constexpr float kObjectDescent = 4.f;
  constexpr float kLineHeight = 20.f;

  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  para_style.SetLineHeightInPxAtLeast(kLineHeight);
  para->SetParagraphStyle(&para_style);
  para->AddShapeRun(nullptr,
                    std::make_shared<MockInlineObject>(
                        kObjectWidth, kObjectAscent, kObjectDescent),
                    false);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 100.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);
  EXPECT_FLOAT_EQ(line->GetContentHeight(), kObjectDescent - kObjectAscent);
  EXPECT_FLOAT_EQ(line->GetLineHeight(), kLineHeight);
}

TEST_F(TextLayoutTest, ExactLineBoxOnlyExpandsInlineObjectLine) {
  constexpr float kLineHeight = 20.f;

  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style text_style;
  text_style.SetTextSize(40.f);
  para_style.SetDefaultStyle(text_style);
  para_style.SetLineHeightInPx(kLineHeight, RulerType::kExact);
  para_style.SetInlineVerticalAlignmentMode(
      InlineVerticalAlignmentMode::kLineBox);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&text_style, "first\nsecond", 12);
  para->AddShapeRun(&text_style,
                    std::make_shared<MockInlineObject>(20.f, -20.f, 10.f),
                    false);
  para->AddTextRun(&text_style, "\nthird", 6);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(1000.f, 200.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 3u);
  EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), kLineHeight);
  EXPECT_FLOAT_EQ(region->GetLine(1)->GetLineHeight(), 30.f);
  EXPECT_FLOAT_EQ(region->GetLine(2)->GetLineHeight(), kLineHeight);
  EXPECT_GE(region->GetLine(1)->GetLineBaseLine() - 20.f,
            region->GetLine(1)->GetLineTop());
  EXPECT_LE(region->GetLine(1)->GetLineBaseLine() + 10.f,
            region->GetLine(1)->GetLineBottom());
  EXPECT_LE(region->GetLine(0)->GetLineBottom(),
            region->GetLine(1)->GetLineTop());
  EXPECT_LE(region->GetLine(1)->GetLineBottom(),
            region->GetLine(2)->GetLineTop());
}

TEST_F(TextLayoutTest, ExactLineBoxKeepsInlineObjectOutOfTextMetrics) {
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style text_style;
  text_style.SetTextSize(20.f);
  para_style.SetDefaultStyle(text_style);
  para_style.SetLineHeightInPx(20.f, RulerType::kExact);
  para_style.SetInlineVerticalAlignmentMode(
      InlineVerticalAlignmentMode::kLineBox);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&text_style, "A");
  para->AddShapeRun(
      &text_style, std::make_shared<MockInlineObject>(10.f, -60.f, 0.f), false);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 100.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  EXPECT_FLOAT_EQ(region->GetLine(0)->GetContentHeight(), 20.f);
  EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), 65.f);
}

TEST_F(TextLayoutTest, ExactLineBoxJointlyFitsTopAndBottomObjects) {
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style text_style;
  text_style.SetTextSize(20.f);
  Style top_style = text_style;
  top_style.SetVerticalAlignment(CharacterVerticalAlignment::kTop);
  Style bottom_style = text_style;
  bottom_style.SetVerticalAlignment(CharacterVerticalAlignment::kBottom);
  para_style.SetDefaultStyle(text_style);
  para_style.SetLineHeightInPx(20.f, RulerType::kExact);
  para_style.SetInlineVerticalAlignmentMode(
      InlineVerticalAlignmentMode::kLineBox);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&text_style, "A");
  auto top_object = std::make_shared<MockInlineObject>(10.f, -40.f, 0.f);
  auto bottom_object = std::make_shared<MockInlineObject>(10.f, -40.f, 0.f);
  para->AddShapeRun(&top_style, top_object, false);
  para->AddShapeRun(&bottom_style, bottom_object, false);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 100.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  EXPECT_FLOAT_EQ(line->GetLineHeight(), 40.f);
  EXPECT_FLOAT_EQ(top_object->GetYOffset(), line->GetLineTop());
  EXPECT_FLOAT_EQ(bottom_object->GetYOffset(), line->GetLineBottom() - 40.f);
}

TEST_F(TextLayoutTest, ExactLineBoxRelayoutUsesResolvedObjectHeight) {
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style text_style;
  text_style.SetTextSize(20.f);
  para_style.SetDefaultStyle(text_style);
  para_style.SetLineHeightInPx(20.f, RulerType::kExact);
  para_style.SetInlineVerticalAlignmentMode(
      InlineVerticalAlignmentMode::kLineBox);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&text_style, "A");
  para->AddShapeRun(
      &text_style, std::make_shared<MockInlineObject>(10.f, -60.f, 0.f), false);
  para->AddTextRun(&text_style, "B");

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<HeightSensitiveLayoutRegion>();
  auto* region_ptr = region.get();
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  EXPECT_LE(region_ptr->GetRangeQueryCount(), 4);
  EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), 65.f);
}

TEST_F(TextLayoutTest, ExactLineBoxMatchesCssVerticalAlignGeometry) {
  constexpr float kFontSize = 40.f;
  constexpr float kLineHeight = 20.f;
  struct Expectation {
    CharacterVerticalAlignment alignment;
    float object_height;
    float line_height;
    float object_top;
  };
  const Expectation expectations[] = {
      {CharacterVerticalAlignment::kBaseLine, 10.f, 20.f, 10.f},
      {CharacterVerticalAlignment::kSuperScript, 10.f, 23.2f, 0.f},
      {CharacterVerticalAlignment::kSubScript, 10.f, 28.f, 18.f},
      {CharacterVerticalAlignment::kMiddle, 10.f, 20.f, 5.f},
      {CharacterVerticalAlignment::kTextTop, 10.f, 30.f, 0.f},
      {CharacterVerticalAlignment::kTextBottom, 10.f, 30.f, 20.f},
      {CharacterVerticalAlignment::kTop, 10.f, 20.f, 0.f},
      {CharacterVerticalAlignment::kBottom, 10.f, 20.f, 10.f},
      {CharacterVerticalAlignment::kBaseLine, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kSuperScript, 60.f, 73.2f, 0.f},
      {CharacterVerticalAlignment::kSubScript, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kMiddle, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kTextTop, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kTextBottom, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kTop, 60.f, 60.f, 0.f},
      {CharacterVerticalAlignment::kBottom, 60.f, 60.f, 0.f},
  };

  for (const auto& expectation : expectations) {
    SCOPED_TRACE(static_cast<int>(expectation.alignment));
    SCOPED_TRACE(expectation.object_height);
    auto para = std::make_unique<ParagraphImpl>();
    ParagraphStyle para_style;
    Style text_style;
    text_style.SetTextSize(kFontSize);
    Style object_style = text_style;
    object_style.SetVerticalAlignment(expectation.alignment);
    para_style.SetDefaultStyle(text_style);
    para_style.SetLineHeightInPx(kLineHeight, RulerType::kExact);
    para_style.SetInlineVerticalAlignmentMode(
        InlineVerticalAlignmentMode::kLineBox);
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(&text_style, "A", 1);
    auto object = std::make_shared<MockInlineObject>(
        10.f, -expectation.object_height, 0.f);
    para->AddShapeRun(&object_style, object, false);
    para->AddTextRun(&text_style, "A", 1);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(1000.f, 300.f);
    layout.Layout(para.get(), region.get(), context);

    ASSERT_EQ(region->GetLineCount(), 1u);
    EXPECT_NEAR(region->GetLine(0)->GetLineHeight(), expectation.line_height,
                0.001f);
    EXPECT_NEAR(object->GetYOffset() - region->GetLine(0)->GetLineTop(),
                expectation.object_top, 0.001f);
  }
}

TEST_F(TextLayoutTest,
       LineBoxSuperAndSubUseContainingInlineFontInsteadOfObjectHeight) {
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style parent_style;
  parent_style.SetTextSize(40.f);
  para_style.SetDefaultStyle(parent_style);
  para_style.SetLineHeightInPx(20.f, RulerType::kExact);
  para_style.SetInlineVerticalAlignmentMode(
      InlineVerticalAlignmentMode::kLineBox);
  para->SetParagraphStyle(&para_style);

  // The object is inside a nested 12 px inline, while the paragraph root is
  // 40 px. Its computed style is the containing inline style available after
  // the DOM has been flattened into TTText runs.
  Style containing_inline_style;
  containing_inline_style.SetTextSize(12.f);
  containing_inline_style.SetVerticalAlignment(
      CharacterVerticalAlignment::kSuperScript);
  Style sub_style = containing_inline_style;
  sub_style.SetVerticalAlignment(CharacterVerticalAlignment::kSubScript);

  para->AddTextRun(&parent_style, "x");
  para->AddShapeRun(&containing_inline_style,
                    std::make_shared<MockInlineObject>(20.f, -60.f, 0.f),
                    false);
  para->AddShapeRun(
      &sub_style, std::make_shared<MockInlineObject>(20.f, -60.f, 0.f), false);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(100.f, 100.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);
  const float baseline = line->GetLineBaseLine();

  float super_rect[4]{};
  float sub_rect[4]{};
  line->GetCharBoundingRect(super_rect, 1);
  line->GetCharBoundingRect(sub_rect, 2);

  // CSS super/sub use the parent inline's font metrics. With the current
  // fallback ratios, a 12 px inherited font shifts by -3.96 px / +2.4 px even
  // though the replaced inline itself is 60 px tall.
  EXPECT_NEAR(super_rect[1] - baseline, -63.96f, 0.001f);
  EXPECT_NEAR(sub_rect[1] - baseline, -57.6f, 0.001f);
  EXPECT_NEAR(line->GetLineHeight(), 66.36f, 0.001f);
}

TEST_F(TextLayoutTest, ExactLineHeightDoesNotExpandInlineObjectLine) {
  constexpr float kLineHeight = 20.f;

  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style text_style;
  text_style.SetTextSize(40.f);
  para_style.SetDefaultStyle(text_style);
  para_style.SetLineHeightInPx(kLineHeight, RulerType::kExact);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&text_style, "text", 4);
  para->AddShapeRun(&text_style,
                    std::make_shared<MockInlineObject>(20.f, -20.f, 10.f),
                    false);

  TTTextContext context;
  TextLayout layout(GetFixedSizeMockShaper());
  auto region = std::make_unique<LayoutRegion>(1000.f, 200.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  EXPECT_FLOAT_EQ(region->GetLine(0)->GetLineHeight(), kLineHeight);
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
    EXPECT_EQ(line->GetEndCharPos(), 9u);
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
    EXPECT_EQ(line->GetEndCharPos(), 9u);
  }
}

TEST_F(TextLayoutTest, ParagraphStyle_EllipsisMixedBidiStripsFromRtlStart) {
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(1.f);
  ParagraphStyle para_style;
  para_style.SetDefaultStyle(style);
  para_style.SetWriteDirection(WriteDirection::kRTL);
  para_style.SetMaxLines(1);
  para_style.SetEllipsis(".");
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&style, "abcdef");

  TextLayout layout(TestUtils::CreateFixedBidiTestShaper({5, 4, 2, 3, 1, 0},
                                                         {1, 1, 2, 2, 1, 1}));
  TTTextContext context;
  auto region = std::make_unique<LayoutRegion>(4.f, 3.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  float retained_char_rect[4];
  float stripped_char_rect[4];
  line->GetCharBoundingRect(retained_char_rect, 1);
  line->GetCharBoundingRect(stripped_char_rect, 3);

  EXPECT_FLOAT_EQ(retained_char_rect[2], 1.f);
  EXPECT_FLOAT_EQ(stripped_char_rect[2], 0.f);
  EXPECT_EQ(line->GetEndCharPos(), 3u);
}

TEST_F(TextLayoutTest, ParagraphStyle_EllipsisWiderThanIndentedRange) {
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(1.f);
  ParagraphStyle para_style;
  para_style.SetDefaultStyle(style);
  para_style.SetFirstLineIndentInPx(3.f);
  para_style.SetMaxLines(1);
  para_style.SetEllipsis("..");
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&style, "ab");

  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto region = std::make_unique<LayoutRegion>(4.f, 3.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  float text_rect[4];
  line->GetCharBoundingRect(text_rect, 0);
  EXPECT_FLOAT_EQ(text_rect[0], 3.f);
  EXPECT_FLOAT_EQ(text_rect[2], 1.f);
  EXPECT_EQ(line->GetEndCharPos(), 1u);
}

TEST_F(TextLayoutTest, ParagraphStyle_WriteDirectionUpdatesAfterLayout) {
  const char* text = "The quick brown fox jumps over the lazy dog.";
  const float page_width = 12.0f;
  const float page_height = 3.0f;
  const float text_size = 1.0f;
  const char* ellipsis = "...";

  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(text_size);
  auto& para_style = para->GetParagraphStyle();
  para_style.SetDefaultStyle(style);
  para_style.SetWriteDirection(WriteDirection::kLTR);
  para->AddTextRun(&style, text);

  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto initial_region = std::make_unique<LayoutRegion>(page_width, page_height);
  layout.Layout(para.get(), initial_region.get(), context);

  para_style.SetWriteDirection(WriteDirection::kRTL);
  para_style.SetMaxLines(1);
  para_style.SetEllipsis(ellipsis);

  TTTextContext rtl_context;
  auto rtl_region = std::make_unique<LayoutRegion>(page_width, page_height);
  layout.Layout(para.get(), rtl_region.get(), rtl_context);

  ASSERT_EQ(rtl_region->GetLineCount(), 1u);
  auto* line = rtl_region->GetLine(0);
  float text_run_rect[4];
  line->GetCharBoundingRect(text_run_rect, line->GetStartCharPos());
  EXPECT_FLOAT_EQ(text_run_rect[0], strlen(ellipsis) * text_size);
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

TEST_F(TextLayoutTest, TextLine_GetTightBoundingRectByCharRange_GlyphBounds) {
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(2.f);
  para->AddTextRun(&style, "ab");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);

  float rect[4]{};
  line->GetTightBoundingRectByCharRange(rect, 0, 2);
  EXPECT_FLOAT_EQ(rect[0], 0.f);
  EXPECT_FLOAT_EQ(rect[1], -1.5f);
  EXPECT_FLOAT_EQ(rect[2], 4.f);
  EXPECT_FLOAT_EQ(rect[3], 0.5f);

  line->GetTightBoundingRectByCharRange(rect, 1, 1);
  EXPECT_FLOAT_EQ(rect[0], 0.f);
  EXPECT_FLOAT_EQ(rect[1], 0.f);
  EXPECT_FLOAT_EQ(rect[2], 0.f);
  EXPECT_FLOAT_EQ(rect[3], 0.f);
}

TEST_F(TextLayoutTest,
       TextLine_GetTightBoundingRectByCharRange_ItalicMiddleCharBounds) {
  constexpr float text_size = 10.f;
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  Style style;
  style.SetTextSize(text_size);
  style.SetItalic(true);
  para->AddTextRun(&style, "ab");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);

  float rect[4]{};
  line->GetTightBoundingRectByCharRange(rect, 1, 2);
  EXPECT_FLOAT_EQ(rect[0], text_size * (1.f + 0.1875f));
  EXPECT_FLOAT_EQ(rect[1], -0.75f * text_size);
  EXPECT_FLOAT_EQ(rect[2], text_size * (2.f + 0.1875f));
  EXPECT_FLOAT_EQ(rect[3], 0.25f * text_size);
}

TEST_F(TextLayoutTest,
       TextLine_GetTightBoundingRectByCharRange_LineHeightModes_Consistent) {
  enum class Mode {
    kDefault,
    kExactLarge,
    kExactSmall,
    kAtLeastLarge,
    kAtLeastSmall,
    kPercentLarge,
    kPercentSmall,
  };

  auto layout_helper = [this](Mode mode) {
    TextLayout layout(GetFixedSizeMockShaper());
    TTTextContext context;
    auto para = std::make_unique<ParagraphImpl>();
    ParagraphStyle para_style;
    Style style;
    style.SetTextSize(2.f);
    para_style.SetDefaultStyle(style);
    switch (mode) {
      case Mode::kDefault:
        break;
      case Mode::kExactLarge:
        para_style.SetLineHeightInPxExact(20.f);
        break;
      case Mode::kExactSmall:
        para_style.SetLineHeightInPxExact(1.f);
        break;
      case Mode::kAtLeastLarge:
        para_style.SetLineHeightInPxAtLeast(20.f);
        break;
      case Mode::kAtLeastSmall:
        para_style.SetLineHeightInPxAtLeast(1.f);
        break;
      case Mode::kPercentLarge:
        para_style.SetLineHeightInPercent(1.5f);
        break;
      case Mode::kPercentSmall:
        para_style.SetLineHeightInPercent(0.5f);
        break;
    }
    para->SetParagraphStyle(&para_style);
    para->AddTextRun(&style, "ab");
    auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
    layout.Layout(para.get(), region.get(), context);
    return std::make_pair(std::move(para), std::move(region));
  };

  auto [para_default, region_default] = layout_helper(Mode::kDefault);
  ASSERT_EQ(region_default->GetLineCount(), 1u);
  auto* line_default = region_default->GetLine(0);
  ASSERT_NE(line_default, nullptr);

  float rect_default_02[4]{};
  float rect_default_11[4]{};
  line_default->GetTightBoundingRectByCharRange(rect_default_02, 0, 2);
  line_default->GetTightBoundingRectByCharRange(rect_default_11, 1, 1);

  const Mode modes[] = {
      Mode::kExactLarge,   Mode::kExactSmall,   Mode::kAtLeastLarge,
      Mode::kAtLeastSmall, Mode::kPercentLarge, Mode::kPercentSmall,
  };
  for (auto mode : modes) {
    auto [para, region] = layout_helper(mode);
    ASSERT_EQ(region->GetLineCount(), 1u);
    auto* line = region->GetLine(0);
    ASSERT_NE(line, nullptr);

    float rect_02[4]{};
    float rect_11[4]{};
    line->GetTightBoundingRectByCharRange(rect_02, 0, 2);
    line->GetTightBoundingRectByCharRange(rect_11, 1, 1);
    for (int i = 0; i < 4; ++i) {
      EXPECT_FLOAT_EQ(rect_default_02[i], rect_02[i]);
      EXPECT_FLOAT_EQ(rect_default_11[i], rect_11[i]);
    }
  }
}

TEST_F(TextLayoutTest,
       TextLine_GetTightBoundingRectByCharRange_DominantBaselineTop) {
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style style;
  style.SetTextSize(2.f);
  para_style.SetDefaultStyle(style);
  para_style.SetDominantBaseline(DominantBaseline::kTop);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&style, "ab");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);
  EXPECT_FLOAT_EQ(line->GetLineBaseLine(), line->GetLineTop());

  float rect[4]{};
  line->GetTightBoundingRectByCharRange(rect, 0, 2);
  EXPECT_FLOAT_EQ(rect[0], 0.f);
  EXPECT_FLOAT_EQ(rect[1], 0.f);
  EXPECT_FLOAT_EQ(rect[2], 4.f);
  EXPECT_FLOAT_EQ(rect[3], 2.f);
}

TEST_F(TextLayoutTest,
       TextLine_GetTightBoundingRectByCharRange_MixedFontSizes_MiddleBaseline) {
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style large;
  large.SetTextSize(10.f);
  Style small;
  small.SetTextSize(5.f);
  para_style.SetDefaultStyle(large);
  para_style.SetDominantBaseline(DominantBaseline::kMiddle);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&large, "A");
  para->AddTextRun(&small, "B");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);

  float rect_large[4]{};
  line->GetTightBoundingRectByCharRange(rect_large, 0, 1);
  EXPECT_FLOAT_EQ(rect_large[0], 0.f);
  EXPECT_FLOAT_EQ(rect_large[1], -5.f);
  EXPECT_FLOAT_EQ(rect_large[2], 10.f);
  EXPECT_FLOAT_EQ(rect_large[3], 5.f);

  float rect_small[4]{};
  line->GetTightBoundingRectByCharRange(rect_small, 1, 2);
  EXPECT_FLOAT_EQ(rect_small[0], 10.f);
  EXPECT_FLOAT_EQ(rect_small[1], -2.5f);
  EXPECT_FLOAT_EQ(rect_small[2], 15.f);
  EXPECT_FLOAT_EQ(rect_small[3], 2.5f);
}

TEST_F(TextLayoutTest,
       TextLine_GetTightBoundingRectByCharRange_MixedFontSizes_TopBaseline) {
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style large;
  large.SetTextSize(10.f);
  Style small;
  small.SetTextSize(5.f);
  para_style.SetDefaultStyle(large);
  para_style.SetDominantBaseline(DominantBaseline::kTop);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&large, "A");
  para->AddTextRun(&small, "B");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);

  float rect_large[4]{};
  line->GetTightBoundingRectByCharRange(rect_large, 0, 1);
  EXPECT_FLOAT_EQ(rect_large[0], 0.f);
  EXPECT_FLOAT_EQ(rect_large[1], 0.f);
  EXPECT_FLOAT_EQ(rect_large[2], 10.f);
  EXPECT_FLOAT_EQ(rect_large[3], 10.f);

  float rect_small[4]{};
  line->GetTightBoundingRectByCharRange(rect_small, 1, 2);
  EXPECT_FLOAT_EQ(rect_small[0], 10.f);
  EXPECT_FLOAT_EQ(rect_small[1], 0.f);
  EXPECT_FLOAT_EQ(rect_small[2], 15.f);
  EXPECT_FLOAT_EQ(rect_small[3], 5.f);
}

TEST_F(
    TextLayoutTest,
    TextLine_GetTightBoundingRectByCharRange_MixedFontSizes_AlphabeticBaseline) {
  TextLayout layout(GetFixedSizeMockShaper());
  TTTextContext context;
  auto para = std::make_unique<ParagraphImpl>();
  ParagraphStyle para_style;
  Style large;
  large.SetTextSize(10.f);
  Style small;
  small.SetTextSize(5.f);
  para_style.SetDefaultStyle(large);
  para_style.SetDominantBaseline(DominantBaseline::kAlphabetic);
  para->SetParagraphStyle(&para_style);
  para->AddTextRun(&large, "A");
  para->AddTextRun(&small, "B");
  auto region = std::make_unique<LayoutRegion>(100.f, 50.f);
  layout.Layout(para.get(), region.get(), context);

  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  ASSERT_NE(line, nullptr);

  float rect_large[4]{};
  line->GetTightBoundingRectByCharRange(rect_large, 0, 1);
  EXPECT_FLOAT_EQ(rect_large[0], 0.f);
  EXPECT_FLOAT_EQ(rect_large[1], -7.5f);
  EXPECT_FLOAT_EQ(rect_large[2], 10.f);
  EXPECT_FLOAT_EQ(rect_large[3], 2.5f);

  float rect_small[4]{};
  line->GetTightBoundingRectByCharRange(rect_small, 1, 2);
  EXPECT_FLOAT_EQ(rect_small[0], 10.f);
  EXPECT_FLOAT_EQ(rect_small[1], -3.75f);
  EXPECT_FLOAT_EQ(rect_small[2], 15.f);
  EXPECT_FLOAT_EQ(rect_small[3], 1.25f);
}

TEST_F(TextLayoutTest, InlineObjectWithBaselineOffset) {
  const float page_width = 100.0f;
  const float page_height = 100.0f;
  const float width = 20.0f;
  const float ascent = -15.0f;
  const float descent = 5.0f;
  const float baseline_offset = 10.0f;

  auto layout_helper = [this, width, ascent, descent, baseline_offset,
                        page_width, page_height]() {
    auto para = std::make_unique<ParagraphImpl>();
    Style style;
    style.SetBaselineOffset(baseline_offset);

    auto inline_obj =
        std::make_shared<MockInlineObject>(width, ascent, descent);
    para->AddShapeRun(style.GetImpl(), inline_obj, true, false, 0);

    TTTextContext context;
    TextLayout layout(GetFixedSizeMockShaper());
    auto region = std::make_unique<LayoutRegion>(page_width, page_height);
    layout.Layout(para.get(), region.get(), context);

    return std::make_pair(std::move(para), std::move(region));
  };

  auto [para, region] = layout_helper();
  ASSERT_EQ(region->GetLineCount(), 1u);
  auto* line = region->GetLine(0);
  auto baseline = line->GetLineBaseLine();

  float rect[4];
  line->GetCharBoundingRect(rect, 0);

  // rect: [left, top, width, height]
  EXPECT_FLOAT_EQ(rect[2], width);
  EXPECT_FLOAT_EQ(rect[3], descent - ascent);

  EXPECT_FLOAT_EQ(rect[1] - baseline, baseline_offset + ascent);
}

}  // namespace tttext
}  // namespace ttoffice
