// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gtest/gtest.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/style/style_manager.h"
#include "test_utils.h"

using namespace ttoffice::tttext;

namespace {
constexpr float kIntrinsicTestGlyphWidth = 10.f;

std::unique_ptr<Paragraph> CreateFormattedParagraph(const char* text) {
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(kIntrinsicTestGlyphWidth);
  paragraph->AddTextRun(&style, text);
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);
  return paragraph;
}

class InspectableParagraphImpl : public ParagraphImpl {
 public:
  RunType GetRunType(uint32_t index) const {
    return run_lst_[index]->GetType();
  }
};

// Checks that the grapheme clusters of `paragraph` tile [0, GetCharCount())
// exactly, and that every offset inside a cluster reports the same range.
void ExpectGraphemeClustersTile(const Paragraph& paragraph) {
  const uint32_t char_count = paragraph.GetCharCount();
  uint32_t start = 0;
  while (start < char_count) {
    const auto range = paragraph.GetGraphemeBoundary(start);
    EXPECT_EQ(range.first, start);
    ASSERT_GT(range.second, start);
    ASSERT_LE(range.second, char_count);
    for (uint32_t offset = start; offset < range.second; ++offset) {
      EXPECT_EQ(paragraph.GetGraphemeBoundary(offset), range) << offset;
    }
    start = range.second;
  }
  EXPECT_EQ(start, char_count);
  EXPECT_EQ(paragraph.GetGraphemeBoundary(char_count), std::make_pair(0u, 0u));
}
}  // namespace

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

TEST(ParagraphTest, PunctuationCompressStyleAndRunSplit) {
  ParagraphStyle style;
  style.SetPunctuationCompressOptions(PunctuationCompressOption::kAll |
                                      PunctuationCompressOption::kLineEdge);
  style.UpdatePunctuationCompressConfig({U'（', PunctuationType::kNone});
  style.UpdatePunctuationCompressConfig(
      {U'（', PunctuationType::kOpen, 0.5f, 0.25f, 0.4f});
  style.UpdatePunctuationCompressConfig(
      {U'）', PunctuationType::kClose, 0.5f, 0.25f, 0.4f});

  InspectableParagraphImpl paragraph;
  paragraph.SetParagraphStyle(&style);
  paragraph.AddTextRun(nullptr, u8"a（b）c");
  auto page = TestUtils::SimpleLayoutParagraphByWidth(&paragraph, 100.f);

  EXPECT_EQ(paragraph.GetRunCount(), 5u);
  EXPECT_EQ(paragraph.GetRunType(0), RunType::kTextRun);
  EXPECT_EQ(paragraph.GetRunType(1), RunType::kPunctuationRun);
  EXPECT_EQ(paragraph.GetRunType(2), RunType::kTextRun);
  EXPECT_EQ(paragraph.GetRunType(3), RunType::kPunctuationRun);
  EXPECT_EQ(paragraph.GetRunType(4), RunType::kTextRun);
  EXPECT_EQ(
      paragraph.GetParagraphStyle().GetPunctuationCompressOptions(),
      PunctuationCompressOption::kAll | PunctuationCompressOption::kLineEdge);
}

TEST(ParagraphTest, PunctuationConfigWithoutRuleDoesNotSplitRun) {
  ParagraphStyle style;
  style.UpdatePunctuationCompressConfig(
      {U'（', PunctuationType::kOpen, 0.5f, 0.25f, 0.4f});

  InspectableParagraphImpl paragraph;
  paragraph.SetParagraphStyle(&style);
  paragraph.AddTextRun(nullptr, u8"a（b");
  auto page = TestUtils::SimpleLayoutParagraphByWidth(&paragraph, 100.f);

  EXPECT_EQ(paragraph.GetRunCount(), 1u);
  EXPECT_EQ(paragraph.GetRunType(0), RunType::kTextRun);
}

TEST(ParagraphTest, SaveRestoreStyle) {
  class TestParagraphImpl : public ParagraphImpl {
   public:
    StyleManager* GetStyleManager() { return style_manager_.get(); }
  };

  auto paragraph_impl = std::make_unique<TestParagraphImpl>();
  auto* style_manager = paragraph_impl->GetStyleManager();
  std::unique_ptr<Paragraph> paragraph = std::move(paragraph_impl);
  paragraph->AddTextRun(nullptr, "abc");

  Style red_style;
  red_style.SetForegroundColor(TTColor::RED);
  Style green_style;
  green_style.SetForegroundColor(TTColor::GREEN);

  paragraph->ApplyStyleInRange(red_style, 0, 3);
  paragraph->SaveStyle();
  paragraph->ApplyStyleInRange(green_style, 1, 1);
  EXPECT_EQ(style_manager->GetForegroundColor(1), TTColor(TTColor::GREEN));

  paragraph->RestoreStyle();
  EXPECT_EQ(style_manager->GetForegroundColor(1), TTColor(TTColor::RED));
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

TEST(ParagraphTest, GetGraphemeBoundary_Ascii) {
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "hello");
  ASSERT_EQ(paragraph->GetCharCount(), 5u);
  for (uint32_t i = 0; i < paragraph->GetCharCount(); ++i) {
    EXPECT_EQ(paragraph->GetGraphemeBoundary(i), std::make_pair(i, i + 1));
  }
  ExpectGraphemeClustersTile(*paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_EmojiAndKeycap) {
  // No layout is performed: the boundary is resolved from the content only.
  auto paragraph = Paragraph::Create();
  // Two flags, each made of a pair of regional indicators.
  paragraph->AddTextRun(nullptr, "\U0001F1E8\U0001F1F3\U0001F1E8\U0001F1F3");
  // Emoji base followed by an emoji modifier.
  paragraph->AddTextRun(nullptr, "\U0001F44D\U0001F3FB");
  // Emoji base followed by a variation selector, a zero width joiner and
  // another emoji base.
  paragraph->AddTextRun(nullptr, "\U0001F3F3\uFE0F\u200D\U0001F308");
  // Keycap sequence: keycap base + U+FE0F + U+20E3.
  paragraph->AddTextRun(nullptr, "1\uFE0F\u20E3");
  ASSERT_EQ(paragraph->GetCharCount(), 13u);
  EXPECT_EQ(paragraph->GetGraphemeBoundary(0), std::make_pair(0u, 2u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(1), std::make_pair(0u, 2u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(2), std::make_pair(2u, 4u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(3), std::make_pair(2u, 4u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(4), std::make_pair(4u, 6u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(5), std::make_pair(4u, 6u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(6), std::make_pair(6u, 10u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(9), std::make_pair(6u, 10u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(10), std::make_pair(10u, 13u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(12), std::make_pair(10u, 13u));
  ExpectGraphemeClustersTile(*paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_MixedContent) {
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "a\U0001F44D\U0001F3FBb");
  // Devanagari: consonant + virama + consonant + dependent vowel sign.
  paragraph->AddTextRun(nullptr, "क्षि");
  paragraph->AddTextRun(nullptr, "e\u0301");
  ASSERT_EQ(paragraph->GetCharCount(), 10u);
  EXPECT_EQ(paragraph->GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(1), std::make_pair(1u, 3u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(2), std::make_pair(1u, 3u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(3), std::make_pair(3u, 4u));
  // The whole Devanagari sequence is a single cluster.
  EXPECT_EQ(paragraph->GetGraphemeBoundary(4), std::make_pair(4u, 8u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(7), std::make_pair(4u, 8u));
  // A combining mark stays with the character it is attached to.
  EXPECT_EQ(paragraph->GetGraphemeBoundary(8), std::make_pair(8u, 10u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(9), std::make_pair(8u, 10u));
  ExpectGraphemeClustersTile(*paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_CombiningMarks) {
  // A mark used to be split from its base, which let the shaper hand it to the
  // fallback font on its own.
  auto paragraph = Paragraph::Create();
  // Latin base with two stacked marks.
  paragraph->AddTextRun(nullptr, "e\u0323\u0301");
  // Thai consonant with a tone mark.
  paragraph->AddTextRun(nullptr, "\u0E01\u0E48");
  // Hebrew letter with a point.
  paragraph->AddTextRun(nullptr, "\u05D0\u05B7");
  ASSERT_EQ(paragraph->GetCharCount(), 7u);
  EXPECT_EQ(paragraph->GetGraphemeBoundary(0), std::make_pair(0u, 3u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(2), std::make_pair(0u, 3u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(3), std::make_pair(3u, 5u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(5), std::make_pair(5u, 7u));
  // A mark with nothing in front of it is a cluster of its own.
  auto standalone = Paragraph::Create();
  standalone->AddTextRun(nullptr, "\u0301ab");
  EXPECT_EQ(standalone->GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(standalone->GetGraphemeBoundary(1), std::make_pair(1u, 2u));
  ExpectGraphemeClustersTile(*standalone);
}

TEST(ParagraphTest, GetGraphemeBoundary_AfterLayoutUsesRenderedPieces) {
  // Laid out with a real font: the range describes what the shaper produced.
  // ASCII stays one cluster per character, and both the combining mark and the
  // flag stay with what precedes them, because the font rendered them as part
  // of the same piece.
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "abc");
  paragraph->AddTextRun(nullptr, "e\u0301");
  paragraph->AddTextRun(nullptr, "\U0001F1E8\U0001F1F3");
  ASSERT_EQ(paragraph->GetCharCount(), 7u);
  TTTextContext context;
  TextLayout layout(TestUtils::getRealShaper());
  auto region = std::make_unique<LayoutRegion>(400.f, 400.f);
  layout.Layout(paragraph.get(), region.get(), context);
  EXPECT_EQ(paragraph->GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(2), std::make_pair(2u, 3u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(3), std::make_pair(3u, 5u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(4), std::make_pair(3u, 5u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(5), std::make_pair(5u, 7u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(6), std::make_pair(5u, 7u));
  ExpectGraphemeClustersTile(*paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_SplitsWhereFontHasNoGlyph) {
  // None of the fonts available here covers Devanagari, so the conjunct is
  // rendered as one placeholder glyph per character. It is reported the way it
  // is drawn: as four clusters, not one.
  auto paragraph = Paragraph::Create();
  paragraph->AddTextRun(nullptr, "\u0915\u094D\u0937\u093F");
  ASSERT_EQ(paragraph->GetCharCount(), 4u);
  TTTextContext context;
  TextLayout layout(TestUtils::getRealShaper());
  auto region = std::make_unique<LayoutRegion>(400.f, 400.f);
  layout.Layout(paragraph.get(), region.get(), context);
  for (uint32_t offset = 0; offset < 4; ++offset) {
    EXPECT_EQ(paragraph->GetGraphemeBoundary(offset),
              std::make_pair(offset, offset + 1))
        << offset;
  }
  ExpectGraphemeClustersTile(*paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_SplitsWhatShapingSeparated) {
  // The test shaper gives every character its own glyph with an advance, which
  // is what a font with no glyph for the sequence produces: the flag and the
  // mark then fall apart into their characters, and so do the clusters.
  InspectableParagraphImpl paragraph;
  paragraph.AddTextRun(nullptr, "\U0001F1E8\U0001F1F3");
  paragraph.AddTextRun(nullptr, "e\u0301");
  ASSERT_EQ(paragraph.GetCharCount(), 4u);
  TestUtils::SimpleLayoutParagraphByWidth(&paragraph, 300.f);
  EXPECT_EQ(paragraph.GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(1), std::make_pair(1u, 2u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(2), std::make_pair(2u, 3u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(3), std::make_pair(3u, 4u));
  ExpectGraphemeClustersTile(paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_SplitsAtRunEdge) {
  // Two runs are shaped and drawn on their own, so a cluster reaching across
  // the edge is cut there even though the codepoint rules keep it whole.
  InspectableParagraphImpl paragraph;
  Style red;
  red.SetForegroundColor(TTColor::RED);
  paragraph.AddTextRun(&red, "e");
  paragraph.AddTextRun(nullptr, "\u0301");
  ASSERT_EQ(paragraph.GetCharCount(), 2u);
  TestUtils::SimpleLayoutParagraphByWidth(&paragraph, 300.f);
  EXPECT_EQ(paragraph.GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(1), std::make_pair(1u, 2u));
  ExpectGraphemeClustersTile(paragraph);
}

TEST(ParagraphTest, GetGraphemeBoundary_RightToLeft) {
  // A right to left run stores its glyphs in visual order, so the glyph range
  // of a cluster has to be resolved the other way round.
  InspectableParagraphImpl paragraph;
  ParagraphStyle style;
  style.SetWriteDirection(WriteDirection::kRTL);
  paragraph.SetParagraphStyle(&style);
  paragraph.AddTextRun(nullptr, "\u05D0\u05B7\u05D1");
  ASSERT_EQ(paragraph.GetCharCount(), 3u);

  // The test shaper draws every character on its own, so the point splits off.
  TestUtils::SimpleLayoutParagraphByWidth(&paragraph, 300.f);
  EXPECT_EQ(paragraph.GetGraphemeBoundary(0), std::make_pair(0u, 1u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(1), std::make_pair(1u, 2u));
  EXPECT_EQ(paragraph.GetGraphemeBoundary(2), std::make_pair(2u, 3u));
  ExpectGraphemeClustersTile(paragraph);

  // With a real font the point is drawn as part of the letter, so it stays.
  auto shaped = Paragraph::Create();
  shaped->SetParagraphStyle(&style);
  shaped->AddTextRun(nullptr, "\u05D0\u05B7\u05D1");
  TTTextContext context;
  TextLayout layout(TestUtils::getRealShaper());
  auto region = std::make_unique<LayoutRegion>(400.f, 400.f);
  layout.Layout(shaped.get(), region.get(), context);
  EXPECT_EQ(shaped->GetGraphemeBoundary(0), std::make_pair(0u, 2u));
  EXPECT_EQ(shaped->GetGraphemeBoundary(1), std::make_pair(0u, 2u));
  EXPECT_EQ(shaped->GetGraphemeBoundary(2), std::make_pair(2u, 3u));
  ExpectGraphemeClustersTile(*shaped);
}

TEST(ParagraphTest, GetGraphemeBoundary_OutOfRange) {
  auto paragraph = Paragraph::Create();
  EXPECT_EQ(paragraph->GetGraphemeBoundary(0), std::make_pair(0u, 0u));
  paragraph->AddTextRun(nullptr, "ab");
  ASSERT_EQ(paragraph->GetCharCount(), 2u);
  EXPECT_EQ(paragraph->GetGraphemeBoundary(1), std::make_pair(1u, 2u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(2), std::make_pair(0u, 0u));
  EXPECT_EQ(paragraph->GetGraphemeBoundary(100), std::make_pair(0u, 0u));
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

TEST(ParagraphTest, MinIntrinsicWidthUsesHardBreaks) {
  auto paragraph = CreateFormattedParagraph("ab\ncdef");

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 40.f);
}

TEST(ParagraphTest, MinIntrinsicWidthBreaksBetweenCJKCharacters) {
  auto paragraph = CreateFormattedParagraph(u8"中文文本");

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 10.f);
}

TEST(ParagraphTest, MinIntrinsicWidthKeepsLongEnglishWord) {
  auto paragraph = CreateFormattedParagraph("short elephant");

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 80.f);
}

TEST(ParagraphTest, MinIntrinsicWidthTrimsBreakableSpaces) {
  auto paragraph = CreateFormattedParagraph("aa    bbb");

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 30.f);
}

TEST(ParagraphTest, MinIntrinsicWidthTrimsSpacesAfterMultibyteCharacters) {
  auto paragraph = CreateFormattedParagraph(u8"中 ");

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 10.f);
}

TEST(ParagraphTest, MinIntrinsicWidthTrimsUnicodeSpaces) {
  for (const auto* text : {u8"a\u2003", u8"a\u3000"}) {
    auto paragraph = CreateFormattedParagraph(text);

    EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 10.f);
  }
}

TEST(ParagraphTest, MinIntrinsicWidthTrimsSpacesAcrossRuns) {
  auto paragraph = Paragraph::Create();
  Style first_style;
  first_style.SetTextSize(kIntrinsicTestGlyphWidth);
  Style second_style;
  second_style.SetTextSize(kIntrinsicTestGlyphWidth + 1.f);
  paragraph->AddTextRun(&first_style, "hello ");
  paragraph->AddTextRun(&second_style, " ");
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 50.f);
}

TEST(ParagraphTest, MinIntrinsicWidthRespectsBreakAll) {
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(kIntrinsicTestGlyphWidth);
  style.SetWordBreak(WordBreakType::kBreakAll);
  paragraph->AddTextRun(&style, "elephant");
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 10.f);
}

TEST(ParagraphTest, MinIntrinsicWidthRespectsWordBreakStyleRanges) {
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(kIntrinsicTestGlyphWidth);
  paragraph->AddTextRun(&style, "elephant");
  Style break_all;
  break_all.SetWordBreak(WordBreakType::kBreakAll);
  paragraph->ApplyStyleInRange(break_all, 0, 3);
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);

  // The first three characters can break individually; "phant" stays whole.
  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 50.f);
}

TEST(ParagraphTest, MinIntrinsicWidthFindsBreakAllAfterNormalText) {
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(kIntrinsicTestGlyphWidth);
  paragraph->AddTextRun(&style, "elephant");
  Style break_all;
  break_all.SetWordBreak(WordBreakType::kBreakAll);
  paragraph->ApplyStyleInRange(break_all, 3, 3);
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);

  // The first break-all opportunity follows "elep".
  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 40.f);
}

TEST(ParagraphTest, MinIntrinsicWidthIncludesPlaceholder) {
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(kIntrinsicTestGlyphWidth);
  paragraph->AddTextRun(&style, "aa");
  paragraph->AddShapeRun(&style, std::make_shared<TestShape>(45.f, 10.f),
                         false);
  paragraph->AddTextRun(&style, "bbb");
  TestUtils::SimpleLayoutParagraphByWidth(paragraph.get(), 1000.f);

  EXPECT_FLOAT_EQ(paragraph->GetMinIntrinsicWidth(), 45.f);
}
