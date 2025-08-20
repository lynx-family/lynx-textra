// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_STRING_H_
#define SRC_TEXTLAYOUT_UTILS_TT_STRING_H_

#include <textra/macro.h>

#include <cstdint>
#include <string>
#include <vector>

#include "src/textlayout/utils/u_8_string.h"
namespace ttoffice {
namespace tttext {
using CharPos = uint32_t;
class TTStringPiece;
class L_EXPORT TTStringInterface {
 public:
  TTStringInterface() = default;
  virtual ~TTStringInterface() = default;

 public:
  virtual bool Empty() const = 0;
  virtual std::string ToString() const = 0;
  virtual const char* Data() const = 0;
  virtual uint32_t Length() const = 0;
  virtual uint32_t GetCharCount() const = 0;
  virtual uint32_t GetUnicode(uint32_t char_idx) const = 0;
  virtual uint32_t CharPosToUtf8Pos(uint32_t idx) const = 0;
  virtual uint32_t Utf8PosToCharPos(uint32_t idx) const = 0;
  virtual uint32_t GetBytesCountOfChar(uint32_t idx) const = 0;
  virtual TTStringPiece SubStr(uint32_t char_offset,
                               uint32_t char_length) const = 0;
  virtual char operator[](uint32_t bytes) const = 0;
};
class L_EXPORT TTString final : public TTStringInterface {
  friend TTStringPiece;

 public:
  TTString() = default;
  TTString(const TTString& tt_string) : TTString(tt_string.string_) {}
  L_EXPORT explicit TTString(const char* data)
      : TTString({data, strlen(data)}) {}
  explicit TTString(const char* data, uint32_t length)
      : TTString({data, length}) {}
  explicit TTString(const std::string& str) { AppendString(str); }
  explicit TTString(const char32_t* data, uint32_t length)
      : TTString(base::U32StringToU8(data, length)) {}
  L_EXPORT ~TTString() override = default;

 public:
  TTStringPiece ToPiece() const;
  const std::string& ToStringRef() const { return string_; }
  uint32_t GetBytesLengthBetweenChar(uint32_t char_start,
                                     uint32_t char_end) const;
  void Clear();

 public:
  bool Empty() const override { return string_.empty(); }
  std::string ToString() const override { return string_; }
  const char* Data() const override { return string_.c_str(); }
  uint32_t Length() const override {
    return static_cast<uint32_t>(string_.length());
  }
  uint32_t GetCharCount() const override {
    return static_cast<uint32_t>(char_to_utf8_vec_.size());
  }
  uint32_t GetUnicode(uint32_t char_idx) const override;
  uint32_t CharPosToUtf8Pos(uint32_t idx) const override;
  uint32_t Utf8PosToCharPos(uint32_t idx) const override;
  uint32_t GetBytesCountOfChar(uint32_t idx) const override;
  TTStringPiece SubStr(uint32_t char_offset,
                       uint32_t char_length) const override;
  char operator[](uint32_t bytes) const override { return string_[bytes]; }

 public:
  TTString operator+(const std::string& str) { return TTString{string_ + str}; }
  TTString operator+(const TTString& tt_string) {
    return TTString{string_ + tt_string.string_};
  }
  TTString& operator+=(const std::string& str) {
    AppendString(str);
    return *this;
  }
  TTString& operator+=(const TTString& tt_string) {
    AppendString(tt_string.string_);
    return *this;
  }
  void operator=(const std::string& str) {
    Clear();
    AppendString(str);
  }
  void operator=(char* str) {
    Clear();
    AppendString(str);
  }
  bool operator==(const TTString& other) const {
    return string_ == other.string_;
  }
  void AppendString(const std::string& string);
  std::u32string ToUTF32() const;

 private:
  std::string string_;
  std::vector<uint32_t> utf8_to_char_vec_;
  std::vector<uint32_t> char_to_utf8_vec_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_TT_STRING_H_
