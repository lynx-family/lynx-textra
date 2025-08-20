// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/tt_rangef.h"

namespace ttoffice {
namespace tttext {
// const RangeF RangeF::EMPTY_RANGE(RangeF::EMPTY_INDEX, RangeF::EMPTY_INDEX);
RangeF& RangeF::Merge(const RangeF& other) {
  if (Empty()) {
    *this = other;
  } else {
    if (other.Empty() || (!IsAdjacent(other) && Intersect(other).Empty()))
      return *this;
    auto start = std::fmin(GetStart(), other.GetStart());
    auto end = std::fmax(GetEnd(), other.GetEnd());
    *this = RangeF{start, end};
  }
  return *this;
}
RangeF RangeF::Difference(const RangeF& other) {
  RangeF ret(0, 0);
  if (Empty() || other.Empty()) return ret;
  auto r = Intersect(other);
  if (r.Empty()) return ret;
  if (r == *this) {
    start_ = end_ = EmptyIndex();
  } else if (FloatsEqual(r.GetStart(), GetStart())) {
    SetStart(r.GetEnd());
  } else if (FloatsEqual(r.GetEnd(), GetEnd())) {
    SetEnd(r.GetStart());
  } else {
    auto end = GetEnd();
    SetEnd(r.GetStart());
    ret.SetStart(r.GetEnd());
    ret.SetEnd(end);
  }
  return ret;
}
}  // namespace tttext
}  // namespace ttoffice
