// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_INTERNAL_BOUNDARY_ANALYST_H_
#define SRC_TEXTLAYOUT_INTERNAL_BOUNDARY_ANALYST_H_
#include <textra/layout_definition.h>
#include <textra/macro.h>

#include <vector>

#include "src/textlayout/utils/tt_range.h"
#include "src/textlayout/utils/tt_string.h"
namespace ttoffice {
namespace tttext {
class BoundaryAnalyst {
 public:
  BoundaryAnalyst() = delete;
  explicit BoundaryAnalyst(const char32_t* u32_content, uint32_t char_count,
                           LineBreakStrategy line_break_strategy);

 public:
  uint32_t FindNextBoundary(uint32_t start, BoundaryType type) const;
  uint32_t FindPrevBoundary(uint32_t start, BoundaryType type) const;
  BoundaryType GetBoundaryTypeBefore(uint32_t idx) const {
    return idx == 0 ? BoundaryType::kLineBreakable : boundary_[idx - 1];
  }
  BoundaryType GetBoundaryType(uint32_t idx) const {
    TTASSERT(idx < boundary_.size());
    return boundary_[idx];
  }
  void UpgradeBoundaryType(const Range& range, BoundaryType type);

 private:
 private:
  std::vector<BoundaryType> boundary_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_INTERNAL_BOUNDARY_ANALYST_H_
