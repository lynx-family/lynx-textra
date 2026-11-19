// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/boundary_analyst.h"

#include <textra/macro.h>

#include <algorithm>

#include "src/textlayout/style/style_manager.h"
#ifdef BOUNDARY_ANALYST_ICU
#include <textra/icu_wrapper.h>
#endif
namespace ttoffice {
namespace tttext {

using base::UnicodeProperties;

class SimpleBreak {
 public:
  static void SimpleBoundaryBreak(const char32_t* u32_content,
                                  const UnicodeProperties* properties,
                                  uint32_t char_count, BoundaryType* boundary,
                                  LineBreakStrategy line_break_strategy) {
    for (auto k = 0u; k < char_count; k++) {
      const auto current = properties[k];
      const bool is_word_character = IsWordCharacter(u32_content[k], current);
      const bool is_cjk = is_word_character && current.IsCJK();
      if (is_word_character && !is_cjk) {
        if (k < char_count - 1 &&
            IsOrdinaryText(u32_content[k + 1], properties[k + 1])) {
          // avoid break between normal character
          boundary[k] = BoundaryType::kGraphme;
        } else {
          // word
          boundary[k] = BoundaryType::kWord;
        }
      } else if (is_cjk) {
        boundary[k] = BoundaryType::kLineBreakable;
        if (k > 0 && IsOrdinaryText(u32_content[k - 1], properties[k - 1])) {
          boundary[k - 1] = BoundaryType::kLineBreakable;
        }
      } else if (IsBoundarySpace(u32_content[k], current)) {
        boundary[k] = BoundaryType::kLineBreakable;
        if (k > 0 && boundary[k - 1] == BoundaryType::kLineBreakable &&
            !IsBoundarySpace(u32_content[k - 1], properties[k - 1])) {
          boundary[k - 1] = BoundaryType::kWord;
        }
      }
      if (line_break_strategy & kAvoidBreakAroundPunctuation) {
        if (current.bits & UnicodeProperties::kNeutralPunctuation) {
          boundary[k] = BoundaryType::kLineBreakable;
        } else if (current.bits & UnicodeProperties::kLeftPunctuation) {
          // avoid break after left punctuation
          boundary[k] = BoundaryType::kWord;
        } else if (current.bits & UnicodeProperties::kRightPunctuation) {
          if (k > 0 &&
              !IsBoundarySpace(u32_content[k - 1], properties[k - 1])) {
            // avoid break before right punctuation
            boundary[k - 1] = BoundaryType::kWord;
          }
          boundary[k] = BoundaryType::kLineBreakable;
        }
      } else {
        boundary[k] = BoundaryType::kLineBreakable;
      }
    }
  }

  static void ApplyWordBreak(const char32_t* u32_content,
                             const UnicodeProperties* properties,
                             uint32_t char_count, BoundaryType* boundary,
                             const StyleManager& style_manager) {
    bool is_word_character = IsWordCharacter(u32_content[0], properties[0]);
    for (auto k = 0u; k + 1 < char_count; ++k) {
      const bool next_is_word_character =
          IsWordCharacter(u32_content[k + 1], properties[k + 1]);
      if (boundary[k] == BoundaryType::kLineBreakable && is_word_character &&
          next_is_word_character &&
          style_manager.GetWordBreak(k) == WordBreakType::kKeepAll) {
        boundary[k] = BoundaryType::kWord;
      }
      is_word_character = next_is_word_character;
    }
  }

 private:
  static constexpr uint32_t kPunctuation =
      UnicodeProperties::kLeftPunctuation |
      UnicodeProperties::kRightPunctuation |
      UnicodeProperties::kNeutralPunctuation;

  static bool IsBoundarySpace(char32_t ch, UnicodeProperties properties) {
    return ch == U'\0' ||
           (properties.bits & UnicodeProperties::kTrimmableSpace);
  }

  static bool IsWordCharacter(char32_t ch, UnicodeProperties properties) {
    return !IsBoundarySpace(ch, properties) &&
           !(properties.bits & kPunctuation);
  }

  static bool IsOrdinaryText(char32_t ch, UnicodeProperties properties) {
    return IsWordCharacter(ch, properties) && !properties.IsCJK();
  }
};

BoundaryAnalyst::BoundaryAnalyst(const char32_t* u32_content,
                                 uint32_t char_count,
                                 const base::UnicodeProperties* properties,
                                 LineBreakStrategy line_break_strategy,
                                 const StyleManager& style_manager) {
  // runs are left-closed right-open intervals, the last charpos of text is \0
  // character
  if (u32_content == nullptr || char_count == 0) {
    boundary_.assign(char_count, BoundaryType::kLineBreakable);
    return;
  }
  boundary_.assign(char_count, BoundaryType::kNone);
#ifdef BOUNDARY_ANALYST_ICU
  auto& icu_wrapper = ICUWrapper::GetInstance();
  icu_wrapper.icu_boundary_breaker(u32_content, char_count, boundary_);
#else
  SimpleBreak::SimpleBoundaryBreak(u32_content, properties, char_count,
                                   boundary_.data(), line_break_strategy);
#endif
  SimpleBreak::ApplyWordBreak(u32_content, properties, char_count,
                              boundary_.data(), style_manager);
}
uint32_t BoundaryAnalyst::FindNextBoundary(uint32_t start,
                                           BoundaryType type) const {
  for (auto k = start; k < boundary_.size(); ++k) {
    if (GetBoundaryType(k) >= type) return k + 1;
  }
  return static_cast<uint32_t>(boundary_.size());
}
uint32_t BoundaryAnalyst::FindPrevBoundary(uint32_t start,
                                           BoundaryType type) const {
  start = std::min(start, static_cast<uint32_t>(boundary_.size()));
  for (auto k = start; k > 0; --k) {
    if (GetBoundaryTypeBefore(k) >= type) return k;
  }
  return 0;
}
void BoundaryAnalyst::UpgradeBoundaryType(const Range& range,
                                          BoundaryType type) {
  TTASSERT(range.GetEnd() <= boundary_.size());
  for (auto k = range.GetStart(); k < range.GetEnd() && k < boundary_.size();
       k++) {
    if (boundary_[k] < type) boundary_[k] = type;
  }
}
}  // namespace tttext
}  // namespace ttoffice
