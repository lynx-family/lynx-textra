// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/unicode_script.h"

#include <gtest/gtest.h>

#include "src/ports/shaper/coretext/default_font_policy.h"

namespace ttoffice {
namespace tttext {
namespace {

TEST(UnicodeScriptTest, DistinguishesScriptsWithoutChoosingFonts) {
  const struct {
    char32_t codepoint;
    UnicodeScript script;
  } cases[] = {
      {U'A', UnicodeScript::kLatin},
      {U'é', UnicodeScript::kLatin},
      {U'中', UnicodeScript::kHan},
      {U'あ', UnicodeScript::kHiragana},
      {U'カ', UnicodeScript::kKatakana},
      {U'한', UnicodeScript::kHangul},
      {U'এ', UnicodeScript::kBengali},
      {U'ۣ', UnicodeScript::kArabic},
      {U'ۖ', UnicodeScript::kArabic},
      {U'ิ', UnicodeScript::kThai},
      {U'꧔', UnicodeScript::kJavanese},
      {U'क', UnicodeScript::kDevanagari},
      {U'，', UnicodeScript::kCommon},
      {U'1', UnicodeScript::kCommon},
      {U' ', UnicodeScript::kCommon},
      {U'😀', UnicodeScript::kCommon},
      {U'\u0301', UnicodeScript::kInherited},
      {U'\u200D', UnicodeScript::kInherited},
      {U'\uFE0F', UnicodeScript::kInherited},
      {U'\U00030000', UnicodeScript::kHan},
      {U'\U0002A6E0', UnicodeScript::kUnknown},
      {0x110000, UnicodeScript::kUnknown},
  };
  for (const auto& item : cases) {
    EXPECT_EQ(GetUnicodeScript(item.codepoint), item.script)
        << static_cast<uint32_t>(item.codepoint);
  }
}

TEST(UnicodeScriptTest, ClassifiesHalfwidthKanaAndSharedMarks) {
  for (uint32_t ch = 0xFF61; ch <= 0xFFA0; ++ch) {
    const auto expected = ch == 0xFFA0 ? UnicodeScript::kUnknown
                          : ch <= 0xFF65 || ch == 0xFF70 || ch >= 0xFF9E
                              ? UnicodeScript::kCommon
                              : UnicodeScript::kKatakana;
    EXPECT_EQ(GetUnicodeScript(ch), expected) << ch;
  }
}

TEST(CoreTextDefaultFontPolicyTest, UsesUnicodeScriptForFontSelection) {
  const char32_t codepoints[] = {U'A',          U'é',          U'Ａ', U'中',
                                 U'\U00030000', U'\U0002A6E0', U'ۣ',   U'ิ',
                                 U'এ',          U'꧔',          U'あ', U'한'};
  for (const auto codepoint : codepoints) {
    auto selection = coretext::SelectDefaultFont({&codepoint, 1});
    EXPECT_EQ(selection.script, GetUnicodeScript(codepoint));
    EXPECT_FALSE(selection.emoji);
  }
}

TEST(CoreTextDefaultFontPolicyTest, ResolvesNeutralScriptsUsingContext) {
  for (const char32_t* text : {U",", U" ", U"123", U"\u0301"}) {
    auto selection = coretext::SelectDefaultFont(text);
    EXPECT_EQ(coretext::ResolveDefaultFontContext(selection,
                                                  UnicodeScript::kHan, text)
                  .script,
              UnicodeScript::kHan);
    EXPECT_EQ(coretext::ResolveDefaultFontContext(selection,
                                                  UnicodeScript::kLatin, text)
                  .script,
              UnicodeScript::kLatin);
  }
  auto emoji = coretext::ResolveDefaultFontContext(
      coretext::SelectDefaultFont(U"😀"), UnicodeScript::kHan, U"😀");
  EXPECT_TRUE(emoji.emoji);
  EXPECT_EQ(emoji.script, UnicodeScript::kCommon);
  auto unknown = coretext::ResolveDefaultFontContext(
      coretext::SelectDefaultFont(U"\U0002A6E0"), UnicodeScript::kHan,
      U"\U0002A6E0");
  EXPECT_EQ(unknown.script, UnicodeScript::kUnknown);
}

TEST(CoreTextDefaultFontPolicyTest, SelectsHanFontForCommonCJKPunctuation) {
  for (const char32_t* text :
       {U"，", U"。", U"（", U"）", U"、", U"︐", U"，\u0301"}) {
    const auto selection = coretext::SelectDefaultFont(text);
    EXPECT_EQ(selection.script, UnicodeScript::kCommon);
    for (const auto context : {UnicodeScript::kLatin, UnicodeScript::kHan,
                               UnicodeScript::kArabic, UnicodeScript::kThai}) {
      const auto resolved =
          coretext::ResolveDefaultFontContext(selection, context, text);
      EXPECT_EQ(resolved.script, UnicodeScript::kHan);
      EXPECT_FALSE(resolved.emoji);
    }
  }
}

TEST(CoreTextDefaultFontPolicyTest, PreservesGraphemeAndEmojiPolicy) {
  const struct {
    const char32_t* text;
    UnicodeScript script;
    bool emoji;
  } cases[] = {
      {U"A\u0301", UnicodeScript::kLatin, false},
      {U"中\uFE0F", UnicodeScript::kHan, false},
      {U"এۣۖิ꧔", UnicodeScript::kUnknown, false},
      {U"A中", UnicodeScript::kUnknown, false},
      {U"😀", UnicodeScript::kCommon, true},
      {U"🏳️‍🌈", UnicodeScript::kCommon, true},
      {U"1️⃣", UnicodeScript::kCommon, true},
      {U"\u2764\uFE0E", UnicodeScript::kUnknown, false},
  };
  for (const auto& item : cases) {
    auto selection = coretext::SelectDefaultFont(item.text);
    EXPECT_EQ(selection.script, item.script);
    EXPECT_EQ(selection.emoji, item.emoji);
  }
  // Inherited marks follow context, while script-specific marks keep the
  // script reported by the Unicode classifier.
  EXPECT_EQ(coretext::SelectDefaultFont(U"\u0301").script,
            UnicodeScript::kInherited);
  EXPECT_EQ(coretext::SelectDefaultFont(U"ۣۖ").script, UnicodeScript::kArabic);
  EXPECT_EQ(coretext::SelectDefaultFont(U"ۣۖิ").script, UnicodeScript::kUnknown);
}

}  // namespace
}  // namespace tttext
}  // namespace ttoffice
