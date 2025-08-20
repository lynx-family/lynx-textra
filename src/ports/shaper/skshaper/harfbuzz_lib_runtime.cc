// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef HARFBUZZ_LIB_NAME
#error "HARFBUZZ_LIB_NAME macro must be defined in build configuration"
#endif

#include <dlfcn.h>

#include "src/ports/shaper/skshaper/harfbuzz_lib.h"

namespace ttoffice {
namespace tttext {

std::unique_ptr<HarfbuzzLib> HarfbuzzLib::LoadHarfbuzzLib() {
  void* dlhnd = dlopen(HARFBUZZ_LIB_NAME, RTLD_LAZY);

  if (!dlhnd) {
    return nullptr;
  }

  bool resolved_all_syms = true;

  auto resolve_sym = [&](void* hnd, const char name[]) -> void* {
    if (auto* addr = dlsym(dlhnd, name)) {
      return addr;
    }
    resolved_all_syms = false;
    return nullptr;
  };

  HarfbuzzLib hb_lib{};
#define HB_FUNC(func_name)                           \
  *reinterpret_cast<void**>(&hb_lib.f_##func_name) = \
      resolve_sym(dlhnd, #func_name);
  HB_EMIT_FUNCS_TYPE1
  HB_EMIT_FUNCS_TYPE2
#undef HB_FUNC

  if (!resolved_all_syms) {
    dlclose(dlhnd);
    return nullptr;
  }

  return std::make_unique<HarfbuzzLib>(hb_lib);
}

#undef HB_FUNC

}  // namespace tttext
}  // namespace ttoffice
