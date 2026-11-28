// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_TEXTRA_SKITY_BACKEND_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_TEXTRA_SKITY_BACKEND_H_

#include <textra/capi/platform/skity_common/textra_skity_canvas_impl.h>
#include <textra/capi/platform/skity_common/textra_skity_font_manager_impl.h>

/* C11 header-only host adapter. Include this header and link the Textra and
 * Skity SDK libraries; no adapter source or implementation macro is required.
 * Options are updated in place only on success; userdata is preserved.
 * Canvas is borrowed and its destroy callback is a no-op. Owned font records
 * must be adopted exactly once or released with their destroy callback.
 * Manager input is an optional borrowed skity_font_manager (must outlive
 * dependent objects); NULL creates an owned default manager. Typeface input
 * is a borrowed skity_typeface, retained by initialization until destruction.
 */
TEXTRA_EXTERN_C_BEGIN
static inline textra_status_t TEXTRA_CALL
textra_skity_canvas_callbacks_initialize(textra_canvas_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      !options->native_canvas)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  options->backend = TEXTRA_RENDERING_BACKEND_SKITY;
  options->callbacks = &textra_skity_impl_canvas_api;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) ||
      options->manager_callbacks || options->typeface_callbacks)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  void* object = NULL;
  textra_status_t status = textra_skity_impl_create_manager(
      options->userdata, options->font_manager, &object);
  if (status) return status;
  options->font_manager = object;
  options->manager_callbacks = &textra_skity_impl_font_manager_api;
  options->typeface_callbacks = &textra_skity_impl_typeface_api;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_typeface_callbacks_initialize(textra_typeface_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      options->callbacks || !options->native_font)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  void* object = NULL;
  textra_status_t status = textra_skity_impl_import_typeface(
      options->userdata, options->native_font, &object);
  if (status) return status;
  if (!object) return TEXTRA_STATUS_NOT_FOUND;
  options->backend = TEXTRA_RENDERING_BACKEND_SKITY;
  options->native_font = object;
  options->callbacks = &textra_skity_impl_typeface_api;
  return TEXTRA_STATUS_OK;
}
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_TEXTRA_SKITY_BACKEND_H_
