// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/sk_shaper.h"

namespace ttoffice {
namespace tttext {

std::unique_ptr<SkShaper> SkShaper::MakeShapeDontWrapOrReorderForPlatform() {
  return nullptr;
}
}  // namespace tttext
}  // namespace ttoffice
