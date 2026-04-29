// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/justify_analyst.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

using namespace ttoffice::tttext;

namespace {
void ExpectJustifyOpportunities(
    const char* test_case, const char* text,
    std::initializer_list<bool> expected_opportunities) {
  SCOPED_TRACE(test_case);

  const TTString content(text);
  const JustifyAnalyst justify_analyst(content);
  const std::vector<bool> expected(expected_opportunities);
  const auto char_count = static_cast<CharPos>(expected.size());
  ASSERT_EQ(content.GetCharCount(), char_count);

  for (CharPos char_pos = 0; char_pos < char_count; ++char_pos) {
    EXPECT_EQ(justify_analyst.CanInsertJustifySpaceAfter(char_pos),
              static_cast<bool>(expected[char_pos]))
        << "char_pos=" << char_pos;
  }
  EXPECT_FALSE(justify_analyst.CanInsertJustifySpaceAfter(char_count))
      << "out-of-range char_pos";

  for (CharPos start_char = 0; start_char <= char_count; ++start_char) {
    auto next_opportunity = char_count;
    for (CharPos char_pos = start_char; char_pos < char_count; ++char_pos) {
      if (expected[char_pos]) {
        next_opportunity = char_pos + 1;
        break;
      }
    }
    EXPECT_EQ(justify_analyst.FindNextJustifyOpportunity(start_char),
              next_opportunity)
        << "start_char=" << start_char;
  }
  EXPECT_EQ(justify_analyst.FindNextJustifyOpportunity(char_count + 1),
            char_count)
      << "start_char past end";
}
}  // namespace

TEST(JustifyAnalystTest, JustifyOpportunity) {
  ExpectJustifyOpportunities("empty paragraph", "", {});
  ExpectJustifyOpportunities("single ASCII character", "A", {false});
  ExpectJustifyOpportunities("ASCII word without separators", "abc",
                             {false, false, false});
  ExpectJustifyOpportunities("trailing separator", "A ", {false, false});

  ExpectJustifyOpportunities("ASCII word separator", "A B",
                             {false, true, false});
  ExpectJustifyOpportunities("consecutive word separators", "A  B",
                             {false, false, true, false});
  ExpectJustifyOpportunities("non-breaking word separator",
                             "A"
                             u8"\u00a0"
                             "B",
                             {false, true, false});
  ExpectJustifyOpportunities("non-ASCII word separator",
                             "A"
                             u8"\u1361"
                             "B",
                             {false, true, false});
  ExpectJustifyOpportunities("ideographic word separator",
                             "A"
                             u8"\u3000"
                             "B",
                             {false, true, false});

  ExpectJustifyOpportunities("Chinese and English boundaries", u8"A中B",
                             {true, true, false});
  ExpectJustifyOpportunities("Japanese kana and English boundaries",
                             u8"AあBカC", {true, true, true, true, false});
  ExpectJustifyOpportunities("Korean Hangul and English boundaries", u8"A한B",
                             {true, true, false});
  ExpectJustifyOpportunities("Chinese Japanese Korean boundaries", u8"中あ한",
                             {true, true, false});

  ExpectJustifyOpportunities("Chinese punctuation beside CJK text",
                             u8"你，好。", {true, true, true, false});
  ExpectJustifyOpportunities("Chinese punctuation beside ASCII text", u8"A，B",
                             {true, true, false});
  ExpectJustifyOpportunities("Chinese punctuation without CJK text", u8"，。",
                             {true, false});
  ExpectJustifyOpportunities("Chinese left and right punctuation", u8"（A）",
                             {true, true, false});
  ExpectJustifyOpportunities("CJK corner bracket punctuation", u8"「A」",
                             {true, true, false});
  ExpectJustifyOpportunities("fullwidth ASCII punctuation", u8"A．B",
                             {true, true, false});
  ExpectJustifyOpportunities(
      "English punctuation", "Hi, there.",
      {false, false, false, true, false, false, false, false, false, false});

  ExpectJustifyOpportunities("ASCII digits", "12345",
                             {false, false, false, false, false});
  ExpectJustifyOpportunities("digits with word separator", "12 34",
                             {false, false, true, false, false});
  ExpectJustifyOpportunities("digits beside CJK text", u8"1中2",
                             {true, true, false});
}
