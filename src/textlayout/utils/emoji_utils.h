// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_EMOJI_UTILS_H_
#define SRC_TEXTLAYOUT_UTILS_EMOJI_UTILS_H_

#include <cstdint>

namespace ttoffice {
namespace tttext {

struct EmojiUnicodeRange {
  uint32_t start;
  uint32_t end;
};

inline bool IsInEmojiRanges(uint32_t codepoint, const EmojiUnicodeRange* ranges,
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

inline bool IsEmojiTagCharacter(uint32_t codepoint) {
  return codepoint >= 0xE0020 && codepoint <= 0xE007F;
}

inline bool IsZeroWidthJoiner(uint32_t codepoint) {
  return codepoint == 0x200D;
}

inline bool IsCommonEmoji(uint32_t codepoint) {
  static constexpr EmojiUnicodeRange kCommonEmojiRanges[] = {
      {0x00A9, 0x00A9}, {0x00AE, 0x00AE}, {0x203C, 0x203C},   {0x2049, 0x2049},
      {0x2122, 0x2122}, {0x2139, 0x2139}, {0x2194, 0x2199},   {0x21A9, 0x21AA},
      {0x231A, 0x231B}, {0x2328, 0x2328}, {0x23CF, 0x23CF},   {0x23E9, 0x23F3},
      {0x23F8, 0x23FA}, {0x24C2, 0x24C2}, {0x25AA, 0x25AB},   {0x25B6, 0x25B6},
      {0x25C0, 0x25C0}, {0x25FB, 0x25FE}, {0x2600, 0x27BF},   {0x2934, 0x2935},
      {0x2B05, 0x2B07}, {0x2B1B, 0x2B1C}, {0x2B50, 0x2B55},   {0x3030, 0x3030},
      {0x303D, 0x303D}, {0x3297, 0x3299}, {0x1F000, 0x1FAFF},
  };
  return IsInEmojiRanges(codepoint, kCommonEmojiRanges,
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
    if (IsEmojiRegionalIndicator(codepoint) ||
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

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_EMOJI_UTILS_H_
