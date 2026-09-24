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

inline bool IsCombiningMark(uint32_t codepoint) {
  if (codepoint < 0x0300) {
    return false;
  }
  static constexpr UnicodeRange kCombiningMarkRanges[] = {
      {0x0300, 0x036F},   {0x0483, 0x0489},   {0x0591, 0x05BD},
      {0x05BF, 0x05BF},   {0x05C1, 0x05C2},   {0x05C4, 0x05C5},
      {0x05C7, 0x05C7},   {0x0610, 0x061A},   {0x064B, 0x065F},
      {0x0670, 0x0670},   {0x06D6, 0x06DC},   {0x06DF, 0x06E4},
      {0x06E7, 0x06E8},   {0x06EA, 0x06ED},   {0x0711, 0x0711},
      {0x0730, 0x074A},   {0x07A6, 0x07B0},   {0x07EB, 0x07F3},
      {0x08D3, 0x08E1},   {0x08E3, 0x08FF},   {0x093A, 0x093C},
      {0x0941, 0x0948},   {0x094D, 0x094D},   {0x0951, 0x0957},
      {0x0962, 0x0963},   {0x0981, 0x0983},   {0x09BC, 0x09BC},
      {0x09BE, 0x09CD},   {0x09D7, 0x09D7},   {0x09E2, 0x09E3},
      {0x09FE, 0x0A03},   {0x0A3C, 0x0A51},   {0x0A70, 0x0A71},
      {0x0A75, 0x0A75},   {0x0A81, 0x0A83},   {0x0ABC, 0x0ABC},
      {0x0ABE, 0x0ACD},   {0x0AE2, 0x0AE3},   {0x0AFA, 0x0B03},
      {0x0B3C, 0x0B3C},   {0x0B3E, 0x0B57},   {0x0B62, 0x0B63},
      {0x0B82, 0x0B82},   {0x0BBE, 0x0BCD},   {0x0BD7, 0x0BD7},
      {0x0C00, 0x0C04},   {0x0C3C, 0x0C3C},   {0x0C3E, 0x0C56},
      {0x0C62, 0x0C63},   {0x0C81, 0x0C83},   {0x0CBC, 0x0CBC},
      {0x0CBE, 0x0CD6},   {0x0CE2, 0x0CE3},   {0x0D00, 0x0D03},
      {0x0D3B, 0x0D3C},   {0x0D3E, 0x0D4D},   {0x0D57, 0x0D57},
      {0x0D62, 0x0D63},   {0x0D81, 0x0D83},   {0x0DCA, 0x0DDF},
      {0x0DF2, 0x0DF3},   {0x0E31, 0x0E31},   {0x0E33, 0x0E3A},
      {0x0E47, 0x0E4E},   {0x0EB1, 0x0EB1},   {0x0EB3, 0x0EBC},
      {0x0EC8, 0x0ECD},   {0x0F18, 0x0F19},   {0x0F35, 0x0F35},
      {0x0F37, 0x0F37},   {0x0F39, 0x0F39},   {0x0F3E, 0x0F3F},
      {0x0F71, 0x0F84},   {0x0F86, 0x0F87},   {0x0F8D, 0x0FBC},
      {0x0FC6, 0x0FC6},   {0x102D, 0x1037},   {0x1039, 0x103E},
      {0x1056, 0x1059},   {0x105E, 0x1060},   {0x1071, 0x1074},
      {0x1082, 0x1082},   {0x1084, 0x1086},   {0x108D, 0x108D},
      {0x109D, 0x109D},   {0x135D, 0x135F},   {0x17B4, 0x17D3},
      {0x17DD, 0x17DD},   {0x180B, 0x180D},   {0x180F, 0x180F},
      {0x1885, 0x1886},   {0x18A9, 0x18A9},   {0x1AB0, 0x1ACE},
      {0x1CD0, 0x1CD2},   {0x1CD4, 0x1CE8},   {0x1CED, 0x1CED},
      {0x1CF4, 0x1CF4},   {0x1CF7, 0x1CF9},   {0x1DC0, 0x1DFF},
      {0x20D0, 0x20F0},   {0x2CEF, 0x2CF1},   {0x2DE0, 0x2DFF},
      {0x302A, 0x302F},   {0x3099, 0x309A},   {0xA66F, 0xA672},
      {0xA674, 0xA67D},   {0xA69E, 0xA69F},   {0xA6F0, 0xA6F1},
      {0xA8E0, 0xA8F1},   {0xA8FF, 0xA8FF},   {0xA980, 0xA983},
      {0xA9B3, 0xA9C0},   {0xFB1E, 0xFB1E},   {0xFE20, 0xFE2F},
      {0x1D165, 0x1D169}, {0x1D16D, 0x1D172}, {0x1D17B, 0x1D182},
      {0x1D185, 0x1D18B}, {0x1D1AA, 0x1D1AD}, {0xE0020, 0xE007F},
  };
  return IsInUnicodeRanges(
      codepoint, kCombiningMarkRanges,
      static_cast<uint32_t>(sizeof(kCombiningMarkRanges) /
                            sizeof(kCombiningMarkRanges[0])));
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

inline bool IsEmojiKeycapSequence(uint32_t base, uint32_t selector,
                                  uint32_t keycap) {
  return IsEmojiKeycapBase(base) && selector == 0xFE0F &&
         IsCombiningEnclosingKeycap(keycap);
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

struct GraphemeRange {
  uint32_t start;
  uint32_t end;
};

inline bool IsGraphemeClusterJoinable(uint32_t codepoint) {
  return IsVariationSelector(codepoint) || IsEmojiModifier(codepoint) ||
         IsEmojiTagCharacter(codepoint) ||
         IsCombiningEnclosingKeycap(codepoint) ||
         IsZeroWidthJoiner(codepoint) || IsZeroWidthNonJoiner(codepoint) ||
         IsEmojiRegionalIndicator(codepoint) || IsEmojiKeycapBase(codepoint) ||
         IsEmojiBaseForFallback(codepoint) || IsDevanagari(codepoint);
}

template <typename GetCodepoint>
inline uint32_t FindGraphemeClusterEnd(uint32_t start, uint32_t count,
                                       const GetCodepoint& get_codepoint) {
  if (start >= count) {
    return start;
  }
  const uint32_t codepoint = get_codepoint(start);

  if (IsEmojiRegionalIndicator(codepoint) && start + 1 < count &&
      IsEmojiRegionalIndicator(get_codepoint(start + 1))) {
    return start + 2;
  }

  if (start + 2 < count &&
      IsEmojiKeycapSequence(codepoint, get_codepoint(start + 1),
                            get_codepoint(start + 2))) {
    return start + 3;
  }

  if (IsEmojiBaseForFallback(codepoint)) {
    uint32_t end = start + 1;
    while (end < count) {
      const uint32_t next = get_codepoint(end);
      if (IsVariationSelector(next) || IsEmojiModifier(next) ||
          IsEmojiTagCharacter(next)) {
        ++end;
        continue;
      }
      if (IsZeroWidthJoiner(next) && end + 1 < count &&
          IsEmojiBaseForFallback(get_codepoint(end + 1))) {
        end += 2;
        continue;
      }
      break;
    }
    return end;
  }

  if (IsDevanagari(codepoint)) {
    uint32_t end = start + 1;
    while (end < count) {
      const uint32_t next = get_codepoint(end);
      if (IsDevanagariMarkOrJoiner(next)) {
        ++end;
        if (IsDevanagariVirama(next)) {
          while (end < count && (IsZeroWidthNonJoiner(get_codepoint(end)) ||
                                 IsZeroWidthJoiner(get_codepoint(end)))) {
            ++end;
          }
          if (end < count && IsDevanagariConsonant(get_codepoint(end))) {
            ++end;
          }
        }
        continue;
      }
      if (IsDevanagariTrailingSign(next)) {
        ++end;
        continue;
      }
      break;
    }
    return end;
  }

  uint32_t end = start + 1;
  while (end < count && IsCombiningMark(get_codepoint(end))) {
    ++end;
  }
  return end;
}

template <typename GetCodepoint>
inline GraphemeRange FindGraphemeCluster(uint32_t offset, uint32_t count,
                                         const GetCodepoint& get_codepoint) {
  if (offset >= count) {
    return {0, 0};
  }
  uint32_t cluster_start = offset;
  while (cluster_start > 0 &&
         (IsGraphemeClusterJoinable(get_codepoint(cluster_start - 1)) ||
          IsCombiningMark(get_codepoint(cluster_start)))) {
    --cluster_start;
  }
  while (cluster_start <= offset) {
    const uint32_t cluster_end =
        FindGraphemeClusterEnd(cluster_start, count, get_codepoint);
    if (cluster_end > offset) {
      return {cluster_start, cluster_end};
    }
    cluster_start = cluster_end;
  }
  return {offset, offset + 1};
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
