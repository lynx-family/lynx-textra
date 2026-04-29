// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/justify_analyst.h"

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

JustifyAnalyst::JustifyAnalyst(const TTString& content) {
  const auto char_count = content.GetCharCount();
  justify_opportunity_.resize(char_count, false);
  for (auto k = 0u; k + 1 < char_count; ++k) {
    const auto ch = static_cast<char32_t>(content.GetUnicode(k));
    const auto next_ch = static_cast<char32_t>(content.GetUnicode(k + 1));
    justify_opportunity_[k] = CanInsertJustifySpaceAfterChars(ch, next_ch);
  }
}

CharPos JustifyAnalyst::FindNextJustifyOpportunity(CharPos start_char) const {
  for (auto k = start_char; k < justify_opportunity_.size(); ++k) {
    if (justify_opportunity_[k]) {
      return k + 1;
    }
  }
  return static_cast<CharPos>(justify_opportunity_.size());
}

bool JustifyAnalyst::CanInsertJustifySpaceAfter(CharPos char_pos) const {
  return char_pos < justify_opportunity_.size() &&
         justify_opportunity_[char_pos];
}

bool JustifyAnalyst::CanInsertJustifySpaceAfterChars(char32_t ch,
                                                     char32_t next_ch) {
  const bool ch_is_word_separator = base::IsWordSeparator(ch);
  const bool next_is_word_separator = base::IsWordSeparator(next_ch);
  if (ch_is_word_separator && !next_is_word_separator) {
    return true;
  }
  if (base::IsCJK(ch) || base::IsCJK(next_ch) || base::IsCJKPunctuation(ch) ||
      base::IsCJKPunctuation(next_ch)) {
    return true;
  }
  return false;
}

}  // namespace tttext
}  // namespace ttoffice
