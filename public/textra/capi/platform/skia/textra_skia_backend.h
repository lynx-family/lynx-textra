// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
#ifndef __cplusplus
#error "The header-only Skia adapter requires C++17 and the Skia SDK"
#endif

#include <textra/capi/platform/skia/textra_skia_canvas_impl.hpp>
#include <textra/capi/platform/skia/textra_skia_font_manager_impl.hpp>
/* Native canvas: borrowed SkCanvas*. Manager: required SkFontMgr* (retained).
 * Native face: SkTypeface* (retained). */
/* C++17 header-only host adapter exposing C-style initializer signatures.
 * Include the umbrella and link the Skia SDK and Textra core. No adapter
 * source, separate adapter library or implementation macro is required.
 */
/* Initializers update one options record in place. Canvas/Typeface fill
 * backend; the manager uses its TextLayout backend. Set struct_size and the
 * native input pointer; userdata is preserved. Failed initialization leaves the
 * record unchanged. Owned Manager/Typeface records reject reinitialization. On
 * success, adopt once into an engine owner or call the record's destroy
 * callback if it was not adopted. Keep userdata and callback code alive through
 * all dependent objects/caches. Borrowed Canvas inputs keep their native
 * pointer and use a no-op destroy.
 */
TEXTRA_EXTERN_C_BEGIN
static inline textra_status_t TEXTRA_CALL
textra_skia_canvas_callbacks_initialize(textra_canvas_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      !options->native_canvas)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  options->backend = TEXTRA_RENDERING_BACKEND_SKIA;
  options->callbacks = &textra_native::skia_backend::CanvasApi();
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skia_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options) {
  namespace native = textra_native::skia_backend;
  return native::InitializeManager(options, native::ManagerApi(),
                                   native::FaceApi(), native::CreateManager);
}
static inline textra_status_t TEXTRA_CALL
textra_skia_typeface_callbacks_initialize(textra_typeface_options_t* options) {
  namespace native = textra_native::skia_backend;
  return native::InitializeTypeface(options, TEXTRA_RENDERING_BACKEND_SKIA,
                                    native::FaceApi(), native::Import);
}
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
