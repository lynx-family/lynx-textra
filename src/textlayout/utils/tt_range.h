// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_UTILS_TT_RANGE_H_
#define SRC_TEXTLAYOUT_UTILS_TT_RANGE_H_

#include <textra/macro.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace ttoffice {
namespace tttext {
class L_EXPORT Range {
 public:
  using RangeType = uint32_t;

 public:
  Range() = default;
  explicit Range(RangeType start, RangeType end) : start_(start), end_(end) {}
  ~Range() = default;

 public:
  static constexpr RangeType MaxIndex() {
    return std::numeric_limits<RangeType>::max();
  }
  static constexpr RangeType EmptyIndex() { return MaxIndex(); }

 public:
  static Range MakeLR(RangeType left, RangeType right) {
    return Range{left, right};
  }
  static Range MakeLW(RangeType left, RangeType width) {
    return Range{left, MaxIndex() - left < width ? MaxIndex() : left + width};
  }

 public:
  RangeType GetStart() const { return start_; }
  void SetStart(RangeType s) { start_ = s; }
  RangeType GetLength() const { return end_ > start_ ? end_ - start_ : 0; }
  RangeType Width() const {
    return end_ > start_ ? end_ - start_ : start_ - end_;
  }
  void SetLength(RangeType l);
  RangeType GetEnd() const { return end_; }
  void SetEnd(RangeType e) { end_ = e; }
  bool Empty() const { return GetLength() == 0; }
  bool Offset(int32_t offset) {
    if (offset > 0 && std::numeric_limits<RangeType>::max() - GetEnd() <
                          static_cast<RangeType>(offset))
      return false;
    if (offset < 0 && GetStart() - 0 < static_cast<RangeType>(-offset))
      return false;
    start_ += offset;
    end_ += offset;
    return true;
  }
  /**
   * Merge two adjacent Ranges, with the following cases:
   * 1. If self is empty, directly modify to other
   * 2. If other is empty or the two ranges don't overlap, self is not modified
   * 3. Overlapping Ranges are merged into one large Range
   * @param other
   * @return
   */
  Range& Merge(const Range& other);

  /**
   * Calculate the difference set of two Ranges, with the following cases:
   * 1. No intersection or either is empty, return empty Range, self unchanged
   * 2. other contains self or covers any endpoint of self, save the result
   * after subtraction and return empty
   * 3. other is contained by self, self saves the left Range, return the right
   * Range If self is contained by other, self becomes empty set, need careful
   * handling
   * @param other
   * @return
   */
  Range Difference(const Range& other);
  bool Contain(const Range& other) const {
    return GetStart() <= other.GetStart() && other.GetEnd() <= GetEnd();
  }
  bool Contain(RangeType idx) const {
    return GetStart() <= idx && idx < GetEnd();
  }
  static Range Intersect(const Range& left, const Range& right);
  void Intersect(const Range& other);
  bool IsIntersect(const Range& other) const {
    return !(GetEnd() <= other.GetStart() || GetStart() >= other.GetEnd());
  }
  bool IsAdjacent(const Range& other) {
    return Empty() || other.Empty() || GetEnd() == other.GetStart() ||
           GetStart() == other.GetEnd();
  }
  void Swap();
  bool operator==(const Range& other) const;
  bool operator!=(const Range& other) const { return !operator==(other); }
  bool operator<(const Range& other) const;
  bool operator<(RangeType pt) const { return GetEnd() <= pt; }
  bool operator<=(const Range& other) const {
    return operator<(other) || operator==(other);
  }
  bool operator>(const Range& other) const;
  bool operator>(RangeType pt) const { return GetStart() > pt; }
  bool operator>=(const Range& other) const {
    return operator>(other) || operator==(other);
  }

 private:
  RangeType start_;
  RangeType end_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_UTILS_TT_RANGE_H_
