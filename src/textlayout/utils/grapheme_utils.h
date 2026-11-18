// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_
#define SRC_TEXTLAYOUT_UTILS_GRAPHEME_UTILS_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ttoffice {
namespace tttext {

struct UnicodeRange {
  uint32_t start;
  uint32_t end;
};

inline bool IsInUnicodeRanges(uint32_t codepoint, const UnicodeRange* ranges,
                              uint32_t count) {
  // The tables are sorted and contain non-overlapping ranges.
  const auto* end = ranges + count;
  const auto* found = std::lower_bound(
      ranges, end, codepoint, [](const UnicodeRange& range, uint32_t value) {
        return range.end < value;
      });
  return found != end && found->start <= codepoint;
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

// Generated from Unicode Emoji 15.0 properties in
// third_party/libcxx/utils/data/unicode/emoji-data.txt.
// Unicode data terms: https://www.unicode.org/terms_of_use.html
// Unknown/newer characters retain system fallback rather than being guessed
// from broad symbol-block ranges.
inline bool IsEmojiCodepoint(uint32_t codepoint) {
  static constexpr UnicodeRange ranges[] = {
      {0x0023, 0x0023},   {0x002A, 0x002A},   {0x0030, 0x0039},
      {0x00A9, 0x00A9},   {0x00AE, 0x00AE},   {0x203C, 0x203C},
      {0x2049, 0x2049},   {0x2122, 0x2122},   {0x2139, 0x2139},
      {0x2194, 0x2199},   {0x21A9, 0x21AA},   {0x231A, 0x231B},
      {0x2328, 0x2328},   {0x23CF, 0x23CF},   {0x23E9, 0x23F3},
      {0x23F8, 0x23FA},   {0x24C2, 0x24C2},   {0x25AA, 0x25AB},
      {0x25B6, 0x25B6},   {0x25C0, 0x25C0},   {0x25FB, 0x25FE},
      {0x2600, 0x2604},   {0x260E, 0x260E},   {0x2611, 0x2611},
      {0x2614, 0x2615},   {0x2618, 0x2618},   {0x261D, 0x261D},
      {0x2620, 0x2620},   {0x2622, 0x2623},   {0x2626, 0x2626},
      {0x262A, 0x262A},   {0x262E, 0x262F},   {0x2638, 0x263A},
      {0x2640, 0x2640},   {0x2642, 0x2642},   {0x2648, 0x2653},
      {0x265F, 0x2660},   {0x2663, 0x2663},   {0x2665, 0x2666},
      {0x2668, 0x2668},   {0x267B, 0x267B},   {0x267E, 0x267F},
      {0x2692, 0x2697},   {0x2699, 0x2699},   {0x269B, 0x269C},
      {0x26A0, 0x26A1},   {0x26A7, 0x26A7},   {0x26AA, 0x26AB},
      {0x26B0, 0x26B1},   {0x26BD, 0x26BE},   {0x26C4, 0x26C5},
      {0x26C8, 0x26C8},   {0x26CE, 0x26CF},   {0x26D1, 0x26D1},
      {0x26D3, 0x26D4},   {0x26E9, 0x26EA},   {0x26F0, 0x26F5},
      {0x26F7, 0x26FA},   {0x26FD, 0x26FD},   {0x2702, 0x2702},
      {0x2705, 0x2705},   {0x2708, 0x270D},   {0x270F, 0x270F},
      {0x2712, 0x2712},   {0x2714, 0x2714},   {0x2716, 0x2716},
      {0x271D, 0x271D},   {0x2721, 0x2721},   {0x2728, 0x2728},
      {0x2733, 0x2734},   {0x2744, 0x2744},   {0x2747, 0x2747},
      {0x274C, 0x274C},   {0x274E, 0x274E},   {0x2753, 0x2755},
      {0x2757, 0x2757},   {0x2763, 0x2764},   {0x2795, 0x2797},
      {0x27A1, 0x27A1},   {0x27B0, 0x27B0},   {0x27BF, 0x27BF},
      {0x2934, 0x2935},   {0x2B05, 0x2B07},   {0x2B1B, 0x2B1C},
      {0x2B50, 0x2B50},   {0x2B55, 0x2B55},   {0x3030, 0x3030},
      {0x303D, 0x303D},   {0x3297, 0x3297},   {0x3299, 0x3299},
      {0x1F004, 0x1F004}, {0x1F0CF, 0x1F0CF}, {0x1F170, 0x1F171},
      {0x1F17E, 0x1F17F}, {0x1F18E, 0x1F18E}, {0x1F191, 0x1F19A},
      {0x1F1E6, 0x1F1FF}, {0x1F201, 0x1F202}, {0x1F21A, 0x1F21A},
      {0x1F22F, 0x1F22F}, {0x1F232, 0x1F23A}, {0x1F250, 0x1F251},
      {0x1F300, 0x1F321}, {0x1F324, 0x1F393}, {0x1F396, 0x1F397},
      {0x1F399, 0x1F39B}, {0x1F39E, 0x1F3F0}, {0x1F3F3, 0x1F3F5},
      {0x1F3F7, 0x1F4FD}, {0x1F4FF, 0x1F53D}, {0x1F549, 0x1F54E},
      {0x1F550, 0x1F567}, {0x1F56F, 0x1F570}, {0x1F573, 0x1F57A},
      {0x1F587, 0x1F587}, {0x1F58A, 0x1F58D}, {0x1F590, 0x1F590},
      {0x1F595, 0x1F596}, {0x1F5A4, 0x1F5A5}, {0x1F5A8, 0x1F5A8},
      {0x1F5B1, 0x1F5B2}, {0x1F5BC, 0x1F5BC}, {0x1F5C2, 0x1F5C4},
      {0x1F5D1, 0x1F5D3}, {0x1F5DC, 0x1F5DE}, {0x1F5E1, 0x1F5E1},
      {0x1F5E3, 0x1F5E3}, {0x1F5E8, 0x1F5E8}, {0x1F5EF, 0x1F5EF},
      {0x1F5F3, 0x1F5F3}, {0x1F5FA, 0x1F64F}, {0x1F680, 0x1F6C5},
      {0x1F6CB, 0x1F6D2}, {0x1F6D5, 0x1F6D7}, {0x1F6DC, 0x1F6E5},
      {0x1F6E9, 0x1F6E9}, {0x1F6EB, 0x1F6EC}, {0x1F6F0, 0x1F6F0},
      {0x1F6F3, 0x1F6FC}, {0x1F7E0, 0x1F7EB}, {0x1F7F0, 0x1F7F0},
      {0x1F90C, 0x1F93A}, {0x1F93C, 0x1F945}, {0x1F947, 0x1F9FF},
      {0x1FA70, 0x1FA7C}, {0x1FA80, 0x1FA88}, {0x1FA90, 0x1FABD},
      {0x1FABF, 0x1FAC5}, {0x1FACE, 0x1FADB}, {0x1FAE0, 0x1FAE8},
      {0x1FAF0, 0x1FAF8},
  };
  return IsInUnicodeRanges(codepoint, ranges,
                           sizeof(ranges) / sizeof(ranges[0]));
}

inline bool HasDefaultEmojiPresentation(uint32_t codepoint) {
  static constexpr UnicodeRange ranges[] = {
      {0x231A, 0x231B},   {0x23E9, 0x23EC},   {0x23F0, 0x23F0},
      {0x23F3, 0x23F3},   {0x25FD, 0x25FE},   {0x2614, 0x2615},
      {0x2648, 0x2653},   {0x267F, 0x267F},   {0x2693, 0x2693},
      {0x26A1, 0x26A1},   {0x26AA, 0x26AB},   {0x26BD, 0x26BE},
      {0x26C4, 0x26C5},   {0x26CE, 0x26CE},   {0x26D4, 0x26D4},
      {0x26EA, 0x26EA},   {0x26F2, 0x26F3},   {0x26F5, 0x26F5},
      {0x26FA, 0x26FA},   {0x26FD, 0x26FD},   {0x2705, 0x2705},
      {0x270A, 0x270B},   {0x2728, 0x2728},   {0x274C, 0x274C},
      {0x274E, 0x274E},   {0x2753, 0x2755},   {0x2757, 0x2757},
      {0x2795, 0x2797},   {0x27B0, 0x27B0},   {0x27BF, 0x27BF},
      {0x2B1B, 0x2B1C},   {0x2B50, 0x2B50},   {0x2B55, 0x2B55},
      {0x1F004, 0x1F004}, {0x1F0CF, 0x1F0CF}, {0x1F18E, 0x1F18E},
      {0x1F191, 0x1F19A}, {0x1F1E6, 0x1F1FF}, {0x1F201, 0x1F201},
      {0x1F21A, 0x1F21A}, {0x1F22F, 0x1F22F}, {0x1F232, 0x1F236},
      {0x1F238, 0x1F23A}, {0x1F250, 0x1F251}, {0x1F300, 0x1F320},
      {0x1F32D, 0x1F335}, {0x1F337, 0x1F37C}, {0x1F37E, 0x1F393},
      {0x1F3A0, 0x1F3CA}, {0x1F3CF, 0x1F3D3}, {0x1F3E0, 0x1F3F0},
      {0x1F3F4, 0x1F3F4}, {0x1F3F8, 0x1F43E}, {0x1F440, 0x1F440},
      {0x1F442, 0x1F4FC}, {0x1F4FF, 0x1F53D}, {0x1F54B, 0x1F54E},
      {0x1F550, 0x1F567}, {0x1F57A, 0x1F57A}, {0x1F595, 0x1F596},
      {0x1F5A4, 0x1F5A4}, {0x1F5FB, 0x1F64F}, {0x1F680, 0x1F6C5},
      {0x1F6CC, 0x1F6CC}, {0x1F6D0, 0x1F6D2}, {0x1F6D5, 0x1F6D7},
      {0x1F6DC, 0x1F6DF}, {0x1F6EB, 0x1F6EC}, {0x1F6F4, 0x1F6FC},
      {0x1F7E0, 0x1F7EB}, {0x1F7F0, 0x1F7F0}, {0x1F90C, 0x1F93A},
      {0x1F93C, 0x1F945}, {0x1F947, 0x1F9FF}, {0x1FA70, 0x1FA7C},
      {0x1FA80, 0x1FA88}, {0x1FA90, 0x1FABD}, {0x1FABF, 0x1FAC5},
      {0x1FACE, 0x1FADB}, {0x1FAE0, 0x1FAE8}, {0x1FAF0, 0x1FAF8},
  };
  return IsInUnicodeRanges(codepoint, ranges,
                           sizeof(ranges) / sizeof(ranges[0]));
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

enum class EmojiPresentation { kDefault, kText, kEmoji };

struct EmojiClusterInfo {
  EmojiPresentation presentation = EmojiPresentation::kDefault;
};

// Analyze the emoji presentation of one grapheme.
inline EmojiClusterInfo GetEmojiClusterInfo(const char32_t* text,
                                            uint32_t start, uint32_t end) {
  EmojiClusterInfo info;
  if (text == nullptr || start >= end) {
    return info;
  }
  uint32_t emoji_base_index = end;
  bool text_presentation = false;
  bool emoji_sequence = false;
  bool keycap = false;
  for (uint32_t i = start; i < end; ++i) {
    const auto ch = text[i];
    if (emoji_base_index == end && IsEmojiCodepoint(ch) &&
        !IsEmojiModifier(ch)) {
      emoji_base_index = i;
    }
    text_presentation |= ch == 0xFE0E;
    emoji_sequence |= ch == 0xFE0F || IsZeroWidthJoiner(ch) ||
                      IsEmojiModifier(ch) || IsEmojiTagCharacter(ch);
    keycap |= IsCombiningEnclosingKeycap(ch);
  }
  if (text_presentation) {
    info.presentation = EmojiPresentation::kText;
  } else if (emoji_base_index != end) {
    const auto base = text[emoji_base_index];
    if ((IsEmojiKeycapBase(base) && keycap) ||
        HasDefaultEmojiPresentation(base) || emoji_sequence) {
      info.presentation = EmojiPresentation::kEmoji;
    }
  }
  return info;
}

enum class DefaultFontKind : size_t {
  kChinese = 0,
  kEmoji,
  kThai,
  kArabic,
  kSystem,
  kFallback,
  kInherited,
};

inline DefaultFontKind ClassifyDefaultFontCodepoint(uint32_t codepoint) {
  struct FontRange {
    uint32_t start;
    uint32_t end;
    DefaultFontKind kind;
  };
  static constexpr FontRange ranges[] = {
      {0x0000, 0x007F, DefaultFontKind::kSystem},
      {0x0600, 0x06FF, DefaultFontKind::kArabic},
      {0x0750, 0x077F, DefaultFontKind::kArabic},
      {0x0870, 0x08FF, DefaultFontKind::kArabic},
      {0x0E00, 0x0E7F, DefaultFontKind::kThai},
      {0x3001, 0x303F, DefaultFontKind::kChinese},
      {0x3400, 0x4DBF, DefaultFontKind::kChinese},
      {0x4E00, 0x9FFF, DefaultFontKind::kChinese},
      {0xF900, 0xFAFF, DefaultFontKind::kChinese},
      {0xFB50, 0xFDFF, DefaultFontKind::kArabic},
      {0xFE10, 0xFE1F, DefaultFontKind::kChinese},
      {0xFE30, 0xFE6F, DefaultFontKind::kChinese},
      {0xFE70, 0xFEFC, DefaultFontKind::kArabic},
      {0xFF01, 0xFF0F, DefaultFontKind::kChinese},
      {0xFF1A, 0xFF20, DefaultFontKind::kChinese},
      {0xFF3B, 0xFF40, DefaultFontKind::kChinese},
      {0xFF5B, 0xFF65, DefaultFontKind::kChinese},
      {0x10EC0, 0x10EFF, DefaultFontKind::kArabic},
      {0x1EE00, 0x1EEFF, DefaultFontKind::kArabic},
      {0x20000, 0x2FFFF, DefaultFontKind::kChinese},
  };
  const auto* end = ranges + sizeof(ranges) / sizeof(ranges[0]);
  const auto* found = std::lower_bound(
      ranges, end, codepoint,
      [](const FontRange& range, uint32_t value) { return range.end < value; });
  return found != end && found->start <= codepoint ? found->kind
                                                   : DefaultFontKind::kFallback;
}

// Classify a complete grapheme using emoji presentation before script ranges.
inline DefaultFontKind ClassifyDefaultFontCluster(std::u32string_view content) {
  const auto cluster = GetEmojiClusterInfo(
      content.data(), 0, static_cast<uint32_t>(content.size()));
  if (cluster.presentation == EmojiPresentation::kText) {
    return DefaultFontKind::kFallback;
  }
  if (cluster.presentation == EmojiPresentation::kEmoji) {
    return DefaultFontKind::kEmoji;
  }
  auto kind = DefaultFontKind::kInherited;
  for (char32_t ch : content) {
    if (IsVariationSelector(ch) || IsCombiningMark(ch) ||
        IsZeroWidthJoiner(ch) || IsZeroWidthNonJoiner(ch)) {
      continue;
    }
    const auto current = ClassifyDefaultFontCodepoint(ch);
    if (kind == DefaultFontKind::kInherited) {
      kind = current;
    } else if (kind != current) {
      return DefaultFontKind::kFallback;
    }
  }
  return kind;
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
