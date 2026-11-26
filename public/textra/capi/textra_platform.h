// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_PLATFORM_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_PLATFORM_H_

#include <textra/capi/textra_backend.h>

/* Shared factory options for native and callback-backed I* adapters.
 * backend=0 selects the engine's compiled native platform. With callbacks=NULL,
 * native_* is an SDK handle and the factory returns the original built-in I*.
 * Skia/Skity require an explicit backend and callback table; native_* is then
 * the callback object's opaque pointer. Tables are copied at construction.
 * Callback success adopts one native reference; failure leaves it with the
 * caller. userdata/callback modules must outlive every dependent object/cache.
 * Native lifetimes follow the original platform: CGContext/OH canvas/LVGL layer
 * borrowed, Java Canvas retained; CTFont retained, Java Typeface registered,
 * OH/LVGL font borrowed through all dependents and caches. Unknown flags are
 * unsupported.
 */
typedef struct textra_canvas_options {
  uint32_t struct_size;
  textra_rendering_backend_t backend; /* 0 = compiled native default. */
  uint32_t flags;                     /* Must be zero. */
  void* native_canvas;
  const textra_backend_canvas_api_t* callbacks;
  void* userdata;
} textra_canvas_options_t;

/* One font source. The TextLayout selects the backend for all four sources.
 * NULL callback tables select the built-in native manager. Custom managers
 * provide both tables. On successful layout creation one manager reference is
 * adopted; on failure it remains with the caller. Reuse requires another valid
 * reference. Tables are copied; userdata/modules outlive dependent results. */
typedef struct textra_font_manager_options {
  uint32_t struct_size;
  void* font_manager; /* Callback object; NULL for built-in native managers. */
  const textra_backend_font_manager_api_t* manager_callbacks;
  const textra_backend_typeface_api_t* typeface_callbacks;
  void* userdata;
} textra_font_manager_options_t;

typedef struct textra_typeface_options {
  uint32_t struct_size;
  textra_rendering_backend_t backend; /* 0 = compiled native default. */
  uint32_t flags;                     /* Must be zero. */
  const void* native_font; /* Native font or callback-owned face object. */
  const textra_backend_typeface_api_t* callbacks;
  void* userdata;
} textra_typeface_options_t;

TEXTRA_EXTERN_C_BEGIN
/* Select the compiled host-drawn shaping backend for SKIA or SKITY.
 * Prefer the requested SkShaper backend, then SKITY_CORETEXT when available.
 * The caller supplies compatible canvas/font callbacks, including native
 * CoreText font access for the fallback. Returns INVALID for other inputs or
 * when neither implementation is compiled. Does not preload runtime shaping
 * dependencies or change explicit factory behavior. Available since ABI 1.1. */
TEXTRA_API textra_rendering_backend_t TEXTRA_CALL
textra_get_self_rendering_backend(textra_rendering_backend_t preferred);

/* Initializes the original Android Java/C++ SDK.
 * Other engine builds return UNSUPPORTED. Call before concurrent layout. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_android_runtime_initialize(void* jni_env);
TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_PLATFORM_H_
