// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef EXAMPLES_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
#define EXAMPLES_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
#include <textra/capi/textra_platform.h>
/* Native canvas: borrowed SkCanvas*. Manager: required SkFontMgr* (retained).
 * Native face: SkTypeface* (retained). */
/* Include this declaration-only header and link the host adapter library
 * (or compile its .cc once). No implementation macro or SDK headers required.
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
textra_status_t TEXTRA_CALL
textra_skia_canvas_callbacks_initialize(textra_canvas_options_t* options);
textra_status_t TEXTRA_CALL textra_skia_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options);
textra_status_t TEXTRA_CALL
textra_skia_typeface_callbacks_initialize(textra_typeface_options_t* options);
TEXTRA_EXTERN_C_END
#endif  // EXAMPLES_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_BACKEND_H_
