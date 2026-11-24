// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_UNICODE_SCRIPT_H_
#define SRC_TEXTLAYOUT_UTILS_UNICODE_SCRIPT_H_

#include <algorithm>
#include <cstdint>

#include "src/textlayout/utils/grapheme_utils.h"

namespace ttoffice {
namespace tttext {

// A small, range-based classifier, not a complete Unicode Script or
// Script_Extensions implementation. Unknown blocks remain unknown. Emoji
// presentation, language, bidi direction and font selection are separate.
enum class UnicodeScript : uint8_t {
  kUnknown,
  kCommon,
  kInherited,
  kLatin,
  kHan,
  kHiragana,
  kKatakana,
  kHangul,
  kArabic,
  kThai,
  kDevanagari,
  kBengali,
  kJavanese,
};

inline UnicodeScript GetUnicodeScript(uint32_t codepoint) {
  if ((codepoint >= 'A' && codepoint <= 'Z') ||
      (codepoint >= 'a' && codepoint <= 'z') ||
      (codepoint >= 0xFF21 && codepoint <= 0xFF3A) ||
      (codepoint >= 0xFF41 && codepoint <= 0xFF5A)) {
    return UnicodeScript::kLatin;
  }
  // Keep common ASCII/CJK punctuation separate from the letter blocks. The
  // remaining block estimates can include reserved or shared characters.
  if (codepoint <= 0x007F || (codepoint >= 0x2000 && codepoint <= 0x206F) ||
      (codepoint >= 0x3000 && codepoint <= 0x303F) ||
      (codepoint >= 0xFE10 && codepoint <= 0xFE1F) ||
      (codepoint >= 0xFE30 && codepoint <= 0xFE6F) ||
      (codepoint >= 0xFF01 && codepoint <= 0xFF20) ||
      (codepoint >= 0xFF3B && codepoint <= 0xFF40) ||
      (codepoint >= 0xFF5B && codepoint <= 0xFF65) || codepoint == 0xFF70 ||
      (codepoint >= 0xFF9E && codepoint <= 0xFF9F)) {
    return IsZeroWidthJoiner(codepoint) || IsZeroWidthNonJoiner(codepoint)
               ? UnicodeScript::kInherited
               : UnicodeScript::kCommon;
  }
  struct ScriptRange {
    uint32_t start;
    uint32_t end;
    UnicodeScript script;
  };
  static constexpr ScriptRange ranges[] = {
      {0x00AA, 0x00AA, UnicodeScript::kLatin},
      {0x00BA, 0x00BA, UnicodeScript::kLatin},
      {0x00C0, 0x00D6, UnicodeScript::kLatin},
      {0x00D8, 0x00F6, UnicodeScript::kLatin},
      {0x00F8, 0x02AF, UnicodeScript::kLatin},
      {0x0600, 0x06FF, UnicodeScript::kArabic},
      {0x0750, 0x077F, UnicodeScript::kArabic},
      {0x0870, 0x08FF, UnicodeScript::kArabic},
      {0x0900, 0x097F, UnicodeScript::kDevanagari},
      {0x0980, 0x09FF, UnicodeScript::kBengali},
      {0x0E00, 0x0E7F, UnicodeScript::kThai},
      {0x1100, 0x11FF, UnicodeScript::kHangul},
      {0x3040, 0x309F, UnicodeScript::kHiragana},
      {0x30A0, 0x30FF, UnicodeScript::kKatakana},
      {0x3130, 0x318F, UnicodeScript::kHangul},
      {0x3400, 0x4DBF, UnicodeScript::kHan},
      {0x4E00, 0x9FFF, UnicodeScript::kHan},
      {0xA960, 0xA97F, UnicodeScript::kHangul},
      {0xA980, 0xA9DF, UnicodeScript::kJavanese},
      {0xAC00, 0xD7FF, UnicodeScript::kHangul},
      {0xF900, 0xFAFF, UnicodeScript::kHan},
      {0xFB50, 0xFDFF, UnicodeScript::kArabic},
      {0xFE70, 0xFEFC, UnicodeScript::kArabic},
      {0xFF66, 0xFF6F, UnicodeScript::kKatakana},
      {0xFF71, 0xFF9D, UnicodeScript::kKatakana},
      {0x10EC0, 0x10EFF, UnicodeScript::kArabic},
      {0x1EE00, 0x1EEFF, UnicodeScript::kArabic},
      {0x20000, 0x2A6DF, UnicodeScript::kHan},
      {0x2A700, 0x2EE5F, UnicodeScript::kHan},
      {0x2F800, 0x2FA1F, UnicodeScript::kHan},
      {0x30000, 0x3347F, UnicodeScript::kHan},
  };
  const auto* end = ranges + sizeof(ranges) / sizeof(ranges[0]);
  const auto* found = std::lower_bound(
      ranges, end, codepoint, [](const ScriptRange& range, uint32_t value) {
        return range.end < value;
      });
  if (found != end && found->start <= codepoint) return found->script;
  // Script-specific marks above retain their script; only otherwise
  // unclassified combining marks inherit it from their context.
  if (IsCombiningMark(codepoint) || IsVariationSelector(codepoint)) {
    return UnicodeScript::kInherited;
  }
  if (IsEmojiCodepoint(codepoint)) return UnicodeScript::kCommon;
  return UnicodeScript::kUnknown;
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_UNICODE_SCRIPT_H_
