// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/internal/boundary_analyst.h"

#include <textra/macro.h>

#include <memory>
#include <string>
#ifdef BOUNDARY_ANALYST_ICU
#include <textra/icu_wrapper.h>
#endif
namespace ttoffice {
namespace tttext {

#ifndef BOUNDARY_ANALYST_ICU
enum class CharType {
  kNone,
  kWhiteSpace,
  kCJK,
  kLeftPunctuation,
  kNeutralPunctuation,
  kRightPunctuation
};
class SimpleBreak {
 public:
  static void SimpleBoundaryBreak(const char32_t* u32_content,
                                  uint32_t char_count, BoundaryType* boundary,
                                  LineBreakStrategy line_break_strategy) {
    auto char_type = std::make_unique<CharType[]>(char_count);
    for (auto k = 0u; k < char_count; k++) {
      char_type[k] = GetCharType(u32_content[k]);
    }
    for (auto k = 0u; k < char_count; k++) {
      const auto type = char_type[k];
      if (type == CharType::kNone) {
        if (k < char_count - 1 && char_type[k + 1] == CharType::kNone) {
          // avoid break between normal character
          boundary[k] = BoundaryType::kGraphme;
        } else {
          // word
          boundary[k] = BoundaryType::kWord;
        }
      } else if (type == CharType::kCJK) {
        boundary[k] = BoundaryType::kLineBreakable;
        if (k > 0 && char_type[k - 1] == CharType::kNone) {
          boundary[k - 1] = BoundaryType::kLineBreakable;
        }
      } else if (type == CharType::kWhiteSpace) {
        boundary[k] = BoundaryType::kLineBreakable;
        if (k > 0 && boundary[k - 1] == BoundaryType::kLineBreakable &&
            char_type[k - 1] != CharType::kWhiteSpace) {
          boundary[k - 1] = BoundaryType::kWord;
        }
      }
      if (line_break_strategy & kAvoidBreakAroundPunctuation) {
        if (type == CharType::kNeutralPunctuation) {
          boundary[k] = BoundaryType::kLineBreakable;
        } else if (type == CharType::kLeftPunctuation) {
          // avoid break after left punctuation
          boundary[k] = BoundaryType::kWord;
        } else if (type == CharType::kRightPunctuation) {
          if (k > 0 && char_type[k - 1] != CharType::kWhiteSpace) {
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

 private:
  static CharType GetCharType(char32_t ch32) {
    if (ch32 == 0) {
      return CharType::kWhiteSpace;
    }
    if (base::IsCJK(ch32)) {
      return CharType::kCJK;
    }
    if (base::IsSpaceChar(ch32)) {
      return CharType::kWhiteSpace;
    }
    if (StrFind(base::LeftPunctuation(), ch32) ||
        StrFind(base::CJKLeftPunctuation(), ch32)) {
      return CharType::kLeftPunctuation;
    }
    if (StrFind(base::NeutralPunctuation(), ch32)) {
      return CharType::kNeutralPunctuation;
    }
    if (StrFind(base::RightPunctuation(), ch32) ||
        StrFind(base::CJKRightPunctuation(), ch32)) {
      return CharType::kRightPunctuation;
    }
    return CharType::kNone;
  }
  static bool StrFind(const char32_t* string, char32_t ch32) {
    while (*string != 0) {
      if (*string == ch32) {
        return true;
      }
      string++;
    }
    return false;
  }
};
#endif

BoundaryAnalyst::BoundaryAnalyst(const char32_t* u32_content,
                                 uint32_t char_count,
                                 LineBreakStrategy line_break_strategy) {
  // runs are left-closed right-open intervals, the last charpos of text is \0
  // character
#ifdef BOUNDARY_ANALYST_ICU
  boundary_.resize(char_count, BoundaryType::kNone);
  auto& icu_wrapper = ICUWrapper::GetInstance();
  icu_wrapper.icu_boundary_breaker(u32_content, char_count, boundary_);
#else
  boundary_.resize(char_count);
  SimpleBreak::SimpleBoundaryBreak(u32_content, char_count, boundary_.data(),
                                   line_break_strategy);
#endif
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
