// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_DEFAULT_FONT_POLICY_H_
#define SRC_PORTS_SHAPER_CORETEXT_DEFAULT_FONT_POLICY_H_

#include <string_view>

#include "src/textlayout/utils/u_8_string.h"
#include "src/textlayout/utils/unicode_script.h"

namespace ttoffice {
namespace tttext {
namespace coretext {

// Script and emoji presentation are independent. Latin selects the system UI
// font; scripts without a dedicated font use CoreText's full cascade.
struct DefaultFontSelection {
  UnicodeScript script = UnicodeScript::kUnknown;
  bool emoji = false;

  bool operator==(const DefaultFontSelection& other) const {
    return script == other.script && emoji == other.emoji;
  }
  bool operator!=(const DefaultFontSelection& other) const {
    return !(*this == other);
  }
};

inline DefaultFontSelection SelectDefaultFont(std::u32string_view content) {
  const auto cluster = GetEmojiClusterInfo(
      content.data(), 0, static_cast<uint32_t>(content.size()));
  if (cluster.presentation == EmojiPresentation::kText) return {};
  if (cluster.presentation == EmojiPresentation::kEmoji) {
    return {UnicodeScript::kCommon, true};
  }
  auto script = UnicodeScript::kInherited;
  for (char32_t ch : content) {
    const auto current = GetUnicodeScript(ch);
    if (current == UnicodeScript::kInherited) continue;
    if (current == UnicodeScript::kCommon) {
      if (script == UnicodeScript::kInherited) script = current;
      continue;
    }
    if (script == UnicodeScript::kInherited ||
        script == UnicodeScript::kCommon) {
      script = current;
    } else if (script != current) {
      // Keep the entire mixed-script grapheme for CoreText to resolve.
      return {};
    }
  }
  return {script, false};
}

inline DefaultFontSelection ResolveDefaultFontContext(
    DefaultFontSelection selection, UnicodeScript context,
    std::u32string_view content) {
  if (!selection.emoji && (selection.script == UnicodeScript::kCommon ||
                           selection.script == UnicodeScript::kInherited)) {
    // CJK punctuation remains Common in Unicode. Choose its font here without
    // making it the script context for adjacent spaces or punctuation.
    selection.script =
        !content.empty() && base::IsCJKPunctuation(content.front())
            ? UnicodeScript::kHan
            : context;
  }
  return selection;
}

}  // namespace coretext
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_CORETEXT_DEFAULT_FONT_POLICY_H_
