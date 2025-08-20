// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/harfbuzz_lib.h"

namespace ttoffice {
namespace tttext {

#define HB_FUNC(func_name) func_name,

std::unique_ptr<HarfbuzzLib> HarfbuzzLib::LoadHarfbuzzLib() {
  return std::make_unique<HarfbuzzLib>(
      HarfbuzzLib{HB_EMIT_FUNCS_TYPE1 HB_EMIT_FUNCS_TYPE2});
}

#undef HB_FUNC

}  // namespace tttext
}  // namespace ttoffice
