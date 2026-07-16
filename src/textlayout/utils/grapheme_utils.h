// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
#define SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_

#include <cstdint>

namespace ttoffice {
namespace tttext {

struct UnicodeRange {
  uint32_t start;
  uint32_t end;
};

inline bool IsInUnicodeRanges(uint32_t codepoint, const UnicodeRange* ranges,
                              uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (codepoint >= ranges[i].start && codepoint <= ranges[i].end) {
      return true;
    }
  }
  return false;
}

inline bool IsVariationSelector(uint32_t codepoint) {
  return (codepoint >= 0xFE00 && codepoint <= 0xFE0F) ||
         (codepoint >= 0xE0100 && codepoint <= 0xE01EF);
}

inline bool IsEmojiModifier(uint32_t codepoint) {
  return codepoint >= 0x1F3FB && codepoint <= 0x1F3FF;
}

inline bool IsEmojiRegionalIndicator(uint32_t codepoint) {
  return codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF;
}

inline bool IsEmojiKeycapBase(uint32_t codepoint) {
  return (codepoint >= 0x30 && codepoint <= 0x39) || codepoint == 0x23 ||
         codepoint == 0x2A;
}

inline bool IsCombiningEnclosingKeycap(uint32_t codepoint) {
  return codepoint == 0x20E3;
}

inline bool IsEmojiKeycapSequence(const char32_t* text, uint32_t start,
                                  uint32_t end) {
  return text != nullptr && start < end && end - start >= 3 &&
         IsEmojiKeycapBase(text[start]) && text[start + 1] == 0xFE0F &&
         IsCombiningEnclosingKeycap(text[start + 2]);
}

inline bool IsEmojiTagCharacter(uint32_t codepoint) {
  return codepoint >= 0xE0020 && codepoint <= 0xE007F;
}

inline bool IsZeroWidthJoiner(uint32_t codepoint) {
  return codepoint == 0x200D;
}

inline bool IsZeroWidthNonJoiner(uint32_t codepoint) {
  return codepoint == 0x200C;
}

inline bool IsCommonEmoji(uint32_t codepoint) {
  static constexpr UnicodeRange kCommonEmojiRanges[] = {
      {0x00A9, 0x00A9}, {0x00AE, 0x00AE}, {0x203C, 0x203C},   {0x2049, 0x2049},
      {0x2122, 0x2122}, {0x2139, 0x2139}, {0x2194, 0x2199},   {0x21A9, 0x21AA},
      {0x231A, 0x231B}, {0x2328, 0x2328}, {0x23CF, 0x23CF},   {0x23E9, 0x23F3},
      {0x23F8, 0x23FA}, {0x24C2, 0x24C2}, {0x25AA, 0x25AB},   {0x25B6, 0x25B6},
      {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},   {0x2934, 0x2935},
      {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B55},   {0x3030, 0x3030},
      {0x303D, 0x303D}, {0x3297, 0x3299}, {0x1F000, 0x1FAFF},
  };
  return IsInUnicodeRanges(
      codepoint, kCommonEmojiRanges,
      static_cast<uint32_t>(sizeof(kCommonEmojiRanges) /
                            sizeof(kCommonEmojiRanges[0])));
}

inline bool IsEmojiBaseForFallback(uint32_t codepoint) {
  return IsCommonEmoji(codepoint);
}

inline bool IsEmojiFallbackIgnorable(uint32_t codepoint) {
  return IsVariationSelector(codepoint) || IsEmojiModifier(codepoint) ||
         IsEmojiTagCharacter(codepoint) || IsZeroWidthJoiner(codepoint);
}

inline uint32_t EmojiFallbackKey(const char32_t* text, uint32_t start,
                                 uint32_t end) {
  if (text == nullptr || start >= end) {
    return 0;
  }

  for (uint32_t i = start; i < end; ++i) {
    uint32_t codepoint = text[i];
    if (IsCombiningEnclosingKeycap(codepoint) ||
        IsEmojiRegionalIndicator(codepoint) ||
        IsEmojiBaseForFallback(codepoint)) {
      return codepoint;
    }
  }

  for (uint32_t i = start; i < end; ++i) {
    uint32_t codepoint = text[i];
    if (!IsEmojiFallbackIgnorable(codepoint)) {
      return codepoint;
    }
  }

  return text[start];
}

inline bool IsDevanagari(uint32_t codepoint) {
  return codepoint >= 0x0900 && codepoint <= 0x097F;
}

inline bool IsDevanagariVirama(uint32_t codepoint) {
  return codepoint == 0x094D;
}

inline bool IsDevanagariMarkOrJoiner(uint32_t codepoint) {
  static constexpr UnicodeRange kMarkOrJoinerRanges[] = {
      {0x0900, 0x0903},
      {0x0941, 0x0948},
      {0x0951, 0x0957},
      {0x0962, 0x0963},
  };
  return IsInUnicodeRanges(
             codepoint, kMarkOrJoinerRanges,
             static_cast<uint32_t>(sizeof(kMarkOrJoinerRanges) /
                                   sizeof(kMarkOrJoinerRanges[0]))) ||
         codepoint == 0x093A || codepoint == 0x093C ||
         IsDevanagariVirama(codepoint) || IsZeroWidthNonJoiner(codepoint) ||
         IsZeroWidthJoiner(codepoint);
}

inline bool IsDevanagariTrailingSign(uint32_t codepoint) {
  static constexpr UnicodeRange kTrailingSignRanges[] = {
      {0x093E, 0x0940},
      {0x0949, 0x094C},
      {0x094E, 0x094F},
  };
  return IsInUnicodeRanges(
      codepoint, kTrailingSignRanges,
      static_cast<uint32_t>(sizeof(kTrailingSignRanges) /
                            sizeof(kTrailingSignRanges[0])));
}

inline bool IsDevanagariConsonant(uint32_t codepoint) {
  static constexpr UnicodeRange kConsonantRanges[] = {
      {0x0915, 0x0939},
      {0x0958, 0x095F},
  };
  return IsInUnicodeRanges(codepoint, kConsonantRanges,
                           static_cast<uint32_t>(sizeof(kConsonantRanges) /
                                                 sizeof(kConsonantRanges[0])));
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
