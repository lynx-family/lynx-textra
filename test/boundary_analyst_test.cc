// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/boundary_analyst.h"

#include "gtest/gtest.h"
#include "src/textlayout/style/style_manager.h"
#include "test_src.h"
using namespace tttext;
TEST(BoundaryAnalystTest, DefaultBreak) {
  constexpr std::u32string_view text = U"一段文本(一段文本)一段文本";
  const BoundaryAnalyst boundary_analyst(
      text.data(), text.size(), kLineBreakStrategyDefault, StyleManager{});
  const std::vector results{
      BoundaryType::kLineBreakable, BoundaryType::kLineBreakable,
      BoundaryType::kLineBreakable, BoundaryType::kLineBreakable,
      BoundaryType::kWord,          BoundaryType::kLineBreakable,
      BoundaryType::kLineBreakable, BoundaryType::kLineBreakable,
      BoundaryType::kWord,          BoundaryType::kLineBreakable,
      BoundaryType::kLineBreakable, BoundaryType::kLineBreakable,
      BoundaryType::kLineBreakable, BoundaryType::kLineBreakable,
  };
  for (auto i = 1ull; i < text.size(); ++i) {
    EXPECT_EQ(results[i], boundary_analyst.GetBoundaryType(i));
  }

  constexpr std::u32string_view english_sentence = U"an english sentence";
  const BoundaryAnalyst english_analyst(
      english_sentence.data(), english_sentence.size(),
      kLineBreakStrategyDefault, StyleManager{});
  const std::vector english_sentence_results{
      BoundaryType::kGraphme,       BoundaryType::kWord,
      BoundaryType::kLineBreakable, BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kWord,
      BoundaryType::kLineBreakable, BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kGraphme,
      BoundaryType::kGraphme,       BoundaryType::kGraphme,
      BoundaryType::kWord};
  for (auto i = 1ull; i < english_sentence.size(); ++i) {
    EXPECT_EQ(english_sentence_results[i], english_analyst.GetBoundaryType(i));
  }
}

TEST(BoundaryAnalystTest, DisableAvoidBreakAroundPunctuation) {
  constexpr std::u32string_view text = U"一段文本(一段文本)一段文本";
  const BoundaryAnalyst boundary_analyst(
      text.data(), text.size(),
      static_cast<LineBreakStrategy>(0xff ^ kAvoidBreakAroundPunctuation),
      StyleManager{});
  for (auto i = 1ull; i < text.size(); ++i) {
    EXPECT_EQ(BoundaryType::kLineBreakable,
              boundary_analyst.GetBoundaryType(i));
  }
}

TEST(BoundaryAnalystTest, OutOfRangeAccessFallsBackToLineBreakable) {
  constexpr std::u32string_view text = U"text";
  const BoundaryAnalyst boundary_analyst(
      text.data(), text.size(), kLineBreakStrategyDefault, StyleManager{});

  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryType(text.size()));
  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryTypeBefore(text.size() + 1));
}

TEST(BoundaryAnalystTest, EmptyBoundaryAccessFallsBackToLineBreakable) {
  const BoundaryAnalyst boundary_analyst(nullptr, 0, kLineBreakStrategyDefault,
                                         StyleManager{});

  EXPECT_EQ(BoundaryType::kLineBreakable, boundary_analyst.GetBoundaryType(27));
  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryTypeBefore(28));
  EXPECT_EQ(
      0u, boundary_analyst.FindNextBoundary(27, BoundaryType::kLineBreakable));
  EXPECT_EQ(
      0u, boundary_analyst.FindPrevBoundary(28, BoundaryType::kLineBreakable));
}

TEST(BoundaryAnalystTest, NullContentUsesSafeFallbackBoundaries) {
  constexpr uint32_t kCharCount = 28;
  const BoundaryAnalyst boundary_analyst(
      nullptr, kCharCount, kLineBreakStrategyDefault, StyleManager{});

  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryType(kCharCount - 1));
  EXPECT_EQ(kCharCount, boundary_analyst.FindPrevBoundary(
                            kCharCount + 1, BoundaryType::kLineBreakable));
}

TEST(BoundaryAnalystTest, KeepAllPreservesMixedScriptWords) {
  constexpr std::u32string_view text = U"中文abc123かな한글";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  const BoundaryAnalyst analyst(text.data(), text.size(),
                                kLineBreakStrategyDefault, styles);

  for (auto i = 0u; i + 1 < text.size(); ++i) {
    EXPECT_LT(analyst.GetBoundaryType(i), BoundaryType::kLineBreakable) << i;
  }
  EXPECT_EQ(analyst.FindNextBoundary(0, BoundaryType::kLineBreakable),
            text.size());
  EXPECT_EQ(
      analyst.FindPrevBoundary(text.size() - 1, BoundaryType::kLineBreakable),
      0u);
}

TEST(BoundaryAnalystTest, KeepAllUsesWordBreakStyleRanges) {
  constexpr std::u32string_view text = U"中文文本";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 1, 2);
  const BoundaryAnalyst analyst(text.data(), text.size(),
                                kLineBreakStrategyDefault, styles);

  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(1), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(2), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(3), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.FindNextBoundary(1, BoundaryType::kWord), 2u);
}

TEST(BoundaryAnalystTest, KeepAllPreservesSpacesAndExplicitBreaks) {
  constexpr std::u32string_view text = U"中文 文本\u200b测试\n换行";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  BoundaryAnalyst analyst(text.data(), text.size(), kLineBreakStrategyDefault,
                          styles);
  analyst.UpgradeBoundaryType(Range::MakeLW(8, 1),
                              BoundaryType::kMustLineBreak);

  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(2), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(3), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(5), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(6), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(8), BoundaryType::kMustLineBreak);
  EXPECT_EQ(analyst.GetBoundaryType(9), BoundaryType::kWord);
}

TEST(BoundaryAnalystTest, KeepAllPreservesPunctuationRules) {
  constexpr std::u32string_view text = U"中文（测试），后文";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  const BoundaryAnalyst analyst(text.data(), text.size(),
                                kLineBreakStrategyDefault, styles);
  const std::vector expected{
      BoundaryType::kWord,          BoundaryType::kLineBreakable,
      BoundaryType::kWord,          BoundaryType::kWord,
      BoundaryType::kWord,          BoundaryType::kWord,
      BoundaryType::kLineBreakable, BoundaryType::kWord,
      BoundaryType::kLineBreakable};

  for (auto i = 0u; i < text.size(); ++i) {
    EXPECT_EQ(analyst.GetBoundaryType(i), expected[i]) << i;
  }
}

TEST(BoundaryAnalystTest, KanaPunctuationTakesPriorityOverCJK) {
  constexpr std::u32string_view text = U"ア・イ゠ウ";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  const BoundaryAnalyst analyst(text.data(), text.size(),
                                kLineBreakStrategyDefault, styles);

  for (auto i = 0u; i < text.size(); ++i) {
    EXPECT_EQ(analyst.GetBoundaryType(i), BoundaryType::kLineBreakable) << i;
  }
}

TEST(BoundaryAnalystTest, KeepAllAllowsSoftBoundaryUpgrades) {
  constexpr std::u32string_view text = U"中文";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  BoundaryAnalyst analyst(text.data(), text.size(), kLineBreakStrategyDefault,
                          styles);

  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kWord);
  analyst.UpgradeBoundaryType(Range::MakeLW(0, 1),
                              BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kLineBreakable);
  analyst.UpgradeBoundaryType(Range::MakeLW(0, 1),
                              BoundaryType::kMustLineBreak);
  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kMustLineBreak);
}

TEST(BoundaryAnalystTest, KeepAllAppliesWithPunctuationRestrictionDisabled) {
  constexpr std::u32string_view text = U"中文，文本";
  StyleManager styles;
  styles.SetWordBreak(WordBreakType::kKeepAll, 0, text.size());
  const BoundaryAnalyst analyst(
      text.data(), text.size(),
      static_cast<LineBreakStrategy>(0xff ^ kAvoidBreakAroundPunctuation),
      styles);

  EXPECT_EQ(analyst.GetBoundaryType(0), BoundaryType::kWord);
  EXPECT_EQ(analyst.GetBoundaryType(1), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(2), BoundaryType::kLineBreakable);
  EXPECT_EQ(analyst.GetBoundaryType(3), BoundaryType::kWord);
}
