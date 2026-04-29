// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_INTERNAL_JUSTIFY_ANALYST_H_
#define SRC_TEXTLAYOUT_INTERNAL_JUSTIFY_ANALYST_H_

#include <vector>

#include "src/textlayout/utils/tt_string.h"

namespace ttoffice {
namespace tttext {
class JustifyAnalyst {
 public:
  JustifyAnalyst() = delete;
  explicit JustifyAnalyst(const TTString& content);

 public:
  CharPos FindNextJustifyOpportunity(CharPos start_char) const;
  bool CanInsertJustifySpaceAfter(CharPos char_pos) const;

 private:
  static bool CanInsertJustifySpaceAfterChars(char32_t ch, char32_t next_ch);

  std::vector<char> justify_opportunity_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_INTERNAL_JUSTIFY_ANALYST_H_
