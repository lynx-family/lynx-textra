// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/layout_drawer.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>

#include "run/base_run.h"
#include "src/textlayout/paragraph_impl.h"
#include "test_src.h"
#include "test_utils.h"

TEST(TextTest, WordBreakTypeTest1) {
  ParagraphImpl word_break_paragraph1;
  auto layout_page = TextTestClass::WordBreakTypeTest1(
      word_break_paragraph1, CANVAS_WIDTH, CANVAS_HEIGHT);
  TestUtils::DrawLayoutPage(layout_page.get(), "word_break_test1_out.png");
}

TEST(TextTest, WordBreakTypeTest2) {
  ParagraphImpl word_break_paragraph2;
  Style word_break_word_rpr;
  word_break_word_rpr.SetTextSize(24);
  word_break_word_rpr.SetWordBreak(ttoffice::tttext::WordBreakType::kNormal);
  word_break_paragraph2.AddTextRun(
      &word_break_word_rpr,
      "This is a long and "
      " Honorificabilitudinitatibus califragilisticexpialidocious "
      "Taumatawhakatangihangakoauauot amateaturipukakapikimaun "
      "gahoronukupokaiwhe "
      "nuakitanatahu"
      "\n");

  TextLayout layout(TestUtils::getRealShaper());
  auto layout_page =
      std::make_unique<LayoutRegion>(CANVAS_WIDTH, CANVAS_HEIGHT);
  TTTextContext context;
  layout.Layout(&word_break_paragraph2, layout_page.get(), context);
  TestUtils::DrawLayoutPage(layout_page.get(), "word_break_test2_out.png");
}

TEST(TextTest, SubSupTest) {
  ParagraphImpl paragraph;
  auto layout_page =
      TextTestClass::SubSupTest(paragraph, CANVAS_WIDTH, CANVAS_HEIGHT);
  if (layout_page) {
    TestUtils::DrawLayoutPage(layout_page.get(),
                              "sup_script_sub_script_test_out.png");
  }
}
TEST(InternalExternalTest, ParaTest1) {
  std::vector<std::string> text_list = {
      "abc", "这是一个字符串，英语代表需要屏蔽的字符：", "abc",
      "这是一个字符串", "abc"};
  std::vector<bool> ignored_list = {true, false, true, false, true};
  auto para = std::make_unique<ParagraphImpl>();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style r_pr;
  r_pr.SetTextSize(24);
  for (size_t i = 0; i < text_list.size(); i++) {
    if (ignored_list[i]) {
      para->AddGhostShapeRun(&r_pr, std::make_shared<TestShape>());
    } else {
      auto text = text_list[i];
      para->AddTextRun(r_pr, text.c_str(), text.length());
    }
  }
  auto total_content = para->GetContent();
  EXPECT_EQ(total_content.ToString(),
            "这是一个字符串，英语代表需要屏蔽的字符：这是一个字符串");
  EXPECT_EQ(total_content.ToUTF32(),
            U"这是一个字符串，英语代表需要屏蔽的字符：这是一个字符串");
  EXPECT_EQ(total_content.GetCharCount(), 27u);

  auto partial_content1 = para->GetContent(0, 19u);
  //  auto str1 = partial_content1.ToString();
  //  auto in_str1 = partial_content1.ToString();
  EXPECT_EQ(partial_content1.ToString(),
            "这是一个字符串，英语代表需要屏蔽的字符");
  EXPECT_EQ(partial_content1.GetCharCount(), 19u);
  //  EXPECT_EQ(partial_content1.ToString(),
  //            "这是一个字符串，英语代表需要屏蔽的字符");
  //  EXPECT_EQ(partial_content1.GetCharCount(), 19u);

  auto partial_content2 = para->GetContent(18);
  //  auto str2 = partial_content2.ToString();
  //  auto in_str2 = partial_content2.ToString();
  EXPECT_EQ(partial_content2.ToString(), "符：这是一个字符串");
  EXPECT_EQ(partial_content2.GetCharCount(), 9u);
  //  EXPECT_EQ(partial_content2.ToString(), "符：abc这是一个字符串abc");
  //  EXPECT_EQ(partial_content2.GetCharCount(), 15u);
}

TEST(InternalExternalTest, ParaTest2) {
  std::vector<std::string> text_list = {"abcabcabcabcabcabcbacbabcbacbacbacb"};
  std::vector<bool> ignored_list = {true};
  auto para = std::make_unique<ParagraphImpl>();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style r_pr;
  r_pr.SetTextSize(24);
  for (size_t i = 0; i < text_list.size(); i++) {
    if (ignored_list[i]) {
      para->AddGhostShapeRun(&r_pr, std::make_shared<TestShape>());
    } else {
      auto text = text_list[i];
      para->AddTextRun(r_pr, text.c_str(), text.length());
    }
  }
  auto total_content = para->GetContent();
  EXPECT_EQ(total_content.ToString(), "");
  EXPECT_EQ(total_content.GetCharCount(), 0u);
  //  EXPECT_EQ(total_content.ToString(), "");
  //  EXPECT_EQ(total_content.GetCharCount(), 0u);

  auto partial_content1 = para->GetContent(0, 10);
  auto str1 = partial_content1.ToString();
  EXPECT_EQ(partial_content1.ToString(), "");
  EXPECT_EQ(partial_content1.GetCharCount(), 0u);
  //  EXPECT_EQ(partial_content1.ToString(), "");
  //  EXPECT_EQ(partial_content1.GetCharCount(), 0u);

  auto partial_content2 = para->GetContent(1);
  auto str2 = partial_content2.ToString();
  EXPECT_EQ(partial_content2.ToString(), "");
  EXPECT_EQ(partial_content2.GetCharCount(), 0u);
  //  EXPECT_EQ(partial_content2.ToString(), "");
  //  EXPECT_EQ(partial_content2.GetCharCount(), 0u);
}

TEST(InternalExternalTest, ParaTest3) {
  std::vector<std::string> text_list = {
      "这是一个字符串，英语代表需要屏蔽的字符。"};
  std::vector<bool> ignored_list = {false};
  auto para = std::make_unique<ParagraphImpl>();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style r_pr;
  r_pr.SetTextSize(24);
  for (size_t i = 0; i < text_list.size(); i++) {
    if (ignored_list[i]) {
      para->AddGhostShapeRun(&r_pr, std::make_shared<TestShape>());
    } else {
      auto text = text_list[i];
      para->AddTextRun(r_pr, text.c_str(), text.length());
    }
  }
  auto total_content = para->GetContent();
  EXPECT_EQ(total_content.ToString(),
            "这是一个字符串，英语代表需要屏蔽的字符。");
  EXPECT_EQ(total_content.GetCharCount(), 20u);
  //  EXPECT_EQ(total_content.ToString(),
  //            "这是一个字符串，英语代表需要屏蔽的字符。");
  //  EXPECT_EQ(total_content.GetCharCount(), 20u);

  auto partial_content1 = para->GetContent(0, 19);
  auto str1 = partial_content1.ToString();
  //  auto in_str1 = partial_content1.ToString();
  EXPECT_EQ(partial_content1.ToString(),
            "这是一个字符串，英语代表需要屏蔽的字符");
  EXPECT_EQ(partial_content1.GetCharCount(), 19u);
  //  EXPECT_EQ(partial_content1.ToString(),
  //            "这是一个字符串，英语代表需要屏蔽的字符");
  //  EXPECT_EQ(partial_content1.GetCharCount(), 19u);

  auto partial_content2 = para->GetContent(18);
  //  auto str2 = partial_content2.ToString();
  //  auto in_str2 = partial_content2.ToString();
  EXPECT_EQ(partial_content2.ToString(), "符。");
  EXPECT_EQ(partial_content2.GetCharCount(), 2u);
  //  EXPECT_EQ(partial_content2.ToString(), "符。");
  //  EXPECT_EQ(partial_content2.GetCharCount(), 2u);
}

TEST(InternalExternalTest, ParaTest4) {
  std::vector<std::string> text_list = {""};
  std::vector<bool> ignored_list = {false};
  auto para = std::make_unique<ParagraphImpl>();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style r_pr;
  r_pr.SetTextSize(24);
  for (size_t i = 0; i < text_list.size(); i++) {
    if (ignored_list[i]) {
      para->AddGhostShapeRun(&r_pr, std::make_shared<TestShape>());
    } else {
      auto text = text_list[i];
      para->AddTextRun(r_pr, text.c_str(), text.length());
    }
  }
  auto total_content = para->GetContent();
  EXPECT_EQ(total_content.ToString(), "");
  EXPECT_EQ(total_content.GetCharCount(), 0u);
  auto ghost_content = para->GetContent();
  EXPECT_EQ(ghost_content.ToUTF32(), U"");
  EXPECT_EQ(ghost_content.Length(), 0u);

  auto partial_content1 = para->GetContent(0, 10);
  EXPECT_EQ(partial_content1.ToString(), "");
  EXPECT_EQ(partial_content1.GetCharCount(), 0u);
  //  EXPECT_EQ(partial_content1.ToString(), "");
  //  EXPECT_EQ(partial_content1.GetCharCount(), 0u);

  auto partial_content2 = para->GetContent(1);
  auto str2 = partial_content2.ToString();
  EXPECT_EQ(partial_content2.ToString(), "");
  EXPECT_EQ(partial_content2.GetCharCount(), 0u);
  EXPECT_EQ(partial_content2.ToString(), "");
  EXPECT_EQ(partial_content2.GetCharCount(), 0u);
}

// TODO: Fix ParagraphImpl::AddTextRun() assert failure
// TEST(BreakTest, BreakAfterBr) {
//   ParagraphImpl paragraph;
//   const Style& style = paragraph.GetDefaultStyle();
//   paragraph.AddTextRun(style, "文本", strlen("文本"));
//   paragraph.AddTextRun(style, "\u2028", strlen("\u2028"), true);
//   paragraph.AddTextRun(style, "\n", strlen("\n"));
//   TextLayoutConfig config;
//   paragraph.FormatRunList();
//   // TODO need to implement
//   // ASSERT_EQ(paragraph.GetRun(3)->GetBreakType(),
//   LineBreakType::kBreakLine);
// }

TEST(TextTest, LayoutWidthTest) {
  Style default_style;
  default_style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(default_style);
  para->GetParagraphStyle().SetHangingIndentInPx(48);
  para->AddTextRun(&default_style, "第一行\n第二行");
  float width = 400;
  auto region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  EXPECT_EQ(region->GetLayoutedWidth(), 120);

  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kCenter);
  region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  EXPECT_EQ(region->GetLayoutedWidth(), 120);
}

TEST(TextTest, LineSpacingTest) {
  Style default_style;
  default_style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(default_style);
  para->GetParagraphStyle().SetLineSpaceAfterPx(10);
  para->GetParagraphStyle().SetLineSpaceBeforePx(15);
  para->AddTextRun(&default_style, "第一行\n第二行");
  float width = 400;
  auto region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  auto line1 = region->GetLine(0);
  auto line2 = region->GetLine(1);
  EXPECT_EQ(line1->GetLineTop(), 15);
  EXPECT_EQ(line2->GetLineTop() - line1->GetLineBottom(), 25);
}

TEST(TextTest, JustifyAlignment) {
  Style default_style;
  default_style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kJustify);
  para->AddTextRun(&default_style, "第一行\n第二行");
  constexpr float width = 240;
  auto region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width,
                                                        LayoutMode::kAtMost);
  EXPECT_NE(region->GetLayoutedWidth(), 240);
  para = Paragraph::Create();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kJustify);
  para->AddTextRun(&default_style,
                   "下面是一段比较长的中文文本，并且会在段中间换行，用来看多行"
                   "文本的对齐效果。");
  region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width,
                                                   LayoutMode::kAtMost);
  EXPECT_EQ(region->GetLayoutedWidth(), 240);
}

TEST(TextTest, LineBreakStrategyTest) {
  Style default_style;
  default_style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->AddTextRun(&default_style, "一段文本(一段文本)一段文本");
  constexpr float width = 121;
  auto region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  EXPECT_EQ(region->GetLineCount(), 4u);
  EXPECT_EQ(region->GetLine(0)->GetCharCount(), 4u);
  EXPECT_EQ(region->GetLine(1)->GetCharCount(), 4u);
  EXPECT_EQ(region->GetLine(2)->GetCharCount(), 5u);
  EXPECT_EQ(region->GetLine(3)->GetCharCount(), 1u);

  para = Paragraph::Create();
  para->AddTextRun(&default_style, "一段文本(一段文本)一段文本");
  para->GetParagraphStyle().AllowBreakAroundPunctuation(true);
  region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  EXPECT_EQ(region->GetLineCount(), 3u);
  EXPECT_EQ(region->GetLine(0)->GetCharCount(), 5u);
  EXPECT_EQ(region->GetLine(1)->GetCharCount(), 5u);
  EXPECT_EQ(region->GetLine(2)->GetCharCount(), 4u);
}

TEST(TextTest, EllipsisWithIndentTest) {
  Style default_style;
  default_style.SetTextSize(10);
  auto para = Paragraph::Create();
  para->AddTextRun(&default_style,
                   "一段文本一段文本一段文本一段文本一段文本一段文本");
  para->GetParagraphStyle().SetHangingIndentInPx(20);
  para->GetParagraphStyle().SetEllipsis("...");
  para->GetParagraphStyle().SetMaxLines(2);
  constexpr float width = 101;
  auto region = TestUtils::SimpleLayoutParagraphByWidth(para.get(), width);
  EXPECT_EQ(region->GetLine(0)->GetCharCount(), 10u);
  EXPECT_EQ(region->GetLine(1)->GetCharCount(), 8u);
  auto* line = region->GetLine(1);
  float rect[4];
  line->GetBoundingRectByCharRange(rect, line->GetStartCharPos(),
                                   line->GetEndCharPos());
  EXPECT_EQ(rect[0], 20.f);
  EXPECT_EQ(rect[2], 80.f);
}
