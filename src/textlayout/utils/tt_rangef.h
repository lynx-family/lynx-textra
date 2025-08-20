// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_RANGEF_H_
#define SRC_TEXTLAYOUT_UTILS_TT_RANGEF_H_

#include <textra/macro.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include "src/textlayout/utils/float_comparison.h"

namespace ttoffice {
namespace tttext {
class RangeF {
  using RangeType = float;

 public:
  RangeF() = default;
  RangeF(const RangeF& other)
      : start_(other.GetStart()), end_(other.GetEnd()) {}
  explicit RangeF(RangeType start, RangeType end) : start_(start), end_(end) {}
  ~RangeF() = default;

 public:
  static constexpr RangeType MaxIndex() {
    return std::numeric_limits<RangeType>::max();
  }
  static constexpr RangeType EmptyIndex() { return MaxIndex(); }

 public:
  RangeType GetStart() const { return start_; }
  void SetStart(RangeType s) { start_ = s; }
  RangeType GetLength() const { return end_ - start_; }
  //  void SetLength(RangeType l) { length_ = l; }
  RangeType GetEnd() const { return end_; }
  void SetEnd(RangeType e) { end_ = e; }
  bool Empty() const { return FloatsEqual(start_, end_); }
  /**
   * Merge two adjacent Ranges, with the following cases:
   * 1. If self is empty, directly modify to other
   * 2. If other is empty or two ranges don't overlap, self remains unchanged
   * 3. Overlapping Ranges merge into one larger Range
   * @param other
   * @return
   */
  RangeF& Merge(const RangeF& other);

  /**
   * Calculate the difference between two Ranges, with the following cases:
   * 1. No intersection or either is empty, return empty Range, self remains
   * unchanged
   * 2. other contains self or covers either endpoint of self, save the result
   * after subtraction and return empty
   * 3. other is contained by self, self saves the left Range, return the right
   * Range If self is contained by other, self will become empty, need to handle
   * carefully
   * @param other
   * @return
   */
  RangeF Difference(const RangeF& other);
  bool Contain(const RangeF& other) const { return Intersect(other) == other; }
  bool Contain(RangeType idx) const {
    return FloatsLargerOrEqual(idx, GetStart()) &&
           FloatsLargerOrEqual(GetEnd(), idx);
  }
  RangeF Intersect(const RangeF& other) const {
    if (Empty() || FloatsLargerOrEqual(other.GetStart(), GetEnd()) ||
        FloatsLargerOrEqual(GetStart(), other.GetEnd()))
      return RangeF{0, 0};
    auto start = std::fmax(GetStart(), other.GetStart());
    auto end = std::fmin(GetEnd(), other.GetEnd());
    TTASSERT(FloatsLarger(end, start));
    return RangeF{start, end};
  }
  bool IsAdjacent(const RangeF& other) {
    return Empty() || other.Empty() ||
           FloatsEqual(GetEnd(), other.GetStart()) ||
           FloatsEqual(GetStart(), other.GetEnd());
  }
  bool operator==(const RangeF& other) const {
    return FloatsEqual(start_, other.GetStart()) &&
           FloatsEqual(end_, other.GetEnd());
  }
  bool operator!=(const RangeF& other) const { return !operator==(other); }
  bool operator<(const RangeF& other) const {
    return FloatsLarger(other.GetStart(), start_) ||
           (FloatsEqual(start_, other.GetStart()) &&
            FloatsLarger(other.GetEnd(), GetEnd()));
  }
  bool operator<(RangeType pt) const {
    return FloatsLargerOrEqual(pt, GetEnd());
  }
  bool operator<=(const RangeF& other) const {
    return operator<(other) || operator==(other);
  }
  bool operator>(const RangeF& other) const {
    return FloatsLarger(start_, other.GetStart()) ||
           (FloatsEqual(start_, other.GetStart()) &&
            FloatsLarger(GetEnd(), other.GetEnd()));
  }
  bool operator>(RangeType pt) const { return FloatsLarger(GetStart(), pt); }
  bool operator>=(const RangeF& other) const {
    return operator>(other) || operator==(other);
  }
  RangeF& operator=(const RangeF& other) = default;

 private:
  RangeType start_;
  RangeType end_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_TT_RANGEF_H_
