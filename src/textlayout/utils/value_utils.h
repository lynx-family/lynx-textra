// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_VALUE_UTILS_H_
#define SRC_TEXTLAYOUT_UTILS_VALUE_UTILS_H_
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace ttoffice {
namespace tttext {
class Utils {
 public:
  static std::pair<uint32_t, uint32_t> RangeIntersection(
      const std::pair<uint32_t, uint32_t>& range_1,
      const std::pair<uint32_t, uint32_t>& range_2) {
    auto start = std::max(range_1.first, range_2.first);
    auto end = std::min(RangeEnd(range_1), RangeEnd(range_2));
    return {start, end > start ? end - start : 0};
  }
  static std::pair<uint32_t, uint32_t> RangeUnion(
      const std::pair<uint32_t, uint32_t>& range_1,
      const std::pair<uint32_t, uint32_t>& range_2) {
    auto start = std::min(range_1.first, range_2.first);
    auto end = std::max(RangeEnd(range_1), RangeEnd(range_2));
    return {start, end - start};
  }
  static uint32_t RangeEnd(const std::pair<uint32_t, uint32_t>& range) {
    return range.first + range.second;
  }
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_VALUE_UTILS_H_
