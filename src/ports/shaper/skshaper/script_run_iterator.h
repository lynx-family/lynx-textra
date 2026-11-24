// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_SKSHAPER_SCRIPT_RUN_ITERATOR_H_
#define SRC_PORTS_SHAPER_SKSHAPER_SCRIPT_RUN_ITERATOR_H_

#include "src/ports/shaper/skshaper/sk_shaper.h"
#include "src/textlayout/utils/unicode_script.h"

namespace ttoffice {
namespace tttext {

// The upstream SkShaper script-run algorithm, using Textra's approximate
// classifier and UTF-32 offsets instead of HarfBuzz properties and UTF-8 bytes.
class UnicodeScriptRunIterator final : public SkShaper::ScriptRunIterator {
 public:
  UnicodeScriptRunIterator(const char32_t* text, size_t length)
      : text_(text), length_(length) {}

  void consume() override {
    TTASSERT(!atEnd());
    script_ = GetUnicodeScript(text_[end_++]);
    while (end_ < length_) {
      const auto next = GetUnicodeScript(text_[end_]);
      // Hiragana and Katakana use the same OpenType 'kana' script. Keep the
      // leading script tag, but avoid splitting their shared shaping run.
      if (next != script_ && !(IsKana(script_) && IsKana(next))) {
        if (IsNeutral(script_)) {
          script_ = next;
        } else if (!IsNeutral(next)) {
          break;
        }
      }
      ++end_;
    }
    if (script_ == UnicodeScript::kInherited) script_ = UnicodeScript::kCommon;
  }
  size_t endOfCurrentRun() const override { return end_; }
  bool atEnd() const override { return end_ == length_; }
  SkFourByteTag currentScript() const override {
    switch (script_) {
      case UnicodeScript::kLatin:
        return Tag('L', 'a', 't', 'n');
      case UnicodeScript::kHan:
        return Tag('H', 'a', 'n', 'i');
      case UnicodeScript::kHiragana:
        return Tag('H', 'i', 'r', 'a');
      case UnicodeScript::kKatakana:
        return Tag('K', 'a', 'n', 'a');
      case UnicodeScript::kHangul:
        return Tag('H', 'a', 'n', 'g');
      case UnicodeScript::kArabic:
        return Tag('A', 'r', 'a', 'b');
      case UnicodeScript::kThai:
        return Tag('T', 'h', 'a', 'i');
      case UnicodeScript::kDevanagari:
        return Tag('D', 'e', 'v', 'a');
      case UnicodeScript::kBengali:
        return Tag('B', 'e', 'n', 'g');
      case UnicodeScript::kJavanese:
        return Tag('J', 'a', 'v', 'a');
      case UnicodeScript::kCommon:
      case UnicodeScript::kInherited:
        return Tag('Z', 'y', 'y', 'y');
      default:
        // The range classifier is incomplete. Let HarfBuzz guess scripts it
        // does not recognize instead of forcing HB_SCRIPT_UNKNOWN.
        return 0;
    }
  }

 private:
  static bool IsKana(UnicodeScript script) {
    return script == UnicodeScript::kHiragana ||
           script == UnicodeScript::kKatakana;
  }
  static bool IsNeutral(UnicodeScript script) {
    return script == UnicodeScript::kCommon ||
           script == UnicodeScript::kInherited;
  }
  static constexpr SkFourByteTag Tag(uint32_t a, uint32_t b, uint32_t c,
                                     uint32_t d) {
    return (a << 24) | (b << 16) | (c << 8) | d;
  }

  const char32_t* text_;
  size_t length_;
  size_t end_ = 0;
  UnicodeScript script_ = UnicodeScript::kUnknown;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_SKSHAPER_SCRIPT_RUN_ITERATOR_H_
