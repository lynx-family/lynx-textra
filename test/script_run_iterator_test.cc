// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/script_run_iterator.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
namespace {

constexpr SkFourByteTag Tag(const char* text) {
  return (static_cast<uint32_t>(text[0]) << 24) |
         (static_cast<uint32_t>(text[1]) << 16) |
         (static_cast<uint32_t>(text[2]) << 8) | text[3];
}
using ScriptRuns = std::vector<std::pair<size_t, SkFourByteTag>>;
ScriptRuns Collect(std::u32string_view text) {
  UnicodeScriptRunIterator iterator(text.data(), text.size());
  ScriptRuns runs;
  size_t previous = 0;
  while (!iterator.atEnd()) {
    iterator.consume();
    EXPECT_GT(iterator.endOfCurrentRun(), previous);
    EXPECT_LE(iterator.endOfCurrentRun(), text.size());
    previous = iterator.endOfCurrentRun();
    runs.emplace_back(previous, iterator.currentScript());
  }
  return runs;
}

TEST(ScriptRunIterator, UsesUTF32OffsetsAndScriptSpecificMarks) {
  EXPECT_EQ(Collect(U"এۣۖิ꧔"), (ScriptRuns{{1, Tag("Beng")},
                                        {3, Tag("Arab")},
                                        {4, Tag("Thai")},
                                        {5, Tag("Java")}}));
  EXPECT_EQ(Collect(U"A\U00030000ا"),
            (ScriptRuns{{1, Tag("Latn")}, {2, Tag("Hani")}, {3, Tag("Arab")}}));
}

TEST(ScriptRunIterator, ResolvesNeutralsUsingUpstreamRules) {
  EXPECT_EQ(
      Collect(U"  A， B 中!ا"),
      (ScriptRuns{{7, Tag("Latn")}, {9, Tag("Hani")}, {10, Tag("Arab")}}));
  EXPECT_EQ(Collect(U"\u0301A\u0301中"),
            (ScriptRuns{{3, Tag("Latn")}, {4, Tag("Hani")}}));
  EXPECT_EQ(Collect(U" \u0301"), (ScriptRuns{{2, Tag("Zyyy")}}));
  EXPECT_EQ(Collect(U"\u0370A\u0370"),
            (ScriptRuns{{1, 0}, {2, Tag("Latn")}, {3, 0}}));
  EXPECT_EQ(Collect(U"தமிழ்"), (ScriptRuns{{5, 0}}));
  EXPECT_TRUE(Collect(U"").empty());
}

TEST(ScriptRunIterator, CoalescesKanaButPreservesOtherScriptBoundaries) {
  EXPECT_EQ(Collect(U"ひらがなカタカナ"), (ScriptRuns{{8, Tag("Hira")}}));
  EXPECT_EQ(Collect(U"カタカナひらがな"), (ScriptRuns{{8, Tag("Kana")}}));
  EXPECT_EQ(Collect(U"か\u3099カ\u3099がガ"), (ScriptRuns{{6, Tag("Hira")}}));
  EXPECT_EQ(Collect(U"日本語のテキストとカタカナ、ひらがな。"),
            (ScriptRuns{{3, Tag("Hani")}, {19, Tag("Hira")}}));
  EXPECT_EQ(Collect(U" カひA中ا"), (ScriptRuns{{3, Tag("Kana")},
                                               {4, Tag("Latn")},
                                               {5, Tag("Hani")},
                                               {6, Tag("Arab")}}));
}

TEST(ScriptRunIterator, KeepsEmojiAndIndicSequencesTogether) {
  for (const auto* text : {U"🏳️‍🌈", U"🇨🇳", U"1️⃣", U"क्\u200Dष",
                           U"\U0001F3F4\U000E0067\U000E0062\U000E007F"}) {
    SCOPED_TRACE(static_cast<uint32_t>(text[0]));
    std::u32string input(text);
    const auto runs = Collect(input);
    ASSERT_EQ(runs.size(), 1u);
    EXPECT_EQ(runs.front().first, input.size());
  }
}

}  // namespace
}  // namespace tttext
}  // namespace ttoffice
