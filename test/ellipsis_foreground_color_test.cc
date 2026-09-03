// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph_style.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "mocks.h"
#include "src/textlayout/paragraph_impl.h"
#include "test_utils.h"

using namespace ttoffice::tttext;
using namespace ::testing;

namespace {

constexpr float kTextSize = 10.f;
constexpr float kLineWidth = 130.f;

std::unique_ptr<ParagraphImpl> MakeTruncatedParagraph(
    bool uses_default_foreground) {
  auto paragraph = std::make_unique<ParagraphImpl>();
  auto& paragraph_style = paragraph->GetParagraphStyle();

  Style default_style;
  default_style.SetTextSize(kTextSize);
  default_style.SetForegroundColor(TTColor::BLUE);
  paragraph_style.SetDefaultStyle(default_style);
  paragraph_style.SetMaxLines(1);
  paragraph_style.SetEllipsis("...");
  paragraph_style.SetEllipsisUsesDefaultForeground(uses_default_foreground);

  Style tail_style = default_style;
  tail_style.SetForegroundColor(TTColor::RED);

  Style leading_style = default_style;
  leading_style.SetForegroundColor(TTColor::GREEN);
  paragraph->AddTextRun(&leading_style, "AAAAAAAAAA");
  paragraph->AddTextRun(&tail_style, "BBBBBBBBBBBBBBBBBBBB");
  return paragraph;
}

struct DrawRecord {
  uint32_t glyph_count;
  TTColor fill_color;
};

std::vector<DrawRecord> Draw(LayoutRegion* region,
                             MockCanvasHelper* canvas_helper) {
  std::vector<DrawRecord> records;
  ON_CALL(*canvas_helper, CreatePainter()).WillByDefault(Invoke([]() {
    return std::make_unique<Painter>();
  }));
  EXPECT_CALL(*canvas_helper, DrawGlyphs(_, _, _, nullptr, 0, _, _, _, _, _))
      .WillRepeatedly(Invoke([&](const ITypefaceHelper*, uint32_t glyph_count,
                                 const uint16_t*, const char*, uint32_t, float,
                                 float, float*, float*, Painter* painter) {
        records.push_back(DrawRecord{glyph_count, painter->GetFillColor()});
      }));
  LayoutDrawer drawer(canvas_helper);
  drawer.DrawLayoutPage(region);
  return records;
}

}  // namespace

TEST(EllipsisForegroundColorTest, DrawsDefaultForegroundColor) {
  auto paragraph = MakeTruncatedParagraph(true);
  auto region =
      TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), kLineWidth);

  NiceMock<MockCanvasHelper> canvas_helper;
  auto records = Draw(region.get(), &canvas_helper);

  ASSERT_FALSE(records.empty());
  EXPECT_EQ(records.back().glyph_count, 3u);
  EXPECT_EQ(records.back().fill_color, TTColor::BLUE);
}

TEST(EllipsisForegroundColorTest, WithoutOverrideKeepsTailForegroundColor) {
  auto paragraph = MakeTruncatedParagraph(false);
  auto region =
      TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), kLineWidth);
  NiceMock<MockCanvasHelper> canvas_helper;
  auto records = Draw(region.get(), &canvas_helper);

  ASSERT_FALSE(records.empty());
  EXPECT_EQ(records.back().glyph_count, 3u);
  EXPECT_EQ(records.back().fill_color, TTColor::RED);
}

TEST(EllipsisForegroundColorTest, NoTruncationDoesNotDrawEllipsis) {
  auto paragraph = MakeTruncatedParagraph(true);
  auto region = TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 400.f);

  NiceMock<MockCanvasHelper> canvas_helper;
  auto records = Draw(region.get(), &canvas_helper);

  ASSERT_FALSE(records.empty());
  for (const auto& record : records) {
    EXPECT_NE(record.glyph_count, 3u);
  }
}
