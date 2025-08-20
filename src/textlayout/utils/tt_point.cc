// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/utils/tt_point.h"

#include "src/textlayout/utils/float_comparison.h"
namespace ttoffice {
namespace tttext {
bool PointF::operator==(const PointF& point) const {
  return FloatsEqual(x_, point.x_) && FloatsEqual(y_, point.y_);
}
}  // namespace tttext
}  // namespace ttoffice
