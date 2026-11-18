// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SHAPE_TEXT_ENCODING_H_
#define SRC_PORTS_SHAPER_CORETEXT_SHAPE_TEXT_ENCODING_H_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "src/textlayout/utils/log_util.h"
#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

// The input and scratch storage must outlive this object. CoreText indices are
// UTF-16 offsets; Textra indices count Unicode scalars. Build both
// representations in one pass, including the end sentinel needed by
// CTRunGetStringRange.
class EncodedShapeText {
 public:
  EncodedShapeText(std::u32string_view text, uint32_t* scratch,
                   size_t scratch_size)
      : text_(text), map_(scratch) {
    utf16_.reserve(text.size() * 2);
    size_t capacity = scratch_size;
    uint32_t scalar_index = 0;
    bool invalid_input = false;
    for (char32_t ch : text) {
      const bool bmp = ch <= 0xD7FF || (ch >= 0xE000 && ch <= 0xFFFF);
      if (!bmp && !(ch >= 0x10000 && ch <= 0x10FFFF)) {
        LogUtil::E("U32StringToU16 Invalid UTF-32 encoding");
        invalid_input = true;
        continue;
      }
      const size_t offset = utf16_.size();
      const size_t units = bmp ? 1 : 2;
      if (offset + units + 1 > capacity) {
        capacity = text.size() * 2 + 1;
        overflow_map_.reset(new uint32_t[capacity]);
        if (offset != 0) std::copy_n(map_, offset, overflow_map_.get());
        map_ = overflow_map_.get();
      }
      map_[offset] = scalar_index;
      if (bmp) {
        utf16_.push_back(static_cast<char16_t>(ch));
      } else {
        const auto value = ch - 0x10000;
        utf16_.push_back(static_cast<char16_t>(0xD800 | (value >> 10)));
        utf16_.push_back(static_cast<char16_t>(0xDC00 | (value & 0x3FF)));
        map_[offset + 1] = scalar_index;
      }
      ++scalar_index;
      all_ascii_ &= ch <= 0x7F;
    }
    if (capacity == 0) {
      overflow_map_.reset(new uint32_t[1]);
      map_ = overflow_map_.get();
    }
    map_[utf16_.size()] = scalar_index;
    if (invalid_input) {
      // Preserve the existing converter's skip-and-compact behavior. Only
      // malformed input needs a normalized copy for font classification.
      normalized_ = base::U16StringToU32(utf16_);
      text_ = normalized_;
    }
  }

  EncodedShapeText(const EncodedShapeText&) = delete;
  EncodedShapeText& operator=(const EncodedShapeText&) = delete;

  const std::u16string& Utf16() const { return utf16_; }
  std::u32string_view Text() const { return text_; }
  const uint32_t* Map() const { return map_; }
  bool IsAscii() const { return all_ascii_; }

 private:
  std::u16string utf16_;
  std::u32string normalized_;
  std::u32string_view text_;
  std::unique_ptr<uint32_t[]> overflow_map_;
  uint32_t* map_;
  bool all_ascii_ = true;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_CORETEXT_SHAPE_TEXT_ENCODING_H_
