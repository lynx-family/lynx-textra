// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/harfbuzz_lib.h"

namespace ttoffice {
namespace tttext {

bool HarfbuzzLib::HasHarfbuzzLib() { return GetHarfbuzzLib() != nullptr; }

const HarfbuzzLib* HarfbuzzLib::GetHarfbuzzLib() {
  static const auto hb_lib = HarfbuzzLib::LoadHarfbuzzLib();
  return hb_lib.get();
}

}  // namespace tttext
}  // namespace ttoffice
