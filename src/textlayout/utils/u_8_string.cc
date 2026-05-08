// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/u_8_string.h"

#include <textra/macro.h>

#include <algorithm>

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

/*
 * CJK Basic                    [4E00-9FFF]   20992 code points, actually 20940
 * characters CJK Extension A              [3400-4DBF]   6592 code points,
 * actually 6582 characters CJK Extension B              [20000-2A6DF] 42720
 * code points, actually 42711 characters CJK Extension C [2A700-2B73F] 4159
 * code points, actually 4149 characters CJK Extension D [2B740-2B81F] 224 code
 * points, actually 222 characters CJK Compatibility Extension  [2F800-2FA1F]
 * 544 code points, actually 542 characters CJK Radical Extension [2E80-2EFF]
 * 128 code points, actually 115 characters CJK Kangxi Radicals [2F00-2FDF] 224
 * code points, actually 214 characters CJK Strokes                  [31C0-31EF]
 * 48 code points, actually 36 characters Hangul Jamo              [1100-11FF]
 * Hangul Compatibility Jamo [3130-318F] Hangul Syllables          [AC00-D7AF]
 * Hangul Jamo Extended       [A960-A97F, D7B0-D7FF]
 * CJK Compatibility          [F900-FAFF]   512
 * code points, actually 477 characters PUA (GBK)                    [E815-E86F]
 * 90 code points, actually 80 characters PUA Component Extension [E400-E5FF]
 * 511 code points, actually 452 characters PUA Character Supplement [E600-E6BF]
 * 191 code points, actually 185 characters Hiragana & Katakana [3040-30ff]
 */
bool IsCJK(char32_t code) {
  return (code >= 0x4e00 && code <= 0x9FFF) ||
         (code >= 0x3400 && code <= 0x4dbf) ||
         (code >= 0x3040 && code <= 0x30ff) ||
         (code >= 0x1100 && code <= 0x11ff) ||
         (code >= 0x3130 && code <= 0x318f) ||
         (code >= 0xa960 && code <= 0xa97f) ||
         (code >= 0xac00 && code <= 0xd7ff) ||
         (code >= 0x20000 && code <= 0x2ffff);
}

bool IsWordSeparator(char32_t code) {
  return code == 0x0020 || code == 0x00a0 || code == 0x1361 || code == 0x3000 ||
         code == 0x10100 || code == 0x10101 || code == 0x1039f ||
         code == 0x1091f;
}

bool IsCJKPunctuation(char32_t code) {
  if ((code >= 0x3001 && code <= 0x303f) ||
      (code >= 0xfe10 && code <= 0xfe1f) ||
      (code >= 0xfe30 && code <= 0xfe6f) ||
      (code >= 0xff01 && code <= 0xff0f) ||
      (code >= 0xff1a && code <= 0xff20) ||
      (code >= 0xff3b && code <= 0xff40) ||
      (code >= 0xff5b && code <= 0xff65)) {
    return true;
  }
  return false;
}

bool IsSpaceChar(char32_t code) {
  return code == ' ' || code == '\r' || code == '\n' || code == '\t' ||
         code == 0x3000;
}
}  // namespace base
}  // namespace ttoffice
