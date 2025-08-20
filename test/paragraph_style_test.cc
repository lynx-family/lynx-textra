// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include "test_src.h"
#include "test_utils.h"

TEST(ParagraphStyleTest, LineGapTest) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::LineGapTest(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(), "line_gap_test_out.png");
}

TEST(ParagraphStyleTest, LineHeightTest1) {
  auto paragraph = TestUtils::GenerateDefaultParagraphTestCase();
  paragraph->GetParagraphStyle().SetLineHeightInPxAtLeast(30);
  TTTextContext context;
  auto layout_page =
      std::make_unique<LayoutRegion>(CANVAS_WIDTH, CANVAS_HEIGHT);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(paragraph.get(), layout_page.get(), context);
  TestUtils::DrawLayoutPage(layout_page.get(), "line_height_test1_out.png");
}

TEST(ParagraphStyleTest, LineHeightTest2) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::LineHeightTest2(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(), "line_height_test2_out.png");
}

TEST(ParagraphStyleTest, VerticalAlignmentTest1) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::VerticalAlignmentTest1(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "vertical_alignment_test1_out.png");
}

TEST(ParagraphStyleTest, VerticalAlignmentTest2) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::VerticalAlignmentTest2(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "vertical_alignment_test2_out.png");
}

TEST(ParagraphStyleTest, VerticalAlignmentTest3) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::VerticalAlignmentTest3(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "vertical_alignment_test3_out.png");
}

TEST(ParagraphStyleTest, VerticalAlignmentTest4) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::VerticalAlignmentTest4(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "vertical_alignment_test4_out.png");
}

TEST(ParagraphStyleTest, HorizontalAlignmentTest1) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::HorizontalAlignmentTest1(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "horizontal_alignment_test1_out.png");
}

TEST(ParagraphStyleTest, HorizontalAlignmentTest2) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::HorizontalAlignmentTest2(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "horizontal_alignment_test2_out.png");
}

TEST(ParagraphStyleTest, HorizontalAlignmentTest3) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::HorizontalAlignmentTest3(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "horizontal_alignment_test3_out.png");
}

TEST(ParagraphStyleTest, HorizontalAlignmentTest4) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::HorizontalAlignmentTest4(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "horizontal_alignment_test4_out.png");
}

TEST(ParagraphStyleTest, HorizontalAlignmentTest5) {
  std::unique_ptr<Paragraph> paragraph = TTParagraph::Create();
  auto layout_page = ParagraphStyleTestClass::HorizontalAlignmentTest5(
      *paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(),
                            "horizontal_alignment_test5_out.png");
}
