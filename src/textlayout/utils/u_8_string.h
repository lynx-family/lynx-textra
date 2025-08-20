// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_U_8_STRING_H_
#define SRC_TEXTLAYOUT_UTILS_U_8_STRING_H_

#include <cassert>
#include <string>

namespace ttoffice {
namespace base {
#define CHAR char
static constexpr const CHAR* CR_STR() { return "\r"; }
static constexpr const CHAR* LF_STR() { return "\n"; }
static constexpr const CHAR* CRLF_STR() { return "\r\n"; }
static constexpr const CHAR* FF_STR() { return "\f"; }
static constexpr const CHAR* NLF_STR() { return "\u2028"; }
static constexpr const CHAR* TAB_STR() { return "\t"; }
static constexpr const CHAR* EMPTY_STR() { return ""; }
static constexpr const CHAR* SPACE_STR() { return " "; }     // Space
static constexpr const CHAR* ZWS_STR() { return "\u200b"; }  // Zero Width Space
static constexpr const CHAR* NBS_STR() { return "\u00a0"; }  // No Break Space
static constexpr char16_t kReplacementU16() { return u'\xFFFD'; }
static constexpr const char32_t* NeutralPunctuation() {
  return U"-_–+=/*@#·^&|\\";
}
static constexpr const char32_t* LeftPunctuation() { return U"([{<"; }
static constexpr const char32_t* RightPunctuation() {
  return U"],.;!?:\"')>}%~`";
}
static constexpr const char32_t* CJKLeftPunctuation() { return U"“（《[【‘"; }
static constexpr const char32_t* CJKRightPunctuation() {
  return U"）】”’》？、，。！：；……～";
}

inline uint32_t Utf8CharBytes(const char* c) {
  /**
   0x00000000 - 0x0000007F:
       0xxxxxxx

  0x00000080 - 0x000007FF:
      110xxxxx 10xxxxxx

      0x00000800 - 0x0000FFFF:
      1110xxxx 10xxxxxx 10xxxxxx

      0x00010000 - 0x001FFFFF:
          11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
              */
  uint8_t cc = *c;
  if ((cc & 0x80u) == 0u) return 1;
  if ((cc & 0xe0u) == 0xc0u) return 2;
  if ((cc & 0xf0u) == 0xe0u) return 3;
  if ((cc & 0xf8u) == 0xf0u) return 4;
  return 0;
}
inline bool IsUtf8CharStart(const char* s) {
  return ((uint8_t)*s & 0xC0u) != 0x80u;
}
inline bool IsUtf8Char10x(const char* s) {
  uint8_t cc = *s;
  int bit1 = cc >> 7 & 1u;
  int bit2 = cc >> 6 & 1u;
  return (bit1 == 1) && (bit2 == 0);
}
inline bool CheckValidUTF8String(const char* str) {
  const auto* p_char = str;
  while (*p_char != '\0') {
    auto count = Utf8CharBytes(p_char);
    if (count == 0) return false;
    for (auto k = 1u; k < count; k++) {
      p_char++;
      if (!IsUtf8Char10x(p_char)) return false;
    }
    p_char++;
  }
  return true;
}
inline bool CheckIsLineBreakChar(const char* s) {
  return *s == '\n' || *s == '\r';
}
inline int CalcCharCount(const char* s, int len) {
  int count = 0;
  int idx = 0;
  while (idx < len) {
    if (IsUtf8CharStart(s + idx++)) count++;
  }
  return count;
}
[[maybe_unused]] static int CharPosToByte(const char* s, int len,
                                          int char_pos) {
  int char_count = 0;
  int offset = 0;
  for (; offset < len && char_count <= char_pos; offset++) {
    if (IsUtf8CharStart(s + offset)) char_count++;
  }
  return offset - 1;
}
[[maybe_unused]] static int ByteToCharPos(const char* s, int len,
                                          int byte_pos) {
  assert(byte_pos <= len && IsUtf8CharStart(s + byte_pos));
  int char_pos = 0;
  for (int idx = 0; idx < byte_pos; idx++) {
    if (IsUtf8CharStart(s + idx)) char_pos++;
  }
  return char_pos;
}
uint32_t U8CharToU32(const char* u8_char, uint32_t* char_len);
inline uint32_t U32Strlen(const char32_t* str) {
  const auto* s = str;
  while (*s) ++s;
  return static_cast<uint32_t>(s - str);
}

inline bool IsInUtf16BMP(char16_t c) { return (c & 0xF800U) != 0xD800; }
inline bool IsLeadSurrogate(uint16_t c) { return ((c) & 0xfffffc00) == 0xd800; }
inline bool IsTrailSurrogate(uint16_t c) {
  return ((c) & 0xfffffc00) == 0xdc00;
}
inline bool IsSurrogate(uint16_t c) { return ((c) & 0xfffff800) == 0xd800; }
inline bool StringEqual(const char* str1, uint32_t len1, const char* str2,
                        uint32_t len2) {
  return len1 == len2 && (strncmp(str1, str2, len1) == 0);
}
inline bool StringEqual(const char* str1, const char* str2) {
  return StringEqual(str1, static_cast<uint32_t>(strlen(str1)), str2,
                     static_cast<uint32_t>(strlen(str2)));
}
inline bool StringEqual(const char* str1, uint32_t len1, const char* str2) {
  return StringEqual(str1, len1, str2, static_cast<uint32_t>(strlen(str2)));
}
std::u32string U8StringToU32(const char* u8str, uint32_t length);
std::u32string U16StringToU32(const char16_t* u16str, uint32_t length);
std::u16string U32StringToU16(const char32_t* u32str, uint32_t length);
std::string U32StringToU8(const char32_t* u32str, uint32_t length);
inline std::u16string U32StringToU16(const std::u32string& u32str) {
  return U32StringToU16(u32str.data(), static_cast<uint32_t>(u32str.length()));
}
inline std::string U32StringToU8(const std::u32string& u32str) {
  return U32StringToU8(u32str.data(), static_cast<uint32_t>(u32str.length()));
}
inline std::u16string U8StringToU16(const char* u8str, uint32_t length) {
  return U32StringToU16(U8StringToU32(u8str, length));
}
inline std::string U16StringToU8(const char16_t* u16str, uint32_t length) {
  return U32StringToU8(U16StringToU32(u16str, length));
}
inline std::u16string U8StringToU16(const std::string& u8str) {
  return U8StringToU16(u8str.data(), static_cast<uint32_t>(u8str.length()));
}
inline std::u32string U8StringToU32(const std::string& u8str) {
  return U8StringToU32(u8str.data(), static_cast<uint32_t>(u8str.length()));
}
inline std::string U16StringToU8(const std::u16string& u16str) {
  return U16StringToU8(u16str.data(), static_cast<uint32_t>(u16str.length()));
}
inline std::u32string U16StringToU32(const std::u16string& u16str) {
  return U16StringToU32(u16str.data(), static_cast<uint32_t>(u16str.length()));
}
uint32_t U32IndexToU16(const char32_t* content, uint32_t length,
                       uint32_t index);
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
 * 48 code points, actually 36 characters CJK Compatibility [F900-FAFF]   512
 * code points, actually 477 characters PUA (GBK)                    [E815-E86F]
 * 90 code points, actually 80 characters PUA Component Extension [E400-E5FF]
 * 511 code points, actually 452 characters PUA Character Supplement [E600-E6BF]
 * 191 code points, actually 185 characters Hiragana & Katakana [3040-30ff]
 */
inline bool IsCJK(char32_t code) {
  return (code >= 0x4e00 && code <= 0x9FFF) ||
         (code >= 0x3400 && code <= 0x4dbf) ||
         (code >= 0x3040 && code <= 0x30ff) ||
         (code >= 0x20000 && code <= 0x2ffff);
}
inline bool IsSpaceChar(char32_t code) {
  return code == ' ' || code == '\r' || code == '\n' || code == '\t';
}
inline bool IsNoneVisibleASCII(char32_t code) { return code < 32; }
}  // namespace base
}  // namespace ttoffice
// using U8String = ttoffice::U8String;
#endif  // SRC_TEXTLAYOUT_UTILS_U_8_STRING_H_
