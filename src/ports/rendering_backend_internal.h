// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef SRC_PORTS_RENDERING_BACKEND_INTERNAL_H_
#define SRC_PORTS_RENDERING_BACKEND_INTERNAL_H_
#include <textra/rendering_backend.h>

namespace ttoffice {
namespace tttext {
namespace internal {
// Compiled backend availability. Drawing adapters are host supplied.
inline bool IsBackendCompiled(RenderingBackend backend) {
  switch (backend) {
#if defined(TEXTRA_ENABLE_ANDROID_BACKEND)
    case RenderingBackend::kAndroid:
#endif
#if defined(TEXTRA_ENABLE_IOS_BACKEND) && defined(ENABLE_CTSHAPER)
    case RenderingBackend::kIOS:
    case RenderingBackend::kIOSSelfRendering:
#endif
#if defined(TEXTRA_ENABLE_MACOS_BACKEND) && defined(ENABLE_CTSHAPER)
    case RenderingBackend::kMacOS:
    case RenderingBackend::kMacOSSelfRendering:
#endif
#if defined(TEXTRA_ENABLE_OHOS_BACKEND)
    case RenderingBackend::kOHOS:
#endif
#if defined(TEXTRA_ENABLE_LVGL_BACKEND)
    case RenderingBackend::kLVGL:
#endif
#if defined(ENABLE_SKSHAPER)
    case RenderingBackend::kSkia:
    case RenderingBackend::kSkity:
#endif
#if defined(ENABLE_CTSHAPER)
    case RenderingBackend::kSkityCoreText:
#endif
      return true;
    default:
      return false;
  }
}
}  // namespace internal
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_RENDERING_BACKEND_INTERNAL_H_
