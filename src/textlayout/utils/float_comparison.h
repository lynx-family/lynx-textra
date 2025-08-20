// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

//

#ifndef SRC_TEXTLAYOUT_UTILS_FLOAT_COMPARISON_H_
#define SRC_TEXTLAYOUT_UTILS_FLOAT_COMPARISON_H_

#include <cmath>

namespace ttoffice {
constexpr float Epsilon() { return 0.0001f; }

inline bool FloatsEqual(const float first, const float second) {
  // can not be a nan, but a inf.
  if (first == second) {
    return true;
  }
  return fabs(first - second) < Epsilon();
}

inline bool FloatsNotEqual(const float first, const float second) {
  return !FloatsEqual(first, second);
}

inline bool FloatsLarger(const float first, const float second) {
  if (first == second) {
    return false;
  }
  return first - second >= Epsilon();
}

inline bool FloatsLargerOrEqual(const float first, const float second) {
  return FloatsEqual(first, second) || FloatsLarger(first, second);
}
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_UTILS_FLOAT_COMPARISON_H_
