// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/justify_analyst.h"

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {

JustifyAnalyst::JustifyAnalyst(
    const std::vector<base::UnicodeProperties>& properties) {
  const auto char_count = properties.size();
  justify_opportunity_.resize(char_count, false);
  for (CharPos k = 0; k + 1 < char_count; ++k) {
    const auto current = properties[k];
    const auto next = properties[k + 1];
    const base::UnicodeProperties pair{current.bits | next.bits};
    justify_opportunity_[k] =
        pair.IsCJK() || pair.IsCJKPunctuation() ||
        (current.Has(base::UnicodeProperties::kWordSeparator) &&
         !next.Has(base::UnicodeProperties::kWordSeparator));
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

}  // namespace tttext
}  // namespace ttoffice
