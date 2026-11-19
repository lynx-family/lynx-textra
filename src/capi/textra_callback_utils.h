// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef SRC_CAPI_TEXTRA_CALLBACK_UTILS_H_
#define SRC_CAPI_TEXTRA_CALLBACK_UTILS_H_
#include <textra/capi/textra_platform.h>
#include <textra/platform_helper.h>

#include <algorithm>
#include <cstring>

namespace ttoffice {
namespace tttext {
namespace backend {
inline RenderingBackend ResolveBackend(uint32_t backend) {
  return backend ? static_cast<RenderingBackend>(backend)
                 : PlatformHelper::GetDefaultBackend();
}

// Caller supplies a valid table. Copy known complete fields so optional
// trailing fields remain compatible without retaining a stack-table address.
template <class Api>
void CopyCallbacks(Api& out, const Api* input) {
  size_t bytes = std::min<size_t>(input->struct_size, sizeof(Api));
  if (bytes > 8) bytes = 8 + (bytes - 8) / sizeof(void*) * sizeof(void*);
  std::memcpy(&out, input, bytes);
}

inline bool CallbackBackend(uint32_t b) {
  return b == TEXTRA_RENDERING_BACKEND_SKIA ||
         b == TEXTRA_RENDERING_BACKEND_SKITY ||
         b == TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT ||
         b == TEXTRA_RENDERING_BACKEND_LVGL;
}
inline bool NativeFontTokens(uint32_t backend) {
  return backend == TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT;
}
}  // namespace backend
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_CAPI_TEXTRA_CALLBACK_UTILS_H_
