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

  for (uint32_t i = 0; i < length;) {
    uint32_t utf32 = u16str[i];

    if (utf32 >= 0xD800 && utf32 <= 0xDBFF) {
      if (i + 1 < length) {
        uint32_t lowSurrogate = u16str[i + 1];
        if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
          utf32 = ((utf32 - 0xD800) << 10) + (lowSurrogate - 0xDC00) + 0x10000;
          i += 2;
        } else {
          LogUtil::E("Invalid UTF-16 encoding");
        }
      } else {
        LogUtil::E("Invalid UTF-16 encoding");
      }
    } else if (utf32 >= 0xDC00 && utf32 <= 0xDFFF) {
      LogUtil::E("Invalid UTF-16 encoding");
    } else {
      i++;
    }

    u32str.push_back(utf32);
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
}  // namespace base
}  // namespace ttoffice
