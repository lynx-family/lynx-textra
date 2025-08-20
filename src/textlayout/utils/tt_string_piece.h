// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_STRING_PIECE_H_
#define SRC_TEXTLAYOUT_UTILS_TT_STRING_PIECE_H_

#include <textra/macro.h>

#include <string>
#include <vector>

#include "src/textlayout/utils/tt_string.h"
namespace ttoffice {
namespace tttext {
class L_EXPORT TTStringPiece final : public TTStringInterface {
 public:
  TTStringPiece() = delete;
  L_EXPORT TTStringPiece(const TTString* tt_string, uint32_t char_offset,
                         uint32_t char_count);
  L_EXPORT ~TTStringPiece() override = default;

 public:
  bool Empty() const override { return char_count_ == 0; }
  L_EXPORT std::string ToString() const override;
  const char* Data() const override {
    return tt_string_->Data() + tt_string_->CharPosToUtf8Pos(char_offset_);
  }
  uint32_t Length() const override {
    return tt_string_->GetBytesLengthBetweenChar(char_offset_,
                                                 char_offset_ + char_count_);
  }
  uint32_t GetCharCount() const override { return char_count_; }
  L_EXPORT uint32_t GetUnicode(uint32_t char_idx) const override {
    return tt_string_->GetUnicode(char_offset_ + char_idx);
  }
  uint32_t CharPosToUtf8Pos(uint32_t idx) const override {
    return tt_string_->CharPosToUtf8Pos(char_offset_ + idx);
  }
  uint32_t Utf8PosToCharPos(uint32_t idx) const override {
    auto base_utf8_pos = tt_string_->CharPosToUtf8Pos(char_offset_);
    return tt_string_->Utf8PosToCharPos(base_utf8_pos + idx);
  }
  uint32_t GetBytesCountOfChar(uint32_t idx) const override {
    return tt_string_->GetBytesCountOfChar(char_offset_ + idx);
  }
  TTStringPiece SubStr(uint32_t char_offset,
                       uint32_t char_length) const override {
    TTASSERT(char_length <= char_count_);
    return tt_string_->SubStr(char_offset_ + char_offset, char_length);
  }
  char operator[](uint32_t bytes) const override {
    TTASSERT(bytes < Length());
    return *(Data() + bytes);
  }

 public:
  uint32_t GetCharOffset() const { return char_offset_; }
  std::u32string ToUTF32() const;

 public:
  void operator=(const TTStringPiece& piece) {
    this->tt_string_ = piece.tt_string_;
    this->char_offset_ = piece.char_offset_;
    this->char_count_ = piece.char_count_;
  }

 private:
#ifdef TTTEXT_DEBUG
  void UpdateContent();
#endif

 private:
  const TTString* tt_string_;
  uint32_t char_offset_;
  uint32_t char_count_;
#ifdef TTTEXT_DEBUG
  std::string debug_content_;
#endif
};
};  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_TT_STRING_PIECE_H_
