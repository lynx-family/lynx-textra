// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_RENDERING_BACKEND_H_
#define PUBLIC_TEXTRA_RENDERING_BACKEND_H_

#include <stdint.h>

/* Shared C/C++ backend identifiers. Factory options may use zero for the
 * compiled native default; explicit enum overloads treat kInvalid as invalid.
 * Each value selects a compatible canvas, font adapter and shaping pipeline.
 * Published numeric values must not be changed or reused. */
typedef uint32_t textra_rendering_backend_t;
#define TEXTRA_RENDERING_BACKEND_INVALID ((textra_rendering_backend_t)0)
#define TEXTRA_RENDERING_BACKEND_SKITY ((textra_rendering_backend_t)1)
#define TEXTRA_RENDERING_BACKEND_SKIA ((textra_rendering_backend_t)2)
#define TEXTRA_RENDERING_BACKEND_ANDROID ((textra_rendering_backend_t)3)
#define TEXTRA_RENDERING_BACKEND_IOS ((textra_rendering_backend_t)4)
#define TEXTRA_RENDERING_BACKEND_MACOS ((textra_rendering_backend_t)5)
#define TEXTRA_RENDERING_BACKEND_OHOS ((textra_rendering_backend_t)6)
#define TEXTRA_RENDERING_BACKEND_LVGL ((textra_rendering_backend_t)7)
#define TEXTRA_RENDERING_BACKEND_IOS_SELF_RENDERING \
  ((textra_rendering_backend_t)8)
#define TEXTRA_RENDERING_BACKEND_MACOS_SELF_RENDERING \
  ((textra_rendering_backend_t)9)
#define TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT ((textra_rendering_backend_t)10)

#ifdef __cplusplus
namespace ttoffice {
namespace tttext {
enum class RenderingBackend : uint32_t {
  kInvalid = TEXTRA_RENDERING_BACKEND_INVALID,
  kSkity = TEXTRA_RENDERING_BACKEND_SKITY,
  kSkia = TEXTRA_RENDERING_BACKEND_SKIA,
  kAndroid = TEXTRA_RENDERING_BACKEND_ANDROID,
  kIOS = TEXTRA_RENDERING_BACKEND_IOS,
  kMacOS = TEXTRA_RENDERING_BACKEND_MACOS,
  kOHOS = TEXTRA_RENDERING_BACKEND_OHOS,
  kLVGL = TEXTRA_RENDERING_BACKEND_LVGL,
  kIOSSelfRendering = TEXTRA_RENDERING_BACKEND_IOS_SELF_RENDERING,
  kMacOSSelfRendering = TEXTRA_RENDERING_BACKEND_MACOS_SELF_RENDERING,
  kSkityCoreText = TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT,
};
}  // namespace tttext
}  // namespace ttoffice
#endif
#endif  // PUBLIC_TEXTRA_RENDERING_BACKEND_H_
