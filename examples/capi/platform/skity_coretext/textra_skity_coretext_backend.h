// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_BACKEND_H_
#define EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_BACKEND_H_
#include <textra/capi/textra_platform.h>
/* Host Skity/CoreText adapter: backend=SKITY_CORETEXT. Canvas is borrowed;
 * manager input is an optional borrowed skity::FontManager*, font input/output
 * is CTFontRef retained by the adapter. Initialize options in place; adopt
 * owned records once or release them on failure. See
 * public/textra/capi/README.md. Link the host adapter library or compile its
 * .cc directly. This header has no SDK dependencies. */
TEXTRA_EXTERN_C_BEGIN
textra_status_t TEXTRA_CALL textra_skity_coretext_canvas_callbacks_initialize(
    textra_canvas_options_t* options);
textra_status_t TEXTRA_CALL
textra_skity_coretext_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options);
textra_status_t TEXTRA_CALL textra_skity_coretext_typeface_callbacks_initialize(
    textra_typeface_options_t* options);
TEXTRA_EXTERN_C_END
#endif  // EXAMPLES_CAPI_PLATFORM_SKITY_CORETEXT_TEXTRA_SKITY_CORETEXT_BACKEND_H_
