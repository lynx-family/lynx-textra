// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/tt_string_piece.h"

#include <textra/macro.h>

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
TTStringPiece::TTStringPiece(const TTString* tt_string, uint32_t char_offset,
                             uint32_t char_count)
    : tt_string_(tt_string),
      char_offset_(char_offset),
      char_count_(char_count) {
#ifdef TTTEXT_DEBUG
  UpdateContent();
#endif
}
std::string TTStringPiece::ToString() const {
  auto u8_start = tt_string_->CharPosToUtf8Pos(char_offset_);
  auto u8_end = tt_string_->CharPosToUtf8Pos(char_offset_ + char_count_);
  return std::string(tt_string_->Data() + u8_start, u8_end - u8_start);
}
std::u32string TTStringPiece::ToUTF32() const {
  auto* start = tt_string_->Data() + CharPosToUtf8Pos(0);
  auto* end = tt_string_->Data() + CharPosToUtf8Pos(GetCharCount());
  return base::U8StringToU32(std::string(start, end - start));
}
#ifdef TTTEXT_DEBUG
void TTStringPiece::UpdateContent() {
  auto byte_start = tt_string_->CharPosToUtf8Pos(char_offset_);
  auto byte_end = tt_string_->CharPosToUtf8Pos(char_offset_ + char_count_);
  debug_content_ =
      std::string(tt_string_->Data() + byte_start, byte_end - byte_start);
}
#endif
}  // namespace tttext
}  // namespace ttoffice
