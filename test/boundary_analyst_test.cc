// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/boundary_analyst.h"

#include "gtest/gtest.h"
#include "test_src.h"
using namespace tttext;
TEST(BoundaryAnalystTest, DefaultBreak) {
  constexpr std::u32string_view text = U"一段文本(一段文本)一段文本";
  const BoundaryAnalyst boundary_analyst(text.data(), text.size(),
                                         kLineBreakStrategyDefault);
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
  const BoundaryAnalyst english_analyst(english_sentence.data(),
                                        english_sentence.size(),
                                        kLineBreakStrategyDefault);
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
      static_cast<LineBreakStrategy>(0xff ^ kAvoidBreakAroundPunctuation));
  for (auto i = 1ull; i < text.size(); ++i) {
    EXPECT_EQ(BoundaryType::kLineBreakable,
              boundary_analyst.GetBoundaryType(i));
  }
}

TEST(BoundaryAnalystTest, OutOfRangeAccessFallsBackToLineBreakable) {
  constexpr std::u32string_view text = U"text";
  const BoundaryAnalyst boundary_analyst(text.data(), text.size(),
                                         kLineBreakStrategyDefault);

  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryType(text.size()));
  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryTypeBefore(text.size() + 1));
}

TEST(BoundaryAnalystTest, EmptyBoundaryAccessFallsBackToLineBreakable) {
  const BoundaryAnalyst boundary_analyst(nullptr, 0, kLineBreakStrategyDefault);

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
  const BoundaryAnalyst boundary_analyst(nullptr, kCharCount,
                                         kLineBreakStrategyDefault);

  EXPECT_EQ(BoundaryType::kLineBreakable,
            boundary_analyst.GetBoundaryType(kCharCount - 1));
  EXPECT_EQ(kCharCount, boundary_analyst.FindPrevBoundary(
                            kCharCount + 1, BoundaryType::kLineBreakable));
}
