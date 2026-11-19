// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/u_8_string.h"

#include <textra/macro.h>

#include <algorithm>
#include <array>
#include <iterator>

#include "src/textlayout/utils/log_util.h"

namespace ttoffice {
namespace base {

uint32_t U8CharToU32(const char* u8_char, uint32_t* char_len) {
  *char_len = 0;
  uint32_t cp = 0;
  uint8_t nxt = u8_char[(*char_len)++];
  if ((nxt & 0x80u) == 0) {
    cp = nxt & 0x7Fu;
    *char_len = 1;
  } else if ((nxt & 0xE0u) == 0xC0) {
    cp = (nxt & 0x1Fu);
    *char_len = 2;
  } else if ((nxt & 0xF0u) == 0xE0) {
    cp = nxt & 0x0Fu;
    *char_len = 3;
  } else if ((nxt & 0xF8u) == 0xF0) {
    cp = nxt & 0x07u;
    *char_len = 4;
  } else {
    cp = 0;
    return -1;
  }
  for (uint32_t j = 1; j < *char_len; ++j) {
    nxt = u8_char[j];
    if ((nxt & 0xC0u) != 0x80) {
      return -1;
    }
    cp <<= 6u;
    cp |= nxt & 0x3fu;
  }
  return cp;
}
bool IsValidUtf8Bytes(const unsigned char* p, int count) {
  for (int i = 0; i < count; ++i) {
    uint8_t c = p[++i];
    if ((c & 0xC0) != 0x80) {
      return false;
    }
  }
  return true;
}

std::u32string U8StringToU32(const char* u8str, uint32_t length) {
  std::u32string u32str;
  u32str.reserve((uint32_t)(length * 0.75f));
  uint32_t i = 0;

  while (i < length) {
    uint32_t utf32 = static_cast<unsigned char>(u8str[i]);
    int additionalBytes = 0;

    if ((utf32 & 0x80u) == 0) {
      // Single-byte UTF-8 character
      additionalBytes = 0;
    } else if ((utf32 & 0xE0u) == 0xC0) {
      // Two-byte UTF-8 character
      utf32 &= 0x1Fu;
      additionalBytes = 1;
    } else if ((utf32 & 0xF0u) == 0xE0) {
      // Three-byte UTF-8 character
      utf32 &= 0x0Fu;
      additionalBytes = 2;
    } else if ((utf32 & 0xF8u) == 0xF0) {
      // Four-byte UTF-8 character
      utf32 &= 0x07u;
      additionalBytes = 3;
    } else {
      LogUtil::E("Invalid UTF-8 encoding");
    }

    for (int j = 0; j < additionalBytes; ++j) {
      if (++i >= length) {
        LogUtil::E("Invalid UTF-8 encoding");
      }

      auto byte = static_cast<unsigned char>(u8str[i]);
      if ((byte & 0xC0u) != 0x80) {
        LogUtil::E("Invalid UTF-8 encoding");
      }

      utf32 = (utf32 << 6u) | (byte & 0x3Fu);
    }

    u32str.push_back(utf32);
    ++i;
  }

  return u32str;
}
std::u32string U16StringToU32(const char16_t* u16str, uint32_t length) {
  std::u32string u32str;
  u32str.reserve((uint32_t)(length * 0.75f));
  char32_t char32 = 0;

  uint32_t i = 0;
  while (i < length) {
    i += U16CharToU32(u16str + i, length - i, &char32);
    u32str.push_back(char32);
  }
  return u32str;
}
std::u16string U32StringToU16(const char32_t* u32str, uint32_t length) {
  std::u16string u16str;
  u16str.reserve(length * 2);

  for (uint32_t i = 0; i < length; ++i) {
    uint32_t utf32 = u32str[i];

    if (utf32 <= 0xD7FF || (utf32 >= 0xE000 && utf32 <= 0xFFFF)) {
      u16str.push_back(static_cast<char16_t>(utf32));
    } else if (utf32 >= 0x10000 && utf32 <= 0x10FFFF) {
      utf32 -= 0x10000;
      u16str.push_back(static_cast<char16_t>(0xD800 | ((utf32 >> 10) & 0x3FF)));
      u16str.push_back(static_cast<char16_t>(0xDC00 | (utf32 & 0x3FF)));
    } else {
      LogUtil::E("U32StringToU16 Invalid UTF-32 encoding");
    }
  }

  return u16str;
}
std::string U32StringToU8(const char32_t* u32str, uint32_t length) {
  std::string utf8;
  utf8.reserve(length * 3);

  for (uint32_t i = 0; i < length; ++i) {
    auto utf32 = u32str[i];

    if (utf32 <= 0x7F) {
      utf8.push_back(utf32);
    } else if (utf32 <= 0x7FF) {
      utf8.push_back(0xC0 | ((utf32 >> 6u) & 0x1Fu));
      utf8.push_back(0x80 | (utf32 & 0x3Fu));
    } else if (utf32 <= 0xFFFF) {
      utf8.push_back(0xE0 | ((utf32 >> 12u) & 0x0Fu));
      utf8.push_back(0x80 | ((utf32 >> 6u) & 0x3Fu));
      utf8.push_back(0x80 | (utf32 & 0x3Fu));
    } else if (utf32 <= 0x10FFFF) {
      utf8.push_back(0xF0 | ((utf32 >> 18u) & 0x07u));
      utf8.push_back(0x80 | ((utf32 >> 12u) & 0x3Fu));
      utf8.push_back(0x80 | ((utf32 >> 6u) & 0x3Fu));
      utf8.push_back(0x80 | (utf32 & 0x3Fu));
    } else {
      LogUtil::E("U32StringToU8 convert error");
    }
  }
  return utf8;
}
uint32_t U32IndexToU16(const char32_t* u32str, uint32_t length,
                       uint32_t index) {
  uint32_t ret = 0;
  for (uint32_t i = 0; i < std::min(length, index); ++i) {
    uint32_t utf32 = u32str[i];

    if (utf32 <= 0xD7FF || (utf32 >= 0xE000 && utf32 <= 0xFFFF)) {
      ret += 1;
    } else if (utf32 >= 0x10000 && utf32 <= 0x10FFFF) {
      ret += 2;
    } else {
      LogUtil::E("U32StringToU16 Invalid UTF-32 encoding");
    }
  }
  return ret;
}

namespace {
struct UnicodeRange {
  uint32_t start;
  uint32_t end;
};

struct Boundary {
  uint32_t start;
  uint32_t properties;
};

template <size_t Capacity>
struct UnicodePropertyTableBuilder {
  std::array<Boundary, Capacity> entries{};
  size_t size = 1;

  // Merge one sorted, non-overlapping source table into the current boundary
  // table. Each overlapping range contributes its property bits.
  template <size_t Count>
  constexpr void AddRanges(const UnicodeRange (&ranges)[Count], uint32_t bits) {
    std::array<Boundary, Capacity> updated{};
    size_t updated_size = 0;
    size_t entry_index = 0;
    size_t range_index = 0;
    bool in_range = false;
    uint32_t base = 0;
    constexpr uint32_t kNoBoundary = UINT32_MAX;
    while (entry_index < size || range_index < Count) {
      const auto entry_start =
          entry_index < size ? entries[entry_index].start : kNoBoundary;
      const auto range_start = range_index < Count
                                   ? (in_range ? ranges[range_index].end + 1
                                               : ranges[range_index].start)
                                   : kNoBoundary;
      const auto start = std::min(entry_start, range_start);
      if (entry_start == start) {
        base = entries[entry_index++].properties;
      }
      // Adjacent source ranges can end and start at the same boundary.
      while (range_index < Count &&
             (in_range ? ranges[range_index].end + 1
                       : ranges[range_index].start) == start) {
        if (in_range) ++range_index;
        in_range = !in_range;
      }
      const auto properties = in_range ? base | bits : base;
      if (updated_size == 0 ||
          updated[updated_size - 1].properties != properties) {
        updated[updated_size++] = {start, properties};
      }
    }
    entries = updated;
    size = updated_size;
  }
};

template <typename... Tables>
constexpr size_t BoundaryCapacity(const Tables&... tables) {
  return 1 + 2 * (... + std::size(tables));
}

constexpr auto BuildUnicodePropertyTable() {
  // Generated by tools/generate_unicode_ranges.py from Unicode 17.0.0.
  // Data copyright © 2025 Unicode, Inc. See NOTICE.
  // Emoji without Emoji_Presentation.
  constexpr UnicodeRange kEmojiTextDefault[] = {
      {0x0023, 0x0023},   {0x002A, 0x002A},   {0x0030, 0x0039},
      {0x00A9, 0x00A9},   {0x00AE, 0x00AE},   {0x203C, 0x203C},
      {0x2049, 0x2049},   {0x2122, 0x2122},   {0x2139, 0x2139},
      {0x2194, 0x2199},   {0x21A9, 0x21AA},   {0x2328, 0x2328},
      {0x23CF, 0x23CF},   {0x23ED, 0x23EF},   {0x23F1, 0x23F2},
      {0x23F8, 0x23FA},   {0x24C2, 0x24C2},   {0x25AA, 0x25AB},
      {0x25B6, 0x25B6},   {0x25C0, 0x25C0},   {0x25FB, 0x25FC},
      {0x2600, 0x2604},   {0x260E, 0x260E},   {0x2611, 0x2611},
      {0x2618, 0x2618},   {0x261D, 0x261D},   {0x2620, 0x2620},
      {0x2622, 0x2623},   {0x2626, 0x2626},   {0x262A, 0x262A},
      {0x262E, 0x262F},   {0x2638, 0x263A},   {0x2640, 0x2640},
      {0x2642, 0x2642},   {0x265F, 0x2660},   {0x2663, 0x2663},
      {0x2665, 0x2666},   {0x2668, 0x2668},   {0x267B, 0x267B},
      {0x267E, 0x267E},   {0x2692, 0x2692},   {0x2694, 0x2697},
      {0x2699, 0x2699},   {0x269B, 0x269C},   {0x26A0, 0x26A0},
      {0x26A7, 0x26A7},   {0x26B0, 0x26B1},   {0x26C8, 0x26C8},
      {0x26CF, 0x26CF},   {0x26D1, 0x26D1},   {0x26D3, 0x26D3},
      {0x26E9, 0x26E9},   {0x26F0, 0x26F1},   {0x26F4, 0x26F4},
      {0x26F7, 0x26F9},   {0x2702, 0x2702},   {0x2708, 0x2709},
      {0x270C, 0x270D},   {0x270F, 0x270F},   {0x2712, 0x2712},
      {0x2714, 0x2714},   {0x2716, 0x2716},   {0x271D, 0x271D},
      {0x2721, 0x2721},   {0x2733, 0x2734},   {0x2744, 0x2744},
      {0x2747, 0x2747},   {0x2763, 0x2764},   {0x27A1, 0x27A1},
      {0x2934, 0x2935},   {0x2B05, 0x2B07},   {0x3030, 0x3030},
      {0x303D, 0x303D},   {0x3297, 0x3297},   {0x3299, 0x3299},
      {0x1F170, 0x1F171}, {0x1F17E, 0x1F17F}, {0x1F202, 0x1F202},
      {0x1F237, 0x1F237}, {0x1F321, 0x1F321}, {0x1F324, 0x1F32C},
      {0x1F336, 0x1F336}, {0x1F37D, 0x1F37D}, {0x1F396, 0x1F397},
      {0x1F399, 0x1F39B}, {0x1F39E, 0x1F39F}, {0x1F3CB, 0x1F3CE},
      {0x1F3D4, 0x1F3DF}, {0x1F3F3, 0x1F3F3}, {0x1F3F5, 0x1F3F5},
      {0x1F3F7, 0x1F3F7}, {0x1F43F, 0x1F43F}, {0x1F441, 0x1F441},
      {0x1F4FD, 0x1F4FD}, {0x1F549, 0x1F54A}, {0x1F56F, 0x1F570},
      {0x1F573, 0x1F579}, {0x1F587, 0x1F587}, {0x1F58A, 0x1F58D},
      {0x1F590, 0x1F590}, {0x1F5A5, 0x1F5A5}, {0x1F5A8, 0x1F5A8},
      {0x1F5B1, 0x1F5B2}, {0x1F5BC, 0x1F5BC}, {0x1F5C2, 0x1F5C4},
      {0x1F5D1, 0x1F5D3}, {0x1F5DC, 0x1F5DE}, {0x1F5E1, 0x1F5E1},
      {0x1F5E3, 0x1F5E3}, {0x1F5E8, 0x1F5E8}, {0x1F5EF, 0x1F5EF},
      {0x1F5F3, 0x1F5F3}, {0x1F5FA, 0x1F5FA}, {0x1F6CB, 0x1F6CB},
      {0x1F6CD, 0x1F6CF}, {0x1F6E0, 0x1F6E5}, {0x1F6E9, 0x1F6E9},
      {0x1F6F0, 0x1F6F0}, {0x1F6F3, 0x1F6F3},
  };

  // Emoji_Presentation.
  constexpr UnicodeRange kEmojiPresentation[] = {
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
      {0x1F6CC, 0x1F6CC}, {0x1F6D0, 0x1F6D2}, {0x1F6D5, 0x1F6D8},
      {0x1F6DC, 0x1F6DF}, {0x1F6EB, 0x1F6EC}, {0x1F6F4, 0x1F6FC},
      {0x1F7E0, 0x1F7EB}, {0x1F7F0, 0x1F7F0}, {0x1F90C, 0x1F93A},
      {0x1F93C, 0x1F945}, {0x1F947, 0x1F9FF}, {0x1FA70, 0x1FA7C},
      {0x1FA80, 0x1FA8A}, {0x1FA8E, 0x1FAC6}, {0x1FAC8, 0x1FAC8},
      {0x1FACD, 0x1FADC}, {0x1FADF, 0x1FAEA}, {0x1FAEF, 0x1FAF8},
  };

  // Variation_Selector.
  constexpr UnicodeRange kVariationSelector[] = {
      {0x180B, 0x180D},
      {0x180F, 0x180F},
      {0xFE00, 0xFE0F},
      {0xE0100, 0xE01EF},
  };

  // Unified_Ideograph.
  constexpr UnicodeRange kUnifiedIdeograph[] = {
      {0x3400, 0x4DBF},   {0x4E00, 0x9FFF},   {0xFA0E, 0xFA0F},
      {0xFA11, 0xFA11},   {0xFA13, 0xFA14},   {0xFA1F, 0xFA1F},
      {0xFA21, 0xFA21},   {0xFA23, 0xFA24},   {0xFA27, 0xFA29},
      {0x20000, 0x2A6DF}, {0x2A700, 0x2B81D}, {0x2B820, 0x2CEAD},
      {0x2CEB0, 0x2EBE0}, {0x2EBF0, 0x2EE5D}, {0x30000, 0x3134A},
      {0x31350, 0x33479},
  };

  // General_Category = Lo in the two CJK Compatibility Ideographs blocks.
  constexpr UnicodeRange kCJKCompatibilityIdeograph[] = {
      {0xF900, 0xFA6D},
      {0xFA70, 0xFAD9},
      {0x2F800, 0x2FA1D},
  };

  // White_Space excluding Line_Break = Glue, plus Line_Break = ZWSpace.
  constexpr UnicodeRange kTrimmableSpace[] = {
      {0x0009, 0x000D}, {0x0020, 0x0020}, {0x0085, 0x0085},
      {0x1680, 0x1680}, {0x2000, 0x2006}, {0x2008, 0x200B},
      {0x2028, 0x2029}, {0x205F, 0x205F}, {0x3000, 0x3000},
  };

  // General_Category = Mark and Grapheme_Cluster_Break = Extend or SpacingMark.
  constexpr UnicodeRange kMarkAndExtender[] = {
      {0x0300, 0x036F},   {0x0483, 0x0489},   {0x0591, 0x05BD},
      {0x05BF, 0x05BF},   {0x05C1, 0x05C2},   {0x05C4, 0x05C5},
      {0x05C7, 0x05C7},   {0x0610, 0x061A},   {0x064B, 0x065F},
      {0x0670, 0x0670},   {0x06D6, 0x06DC},   {0x06DF, 0x06E4},
      {0x06E7, 0x06E8},   {0x06EA, 0x06ED},   {0x0711, 0x0711},
      {0x0730, 0x074A},   {0x07A6, 0x07B0},   {0x07EB, 0x07F3},
      {0x07FD, 0x07FD},   {0x0816, 0x0819},   {0x081B, 0x0823},
      {0x0825, 0x0827},   {0x0829, 0x082D},   {0x0859, 0x085B},
      {0x0897, 0x089F},   {0x08CA, 0x08E1},   {0x08E3, 0x0903},
      {0x093A, 0x093C},   {0x093E, 0x094F},   {0x0951, 0x0957},
      {0x0962, 0x0963},   {0x0981, 0x0983},   {0x09BC, 0x09BC},
      {0x09BE, 0x09C4},   {0x09C7, 0x09C8},   {0x09CB, 0x09CD},
      {0x09D7, 0x09D7},   {0x09E2, 0x09E3},   {0x09FE, 0x09FE},
      {0x0A01, 0x0A03},   {0x0A3C, 0x0A3C},   {0x0A3E, 0x0A42},
      {0x0A47, 0x0A48},   {0x0A4B, 0x0A4D},   {0x0A51, 0x0A51},
      {0x0A70, 0x0A71},   {0x0A75, 0x0A75},   {0x0A81, 0x0A83},
      {0x0ABC, 0x0ABC},   {0x0ABE, 0x0AC5},   {0x0AC7, 0x0AC9},
      {0x0ACB, 0x0ACD},   {0x0AE2, 0x0AE3},   {0x0AFA, 0x0AFF},
      {0x0B01, 0x0B03},   {0x0B3C, 0x0B3C},   {0x0B3E, 0x0B44},
      {0x0B47, 0x0B48},   {0x0B4B, 0x0B4D},   {0x0B55, 0x0B57},
      {0x0B62, 0x0B63},   {0x0B82, 0x0B82},   {0x0BBE, 0x0BC2},
      {0x0BC6, 0x0BC8},   {0x0BCA, 0x0BCD},   {0x0BD7, 0x0BD7},
      {0x0C00, 0x0C04},   {0x0C3C, 0x0C3C},   {0x0C3E, 0x0C44},
      {0x0C46, 0x0C48},   {0x0C4A, 0x0C4D},   {0x0C55, 0x0C56},
      {0x0C62, 0x0C63},   {0x0C81, 0x0C83},   {0x0CBC, 0x0CBC},
      {0x0CBE, 0x0CC4},   {0x0CC6, 0x0CC8},   {0x0CCA, 0x0CCD},
      {0x0CD5, 0x0CD6},   {0x0CE2, 0x0CE3},   {0x0CF3, 0x0CF3},
      {0x0D00, 0x0D03},   {0x0D3B, 0x0D3C},   {0x0D3E, 0x0D44},
      {0x0D46, 0x0D48},   {0x0D4A, 0x0D4D},   {0x0D57, 0x0D57},
      {0x0D62, 0x0D63},   {0x0D81, 0x0D83},   {0x0DCA, 0x0DCA},
      {0x0DCF, 0x0DD4},   {0x0DD6, 0x0DD6},   {0x0DD8, 0x0DDF},
      {0x0DF2, 0x0DF3},   {0x0E31, 0x0E31},   {0x0E34, 0x0E3A},
      {0x0E47, 0x0E4E},   {0x0EB1, 0x0EB1},   {0x0EB4, 0x0EBC},
      {0x0EC8, 0x0ECE},   {0x0F18, 0x0F19},   {0x0F35, 0x0F35},
      {0x0F37, 0x0F37},   {0x0F39, 0x0F39},   {0x0F3E, 0x0F3F},
      {0x0F71, 0x0F84},   {0x0F86, 0x0F87},   {0x0F8D, 0x0F97},
      {0x0F99, 0x0FBC},   {0x0FC6, 0x0FC6},   {0x102D, 0x1037},
      {0x1039, 0x103E},   {0x1056, 0x1059},   {0x105E, 0x1060},
      {0x1071, 0x1074},   {0x1082, 0x1082},   {0x1084, 0x1086},
      {0x108D, 0x108D},   {0x109D, 0x109D},   {0x135D, 0x135F},
      {0x1712, 0x1715},   {0x1732, 0x1734},   {0x1752, 0x1753},
      {0x1772, 0x1773},   {0x17B4, 0x17D3},   {0x17DD, 0x17DD},
      {0x180B, 0x180D},   {0x180F, 0x180F},   {0x1885, 0x1886},
      {0x18A9, 0x18A9},   {0x1920, 0x192B},   {0x1930, 0x193B},
      {0x1A17, 0x1A1B},   {0x1A55, 0x1A5E},   {0x1A60, 0x1A60},
      {0x1A62, 0x1A62},   {0x1A65, 0x1A7C},   {0x1A7F, 0x1A7F},
      {0x1AB0, 0x1ADD},   {0x1AE0, 0x1AEB},   {0x1B00, 0x1B04},
      {0x1B34, 0x1B44},   {0x1B6B, 0x1B73},   {0x1B80, 0x1B82},
      {0x1BA1, 0x1BAD},   {0x1BE6, 0x1BF3},   {0x1C24, 0x1C37},
      {0x1CD0, 0x1CD2},   {0x1CD4, 0x1CE8},   {0x1CED, 0x1CED},
      {0x1CF4, 0x1CF4},   {0x1CF7, 0x1CF9},   {0x1DC0, 0x1DFF},
      {0x20D0, 0x20F0},   {0x2CEF, 0x2CF1},   {0x2D7F, 0x2D7F},
      {0x2DE0, 0x2DFF},   {0x302A, 0x302F},   {0x3099, 0x309A},
      {0xA66F, 0xA672},   {0xA674, 0xA67D},   {0xA69E, 0xA69F},
      {0xA6F0, 0xA6F1},   {0xA802, 0xA802},   {0xA806, 0xA806},
      {0xA80B, 0xA80B},   {0xA823, 0xA827},   {0xA82C, 0xA82C},
      {0xA880, 0xA881},   {0xA8B4, 0xA8C5},   {0xA8E0, 0xA8F1},
      {0xA8FF, 0xA8FF},   {0xA926, 0xA92D},   {0xA947, 0xA953},
      {0xA980, 0xA983},   {0xA9B3, 0xA9C0},   {0xA9E5, 0xA9E5},
      {0xAA29, 0xAA36},   {0xAA43, 0xAA43},   {0xAA4C, 0xAA4D},
      {0xAA7C, 0xAA7C},   {0xAAB0, 0xAAB0},   {0xAAB2, 0xAAB4},
      {0xAAB7, 0xAAB8},   {0xAABE, 0xAABF},   {0xAAC1, 0xAAC1},
      {0xAAEB, 0xAAEF},   {0xAAF5, 0xAAF6},   {0xABE3, 0xABEA},
      {0xABEC, 0xABED},   {0xFB1E, 0xFB1E},   {0xFE00, 0xFE0F},
      {0xFE20, 0xFE2F},   {0x101FD, 0x101FD}, {0x102E0, 0x102E0},
      {0x10376, 0x1037A}, {0x10A01, 0x10A03}, {0x10A05, 0x10A06},
      {0x10A0C, 0x10A0F}, {0x10A38, 0x10A3A}, {0x10A3F, 0x10A3F},
      {0x10AE5, 0x10AE6}, {0x10D24, 0x10D27}, {0x10D69, 0x10D6D},
      {0x10EAB, 0x10EAC}, {0x10EFA, 0x10EFF}, {0x10F46, 0x10F50},
      {0x10F82, 0x10F85}, {0x11000, 0x11002}, {0x11038, 0x11046},
      {0x11070, 0x11070}, {0x11073, 0x11074}, {0x1107F, 0x11082},
      {0x110B0, 0x110BA}, {0x110C2, 0x110C2}, {0x11100, 0x11102},
      {0x11127, 0x11134}, {0x11145, 0x11146}, {0x11173, 0x11173},
      {0x11180, 0x11182}, {0x111B3, 0x111C0}, {0x111C9, 0x111CC},
      {0x111CE, 0x111CF}, {0x1122C, 0x11237}, {0x1123E, 0x1123E},
      {0x11241, 0x11241}, {0x112DF, 0x112EA}, {0x11300, 0x11303},
      {0x1133B, 0x1133C}, {0x1133E, 0x11344}, {0x11347, 0x11348},
      {0x1134B, 0x1134D}, {0x11357, 0x11357}, {0x11362, 0x11363},
      {0x11366, 0x1136C}, {0x11370, 0x11374}, {0x113B8, 0x113C0},
      {0x113C2, 0x113C2}, {0x113C5, 0x113C5}, {0x113C7, 0x113CA},
      {0x113CC, 0x113D0}, {0x113D2, 0x113D2}, {0x113E1, 0x113E2},
      {0x11435, 0x11446}, {0x1145E, 0x1145E}, {0x114B0, 0x114C3},
      {0x115AF, 0x115B5}, {0x115B8, 0x115C0}, {0x115DC, 0x115DD},
      {0x11630, 0x11640}, {0x116AB, 0x116B7}, {0x1171D, 0x1171F},
      {0x11722, 0x1172B}, {0x1182C, 0x1183A}, {0x11930, 0x11935},
      {0x11937, 0x11938}, {0x1193B, 0x1193E}, {0x11940, 0x11940},
      {0x11942, 0x11943}, {0x119D1, 0x119D7}, {0x119DA, 0x119E0},
      {0x119E4, 0x119E4}, {0x11A01, 0x11A0A}, {0x11A33, 0x11A39},
      {0x11A3B, 0x11A3E}, {0x11A47, 0x11A47}, {0x11A51, 0x11A5B},
      {0x11A8A, 0x11A99}, {0x11B60, 0x11B67}, {0x11C2F, 0x11C36},
      {0x11C38, 0x11C3F}, {0x11C92, 0x11CA7}, {0x11CA9, 0x11CB6},
      {0x11D31, 0x11D36}, {0x11D3A, 0x11D3A}, {0x11D3C, 0x11D3D},
      {0x11D3F, 0x11D45}, {0x11D47, 0x11D47}, {0x11D8A, 0x11D8E},
      {0x11D90, 0x11D91}, {0x11D93, 0x11D97}, {0x11EF3, 0x11EF6},
      {0x11F00, 0x11F01}, {0x11F03, 0x11F03}, {0x11F34, 0x11F3A},
      {0x11F3E, 0x11F42}, {0x11F5A, 0x11F5A}, {0x13440, 0x13440},
      {0x13447, 0x13455}, {0x1611E, 0x1612F}, {0x16AF0, 0x16AF4},
      {0x16B30, 0x16B36}, {0x16F4F, 0x16F4F}, {0x16F51, 0x16F87},
      {0x16F8F, 0x16F92}, {0x16FE4, 0x16FE4}, {0x16FF0, 0x16FF1},
      {0x1BC9D, 0x1BC9E}, {0x1CF00, 0x1CF2D}, {0x1CF30, 0x1CF46},
      {0x1D165, 0x1D169}, {0x1D16D, 0x1D172}, {0x1D17B, 0x1D182},
      {0x1D185, 0x1D18B}, {0x1D1AA, 0x1D1AD}, {0x1D242, 0x1D244},
      {0x1DA00, 0x1DA36}, {0x1DA3B, 0x1DA6C}, {0x1DA75, 0x1DA75},
      {0x1DA84, 0x1DA84}, {0x1DA9B, 0x1DA9F}, {0x1DAA1, 0x1DAAF},
      {0x1E000, 0x1E006}, {0x1E008, 0x1E018}, {0x1E01B, 0x1E021},
      {0x1E023, 0x1E024}, {0x1E026, 0x1E02A}, {0x1E08F, 0x1E08F},
      {0x1E130, 0x1E136}, {0x1E2AE, 0x1E2AE}, {0x1E2EC, 0x1E2EF},
      {0x1E4EC, 0x1E4EF}, {0x1E5EE, 0x1E5EF}, {0x1E6E3, 0x1E6E3},
      {0x1E6E6, 0x1E6E6}, {0x1E6EE, 0x1E6EF}, {0x1E6F5, 0x1E6F5},
      {0x1E8D0, 0x1E8D6}, {0x1E944, 0x1E94A}, {0xE0100, 0xE01EF},
  };

  // Marks outside the two grapheme extender classes.
  constexpr UnicodeRange kMarkOnly[] = {
      {0x102B, 0x102C},   {0x1038, 0x1038}, {0x1062, 0x1064}, {0x1067, 0x106D},
      {0x1083, 0x1083},   {0x1087, 0x108C}, {0x108F, 0x108F}, {0x109A, 0x109C},
      {0x1A61, 0x1A61},   {0x1A63, 0x1A64}, {0xAA7B, 0xAA7B}, {0xAA7D, 0xAA7D},
      {0x11720, 0x11721},
  };

  // Grapheme extenders outside General_Category = Mark.
  constexpr UnicodeRange kExtenderOnly[] = {
      {0x0E33, 0x0E33}, {0x0EB3, 0x0EB3},   {0x200C, 0x200C},
      {0xFF9E, 0xFF9F}, {0x1F3FB, 0x1F3FF}, {0xE0020, 0xE007F},
  };

  // General_Category = Punctuation in the CJK punctuation blocks used by
  // layout.
  constexpr UnicodeRange kCJKPunctuation[] = {
      {0x3001, 0x3003}, {0x3008, 0x3011}, {0x3014, 0x301F}, {0x3030, 0x3030},
      {0x303D, 0x303D}, {0x30A0, 0x30A0}, {0x30FB, 0x30FB}, {0xFE10, 0xFE19},
      {0xFE30, 0xFE52}, {0xFE54, 0xFE61}, {0xFE63, 0xFE63}, {0xFE68, 0xFE68},
      {0xFE6A, 0xFE6B}, {0xFF01, 0xFF03}, {0xFF05, 0xFF0A}, {0xFF0C, 0xFF0F},
      {0xFF1A, 0xFF1B}, {0xFF1F, 0xFF20}, {0xFF3B, 0xFF3D}, {0xFF3F, 0xFF3F},
      {0xFF5B, 0xFF5B}, {0xFF5D, 0xFF5D}, {0xFF5F, 0xFF65},
  };

  // Block = CJK Symbols and Punctuation or East_Asian_Width = Fullwidth.
  constexpr UnicodeRange kCJKSymbolsAndFullwidthForms[] = {
      {0x3000, 0x303F},
      {0xFF01, 0xFF60},
      {0xFFE0, 0xFFE6},
  };

  // Script = Devanagari.
  constexpr UnicodeRange kScriptDevanagari[] = {
      {0x0900, 0x0950}, {0x0955, 0x0963},   {0x0966, 0x097F},
      {0xA8E0, 0xA8FF}, {0x11B00, 0x11B09},
  };

  // Script = Arabic.
  constexpr UnicodeRange kScriptArabic[] = {
      {0x0600, 0x0604},   {0x0606, 0x060B},   {0x060D, 0x061A},
      {0x061C, 0x061E},   {0x0620, 0x063F},   {0x0641, 0x064A},
      {0x0656, 0x066F},   {0x0671, 0x06DC},   {0x06DE, 0x06FF},
      {0x0750, 0x077F},   {0x0870, 0x0891},   {0x0897, 0x08E1},
      {0x08E3, 0x08FF},   {0xFB50, 0xFD3D},   {0xFD40, 0xFDCF},
      {0xFDF0, 0xFDFF},   {0xFE70, 0xFE74},   {0xFE76, 0xFEFC},
      {0x10E60, 0x10E7E}, {0x10EC2, 0x10EC7}, {0x10ED0, 0x10ED8},
      {0x10EFA, 0x10EFF}, {0x1EE00, 0x1EE03}, {0x1EE05, 0x1EE1F},
      {0x1EE21, 0x1EE22}, {0x1EE24, 0x1EE24}, {0x1EE27, 0x1EE27},
      {0x1EE29, 0x1EE32}, {0x1EE34, 0x1EE37}, {0x1EE39, 0x1EE39},
      {0x1EE3B, 0x1EE3B}, {0x1EE42, 0x1EE42}, {0x1EE47, 0x1EE47},
      {0x1EE49, 0x1EE49}, {0x1EE4B, 0x1EE4B}, {0x1EE4D, 0x1EE4F},
      {0x1EE51, 0x1EE52}, {0x1EE54, 0x1EE54}, {0x1EE57, 0x1EE57},
      {0x1EE59, 0x1EE59}, {0x1EE5B, 0x1EE5B}, {0x1EE5D, 0x1EE5D},
      {0x1EE5F, 0x1EE5F}, {0x1EE61, 0x1EE62}, {0x1EE64, 0x1EE64},
      {0x1EE67, 0x1EE6A}, {0x1EE6C, 0x1EE72}, {0x1EE74, 0x1EE77},
      {0x1EE79, 0x1EE7C}, {0x1EE7E, 0x1EE7E}, {0x1EE80, 0x1EE89},
      {0x1EE8B, 0x1EE9B}, {0x1EEA1, 0x1EEA3}, {0x1EEA5, 0x1EEA9},
      {0x1EEAB, 0x1EEBB}, {0x1EEF0, 0x1EEF1},
  };

  // Script = Thai.
  constexpr UnicodeRange kScriptThai[] = {
      {0x0E01, 0x0E3A},
      {0x0E40, 0x0E5B},
  };

  // Script = Hangul.
  constexpr UnicodeRange kScriptHangul[] = {
      {0x1100, 0x11FF}, {0x302E, 0x302F}, {0x3131, 0x318E}, {0x3200, 0x321E},
      {0x3260, 0x327E}, {0xA960, 0xA97C}, {0xAC00, 0xD7A3}, {0xD7B0, 0xD7C6},
      {0xD7CB, 0xD7FB}, {0xFFA0, 0xFFBE}, {0xFFC2, 0xFFC7}, {0xFFCA, 0xFFCF},
      {0xFFD2, 0xFFD7}, {0xFFDA, 0xFFDC},
  };

  // Script = Hiragana or Katakana, or Script_Extensions = {Hiragana, Katakana}.
  constexpr UnicodeRange kKana[] = {
      {0x3031, 0x3035},   {0x3041, 0x3096},   {0x3099, 0x30FA},
      {0x30FC, 0x30FF},   {0x31F0, 0x31FF},   {0x32D0, 0x32FE},
      {0x3300, 0x3357},   {0xFF66, 0xFF9F},   {0x1AFF0, 0x1AFF3},
      {0x1AFF5, 0x1AFFB}, {0x1AFFD, 0x1AFFE}, {0x1B000, 0x1B122},
      {0x1B132, 0x1B132}, {0x1B150, 0x1B152}, {0x1B155, 0x1B155},
      {0x1B164, 0x1B167}, {0x1F200, 0x1F200},
  };

  // Script = Devanagari and Indic_Conjunct_Break = Consonant.
  constexpr UnicodeRange kDevanagariConsonant[] = {
      {0x0915, 0x0939},
      {0x0958, 0x095F},
      {0x0978, 0x097F},
  };

  // Block = Basic Latin.
  constexpr UnicodeRange kBlockBasicLatin[] = {
      {0x0000, 0x007F},
  };

  // Layout treats IDEOGRAPHIC SPACE as a word separator alongside the CSS
  // Text 3 word separators.
  // https://www.w3.org/TR/css-text-3/#word-separator
  constexpr UnicodeRange kWordSeparator[] = {
      {0x0020, 0x0020},   {0x00A0, 0x00A0},   {0x1361, 0x1361},
      {0x3000, 0x3000},   {0x10100, 0x10101}, {0x1039F, 0x1039F},
      {0x1091F, 0x1091F},
  };

  // Layout-specific punctuation roles for line breaking.
  constexpr UnicodeRange kLeftPunctuation[] = {
      {U'(', U'('},   {U'<', U'<'},   {U'[', U'['},
      {U'{', U'{'},   {U'‘', U'‘'},   {U'“', U'“'},
      {U'《', U'《'}, {U'【', U'【'}, {U'（', U'（'},
  };
  constexpr UnicodeRange kRightPunctuation[] = {
      {U'!', U'"'},   {U'%', U'%'},   {U'\'', U'\''}, {U')', U')'},
      {U',', U','},   {U'.', U'.'},   {U':', U';'},   {U'>', U'?'},
      {U']', U']'},   {U'`', U'`'},   {U'}', U'~'},   {U'’', U'’'},
      {U'”', U'”'},   {U'…', U'…'},   {U'、', U'。'}, {U'》', U'》'},
      {U'】', U'】'}, {U'！', U'！'}, {U'）', U'）'}, {U'，', U'，'},
      {U'：', U'；'}, {U'？', U'？'}, {U'～', U'～'},
  };
  constexpr UnicodeRange kNeutralPunctuation[] = {
      {U'#', U'#'}, {U'&', U'&'}, {U'*', U'+'},   {U'-', U'-'},   {U'/', U'/'},
      {U'=', U'='}, {U'@', U'@'}, {U'\\', U'\\'}, {U'^', U'_'},   {U'|', U'|'},
      {U'·', U'·'}, {U'–', U'–'}, {U'゠', U'゠'}, {U'・', U'・'},
  };

  using P = UnicodeProperties;
  UnicodePropertyTableBuilder<BoundaryCapacity(
      kBlockBasicLatin, kLeftPunctuation, kRightPunctuation,
      kNeutralPunctuation, kScriptThai, kCJKCompatibilityIdeograph,
      kCJKSymbolsAndFullwidthForms, kDevanagariConsonant, kVariationSelector,
      kScriptDevanagari, kExtenderOnly, kWordSeparator, kTrimmableSpace,
      kMarkOnly, kScriptHangul, kUnifiedIdeograph, kKana, kCJKPunctuation,
      kScriptArabic, kEmojiPresentation, kEmojiTextDefault, kMarkAndExtender)>
      table;
  table.AddRanges(kBlockBasicLatin, P::kBlockBasicLatin);
  table.AddRanges(kLeftPunctuation, P::kLeftPunctuation);
  table.AddRanges(kRightPunctuation, P::kRightPunctuation);
  table.AddRanges(kNeutralPunctuation, P::kNeutralPunctuation);
  table.AddRanges(kScriptThai, P::kScriptThai);
  table.AddRanges(kCJKCompatibilityIdeograph, P::kCJKCompatibilityIdeograph);
  table.AddRanges(kCJKSymbolsAndFullwidthForms,
                  P::kCJKSymbolsAndFullwidthForms);
  table.AddRanges(kDevanagariConsonant, P::kDevanagariConsonant);
  table.AddRanges(kVariationSelector, P::kVariationSelector);
  table.AddRanges(kScriptDevanagari, P::kScriptDevanagari);
  table.AddRanges(kExtenderOnly, P::kGraphemeExtender);
  table.AddRanges(kWordSeparator, P::kWordSeparator);
  table.AddRanges(kTrimmableSpace, P::kTrimmableSpace);
  table.AddRanges(kMarkOnly, P::kGeneralCategoryMark);
  table.AddRanges(kScriptHangul, P::kScriptHangul);
  table.AddRanges(kUnifiedIdeograph, P::kUnifiedIdeograph);
  table.AddRanges(kKana, P::kKana);
  table.AddRanges(kCJKPunctuation, P::kCJKPunctuation);
  table.AddRanges(kScriptArabic, P::kScriptArabic);
  table.AddRanges(kEmojiPresentation, P::kEmoji | P::kEmojiPresentation);
  table.AddRanges(kEmojiTextDefault, P::kEmoji);
  table.AddRanges(kMarkAndExtender,
                  P::kGeneralCategoryMark | P::kGraphemeExtender);
  return table;
}

// The first two planes contain most boundaries. Store their starts as 10-bit
// page offsets and keep full code points for the sparse higher planes.
constexpr uint32_t kPageBits = 10;
constexpr uint32_t kPageSize = 1u << kPageBits;
constexpr uint32_t kPagedLimit = 0x20000;

template <size_t Count>
struct IndexedBoundaries {
  std::array<uint32_t, Count> boundaries{};
  std::array<uint8_t, Count> property_indices{};
  std::array<uint32_t, Count> properties{};
  size_t property_count = 0;
  size_t paged_count = 0;
};

constexpr auto IndexBoundaries() {
  constexpr auto table = BuildUnicodePropertyTable();
  IndexedBoundaries<table.size> result;
  for (size_t i = 0; i < table.size; ++i) {
    const auto boundary = table.entries[i];
    size_t index = 0;
    while (index < result.property_count &&
           result.properties[index] != boundary.properties) {
      ++index;
    }
    if (index == result.property_count) {
      result.properties[result.property_count++] = boundary.properties;
    }
    result.boundaries[i] = boundary.start;
    result.property_indices[i] = static_cast<uint8_t>(index);
    if (boundary.start < kPagedLimit) ++result.paged_count;
  }
  return result;
}

template <size_t Count, size_t PagedCount, size_t PropertyCount>
struct UnicodePropertyTable {
  std::array<uint32_t, PropertyCount> properties;
  std::array<uint8_t, 128> ascii;
  std::array<uint16_t, kPagedLimit / kPageSize + 1> page_offsets;
  std::array<uint16_t, PagedCount> paged;
  std::array<uint32_t, Count - PagedCount> high;
  // One property index per boundary, in the combined order of paged and high.
  std::array<uint8_t, Count> property_indices;
};

constexpr auto kTable = [] {
  constexpr auto indexed = IndexBoundaries();
  static_assert(indexed.property_count <= UINT8_MAX + 1);
  static_assert(indexed.paged_count <= UINT16_MAX);
  static_assert(indexed.boundaries.front() == 0);
  UnicodePropertyTable<indexed.boundaries.size(), indexed.paged_count,
                       indexed.property_count>
      table{};
  table.property_indices = indexed.property_indices;
  for (size_t i = 0; i < table.properties.size(); ++i) {
    table.properties[i] = indexed.properties[i];
  }
  size_t boundary = 0;
  for (size_t ch = 0; ch < table.ascii.size(); ++ch) {
    while (boundary + 1 < indexed.paged_count &&
           indexed.boundaries[boundary + 1] <= ch) {
      ++boundary;
    }
    table.ascii[ch] = indexed.property_indices[boundary];
  }
  boundary = 0;
  for (size_t page = 0; page < table.page_offsets.size(); ++page) {
    table.page_offsets[page] = boundary;
    while (boundary < table.paged.size() &&
           (indexed.boundaries[boundary] >> kPageBits) == page) {
      table.paged[boundary] =
          static_cast<uint16_t>(indexed.boundaries[boundary] & (kPageSize - 1));
      ++boundary;
    }
  }
  for (size_t j = 0; j < table.high.size(); ++j) {
    table.high[j] = indexed.boundaries[indexed.paged_count + j];
  }
  return table;
}();

}  // namespace

bool UnicodeProperties::Has(uint32_t mask) const { return (bits & mask) != 0; }

bool UnicodeProperties::IsGraphemeExtender() const {
  return Has(kGraphemeExtender);
}

bool UnicodeProperties::IsCJKCompatibilityIdeograph() const {
  return Has(kCJKCompatibilityIdeograph);
}

bool UnicodeProperties::IsCJKIdeograph() const {
  return Has(kUnifiedIdeograph | kCJKCompatibilityIdeograph);
}

bool UnicodeProperties::IsCJKSymbolsAndFullwidthForms() const {
  return Has(kCJKSymbolsAndFullwidthForms);
}

bool UnicodeProperties::IsKana() const { return Has(kKana); }

bool UnicodeProperties::IsCJK() const {
  return Has(kUnifiedIdeograph | kCJKCompatibilityIdeograph | kKana |
             kScriptHangul);
}

bool UnicodeProperties::IsCJKPunctuation() const {
  return Has(kCJKPunctuation);
}

bool UnicodeProperties::IsSpaceChar() const { return Has(kTrimmableSpace); }

UnicodeProperties GetUnicodeProperties(uint32_t codepoint) {
  if (codepoint < kTable.ascii.size()) {
    return {kTable.properties[kTable.ascii[codepoint]]};
  }

  size_t next_boundary;
  if (codepoint < kPagedLimit) {
    const auto page = codepoint >> kPageBits;
    const auto* ranges = kTable.paged.data();
    const uint16_t offset = codepoint & (kPageSize - 1);
    const auto* end =
        std::upper_bound(ranges + kTable.page_offsets[page],
                         ranges + kTable.page_offsets[page + 1], offset);
    next_boundary = end - ranges;
  } else {
    const auto end =
        std::upper_bound(kTable.high.begin(), kTable.high.end(), codepoint);
    next_boundary = kTable.paged.size() + (end - kTable.high.begin());
  }
  // The preceding boundary also supplies the properties across page boundaries.
  return {kTable.properties[kTable.property_indices[next_boundary - 1]]};
}

UnicodeProperties GetUnicodePropertiesAt(const UnicodeProperties* cached,
                                         uint32_t index, uint32_t codepoint) {
  return cached ? cached[index] : GetUnicodeProperties(codepoint);
}

bool IsCombiningMark(uint32_t codepoint) {
  if (codepoint < 0x0300) {
    return false;
  }
  return GetUnicodeProperties(codepoint).Has(
      UnicodeProperties::kGeneralCategoryMark);
}

bool IsEmojiModifier(uint32_t codepoint) {
  return codepoint >= 0x1F3FB && codepoint <= 0x1F3FF;
}

bool IsEmojiRegionalIndicator(uint32_t codepoint) {
  return codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF;
}

bool IsEmojiKeycapBase(uint32_t codepoint) {
  return (codepoint >= 0x30 && codepoint <= 0x39) || codepoint == 0x23 ||
         codepoint == 0x2A;
}

bool IsCombiningEnclosingKeycap(uint32_t codepoint) {
  return codepoint == 0x20E3;
}

bool IsEmojiTagCharacter(uint32_t codepoint) {
  return codepoint >= 0xE0020 && codepoint <= 0xE007F;
}

bool IsZeroWidthJoiner(uint32_t codepoint) { return codepoint == 0x200D; }

bool IsZeroWidthNonJoiner(uint32_t codepoint) { return codepoint == 0x200C; }

bool IsEmojiCodepoint(uint32_t codepoint) {
  return GetUnicodeProperties(codepoint).Has(UnicodeProperties::kEmoji);
}

bool HasDefaultEmojiPresentation(uint32_t codepoint) {
  return GetUnicodeProperties(codepoint).Has(
      UnicodeProperties::kEmojiPresentation);
}

bool IsVariationSelector(uint32_t codepoint) {
  return GetUnicodeProperties(codepoint).Has(
      UnicodeProperties::kVariationSelector);
}

bool IsGraphemeExtender(uint32_t codepoint) {
  return codepoint >= 0x0300 &&
         GetUnicodeProperties(codepoint).IsGraphemeExtender();
}

bool IsDevanagari(uint32_t codepoint) {
  return GetUnicodeProperties(codepoint).Has(
      UnicodeProperties::kScriptDevanagari);
}

bool IsDevanagariVirama(uint32_t codepoint) { return codepoint == 0x094D; }

bool IsDevanagariConsonant(uint32_t codepoint) {
  return GetUnicodeProperties(codepoint).Has(
      UnicodeProperties::kDevanagariConsonant);
}

bool IsCJKUnifiedIdeographBMP(char32_t code) {
  return code <= 0xFFFF &&
         GetUnicodeProperties(code).Has(UnicodeProperties::kUnifiedIdeograph);
}

bool IsCJKCompatibilityIdeographBMP(char32_t code) {
  return code <= 0xFFFF &&
         GetUnicodeProperties(code).IsCJKCompatibilityIdeograph();
}

bool IsCJKSupplementaryIdeograph(char32_t code) {
  return code > 0xFFFF && GetUnicodeProperties(code).IsCJKIdeograph();
}

bool IsCJKSymbolsAndFullwidthForms(char32_t code) {
  return GetUnicodeProperties(code).IsCJKSymbolsAndFullwidthForms();
}

bool IsKana(char32_t code) { return GetUnicodeProperties(code).IsKana(); }

bool IsHangul(char32_t code) {
  return GetUnicodeProperties(code).Has(UnicodeProperties::kScriptHangul);
}

bool IsCJK(char32_t code) { return GetUnicodeProperties(code).IsCJK(); }

bool IsCJKPunctuation(char32_t code) {
  return GetUnicodeProperties(code).IsCJKPunctuation();
}

bool IsSpaceChar(char32_t code) {
  return GetUnicodeProperties(code).IsSpaceChar();
}

}  // namespace base
}  // namespace ttoffice
