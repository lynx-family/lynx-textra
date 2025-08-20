// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/tt_range.h"

#include <textra/macro.h>

#include <limits>

namespace ttoffice {
namespace tttext {

void Range::SetLength(Range::RangeType l) {
  end_ = (std::numeric_limits<RangeType>::max() - start_ < l)
             ? std::numeric_limits<RangeType>::max()
             : start_ + l;
}
Range& Range::Merge(const Range& other) {
  if (Empty()) {
    *this = other;
  } else {
    if (other.Empty() ||
        (!IsAdjacent(other) && Range::Intersect(*this, other).Empty()))
      return *this;
    auto start = std::min(GetStart(), other.GetStart());
    auto end = std::max(GetEnd(), other.GetEnd());
    *this = Range{start, end};
  }
  return *this;
}
Range Range::Difference(const Range& other) {
  auto ret = Range{0, 0};
  if (Empty() || other.Empty()) return ret;
  auto r = Range::Intersect(*this, other);
  if (r.Empty()) return ret;
  if (r == *this) {
    start_ = end_ = 0;
  } else if (r.GetStart() == GetStart()) {
    SetStart(r.GetEnd());
  } else if (r.GetEnd() == GetEnd()) {
    SetEnd(r.GetStart());
  } else {
    auto end = GetEnd();
    SetEnd(r.GetStart());
    ret.SetStart(r.GetEnd());
    ret.SetEnd(end);
  }
  return ret;
}
Range Range::Intersect(const Range& left, const Range& right) {
  if (left.Empty() || left.GetEnd() <= right.GetStart() ||
      right.GetEnd() <= left.GetStart())
    return Range{0, 0};
  auto start = std::max(left.GetStart(), right.GetStart());
  auto end = std::min(left.GetEnd(), right.GetEnd());
  TTASSERT(end - start >= 0);
  return Range{start, end};
}
void Range::Intersect(const Range& other) {
  if (Empty() || GetEnd() <= other.GetStart() || other.GetEnd() <= GetStart()) {
    start_ = end_ = 0;
  } else {
    start_ = std::max(GetStart(), other.GetStart());
    end_ = std::min(GetEnd(), other.GetEnd());
  }
  TTASSERT(GetLength() >= 0);
}
void Range::Swap() {
  auto tmp = start_;
  start_ = end_;
  end_ = tmp;
}
bool Range::operator==(const Range& other) const {
  return start_ == other.GetStart() && end_ == other.GetEnd();
}
bool Range::operator<(const Range& other) const {
  return start_ < other.GetStart() ||
         (start_ == other.GetStart() && GetEnd() < other.GetEnd());
}
bool Range::operator>(const Range& other) const {
  return start_ > other.GetStart() ||
         (start_ == other.GetStart() && GetEnd() > other.GetEnd());
}
}  // namespace tttext
}  // namespace ttoffice
