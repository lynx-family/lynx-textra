// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/paragraph.h>

#include "src/textlayout/run/base_run.h"
#include "test_utils.h"

using namespace ttoffice::tttext;

TEST(ParagraphTest, CreateParagraph) {
  auto paragraph = Paragraph::Create();
  ASSERT_NE(paragraph, nullptr);
  EXPECT_EQ(paragraph->GetCharCount(), 0u);
  EXPECT_EQ(paragraph->GetRunCount(), 0u);
}

TEST(ParagraphTest, AddTextRun_ValidInput) {
  auto paragraph = Paragraph::Create();
  // Add a string literal
  paragraph->AddTextRun(nullptr, "Hello");
  EXPECT_EQ(paragraph->GetCharCount(), 5u);
  EXPECT_EQ(paragraph->GetRunCount(), 1u);
  EXPECT_EQ(paragraph->GetContentString(0, 5), std::string("Hello"));
  // Add a std::string
  std::string text = "World";
  paragraph->AddTextRun(nullptr, text.c_str());
  EXPECT_EQ(paragraph->GetCharCount(), 10u);
  EXPECT_EQ(paragraph->GetRunCount(), 2u);
  EXPECT_EQ(paragraph->GetContentString(5, 5u), std::string("World"));
  // Add a string literal with style
  Style style;
  style.SetTextSize(24);
  paragraph->AddTextRun(&style, "Test");
  EXPECT_EQ(paragraph->GetCharCount(), 14u);
  EXPECT_EQ(paragraph->GetRunCount(), 3u);
  EXPECT_EQ(paragraph->GetContentString(10, 4u), std::string("Test"));
  // Add a substring of a string literal
  const char* long_text = "This is a long string but only adding first 4 chars";
  paragraph->AddTextRun(nullptr, long_text, 4);
  EXPECT_EQ(paragraph->GetCharCount(), 18u);
  EXPECT_EQ(paragraph->GetRunCount(), 4u);
  EXPECT_EQ(paragraph->GetContentString(14, 4), "This");
}

TEST(ParagraphTest, SetParagraphStyle) {
  auto paragraph = Paragraph::Create();
  EXPECT_EQ(paragraph->GetParagraphStyle().GetWriteDirection(),
            WriteDirection::kAuto);
  ParagraphStyle new_style;
  new_style.SetWriteDirection(WriteDirection::kRTL);
  paragraph->SetParagraphStyle(&new_style);
  EXPECT_EQ(paragraph->GetParagraphStyle().GetWriteDirection(),
            WriteDirection::kRTL);
}

TEST(ParagraphTest, GetContentString_English) {
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "Hello World");
  // Test normal range
  EXPECT_EQ(paragraph->GetContentString(0, 11), std::string("Hello World"));
  EXPECT_EQ(paragraph->GetContentString(0, 5), std::string("Hello"));
  EXPECT_EQ(paragraph->GetContentString(6, 5), std::string("World"));
  // Test count beyond content length
  EXPECT_EQ(paragraph->GetContentString(0, 100), std::string("Hello World"));
  // Test empty range
  EXPECT_EQ(paragraph->GetContentString(0, 0), std::string(""));
  EXPECT_EQ(paragraph->GetContentString(1, 0), std::string(""));
  // Test out-of-boundary range
  EXPECT_EQ(paragraph->GetContentString(11, 1), std::string(""));
  EXPECT_EQ(paragraph->GetContentString(12, 5), std::string(""));
  EXPECT_EQ(paragraph->GetContentString(-5, 5), std::string(""));
}

TEST(ParagraphTest, GetContentString_SpecialCharacters) {
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "English\nwords");
  EXPECT_EQ(paragraph->GetContentString(0, 13), std::string("English\nwords"));
  paragraph->AddTextRun(nullptr, "中文");
  EXPECT_EQ(paragraph->GetContentString(13, 2), std::string("中文"));
  paragraph->AddTextRun(nullptr, "😊");
  EXPECT_EQ(paragraph->GetContentString(15, 1), std::string("😊"));
}

TEST(ParagraphTest, AddShapeRun_NotFloat) {
  auto paragraph = Paragraph::Create();
  auto shape = std::make_shared<TestShape>();
  paragraph->AddShapeRun(nullptr, shape, false);
  EXPECT_EQ(paragraph->GetCharCount(), 1u);
  EXPECT_EQ(paragraph->GetRunCount(), 1u);
  EXPECT_EQ(paragraph->GetContentString(0, 1),
            BaseRun::ObjectReplacementCharacter());
}

TEST(ParagraphTest, AddShapeRun_Float) {
  auto paragraph = Paragraph::Create();
  auto shape = std::make_shared<TestShape>();
  paragraph->AddShapeRun(nullptr, shape, true);
  EXPECT_EQ(paragraph->GetCharCount(), 1u);
  EXPECT_EQ(paragraph->GetRunCount(), 1u);
  EXPECT_EQ(paragraph->GetContentString(0, 1),
            BaseRun::ObjectReplacementCharacter());
}

TEST(ParagraphTest, AddGhostShapeRun) {
  auto paragraph = Paragraph::Create();
  auto shape = std::make_shared<TestShape>();
  paragraph->AddGhostShapeRun(nullptr, shape);
  EXPECT_EQ(paragraph->GetCharCount(), 0u);
  EXPECT_EQ(paragraph->GetRunCount(), 1u);
}
