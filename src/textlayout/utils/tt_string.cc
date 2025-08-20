// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/tt_string.h"

#include <textra/macro.h>

#include "src/textlayout/utils/tt_string_piece.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
TTStringPiece TTString::ToPiece() const { return SubStr(0, GetCharCount()); }
uint32_t TTString::GetBytesLengthBetweenChar(uint32_t char_start,
                                             uint32_t char_end) const {
  auto u8_start = CharPosToUtf8Pos(char_start);
  auto u8_end = CharPosToUtf8Pos(char_end);
  return u8_end - u8_start;
}
void TTString::Clear() {
  string_ = "";
  char_to_utf8_vec_.clear();
  utf8_to_char_vec_.clear();
}
uint32_t TTString::GetUnicode(uint32_t char_idx) const {
  uint32_t char_len = 0;
  auto* p_u8 = string_.c_str() + CharPosToUtf8Pos(char_idx);
  auto cp = base::U8CharToU32(p_u8, &char_len);
  TTASSERT(char_len == base::Utf8CharBytes(p_u8));
  return cp;
}
uint32_t TTString::CharPosToUtf8Pos(uint32_t idx) const {
  TTASSERT(idx <= char_to_utf8_vec_.size());
  return idx >= char_to_utf8_vec_.size() ? Length() : char_to_utf8_vec_[idx];
}
uint32_t TTString::Utf8PosToCharPos(uint32_t idx) const {
  TTASSERT(idx <= utf8_to_char_vec_.size());
  return idx >= utf8_to_char_vec_.size() ? GetCharCount()
                                         : utf8_to_char_vec_[idx];
}
uint32_t TTString::GetBytesCountOfChar(uint32_t idx) const {
  auto char_len = base::Utf8CharBytes(string_.c_str() + CharPosToUtf8Pos(idx));
  TTASSERT(char_len > 0 && char_len < 5);
  return char_len;
}
void TTString::AppendString(const std::string& string) {
  auto length = string.length();
  if (length == 0) return;
  utf8_to_char_vec_.reserve(utf8_to_char_vec_.size() + length);
  char_to_utf8_vec_.reserve(char_to_utf8_vec_.size() + length);
  auto current_char_pos = this->GetCharCount();
  for (uint32_t idx = 0; idx < string.length(); idx++) {
    if (base::IsUtf8CharStart(string.c_str() + idx)) {
      char_to_utf8_vec_.push_back(idx + Length());
      current_char_pos++;
    }
    utf8_to_char_vec_.emplace_back(current_char_pos - 1);
  }
  char_to_utf8_vec_.shrink_to_fit();
  string_ += string;
}
std::u32string TTString::ToUTF32() const {
  const auto* start = Data() + CharPosToUtf8Pos(0);
  const auto* end = Data() + CharPosToUtf8Pos(GetCharCount());
  return base::U8StringToU32(std::string(start, end - start));
}
TTStringPiece TTString::SubStr(uint32_t char_offset,
                               uint32_t char_length) const {
  TTASSERT(char_offset + char_length <= GetCharCount());
  return TTStringPiece{this, char_offset, char_length};
}
}  // namespace tttext
}  // namespace ttoffice
