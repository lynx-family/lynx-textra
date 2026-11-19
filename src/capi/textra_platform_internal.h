// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef SRC_CAPI_TEXTRA_PLATFORM_INTERNAL_H_
#define SRC_CAPI_TEXTRA_PLATFORM_INTERNAL_H_
#include <textra/capi/textra.h>

/* Private C-linkage helpers; hidden even when a host uses default visibility.
 */
#if defined(__GNUC__) || defined(__clang__)
#define TEXTRA_CAPI_INTERNAL __attribute__((visibility("hidden")))
#else
#define TEXTRA_CAPI_INTERNAL
#endif
TEXTRA_EXTERN_C_BEGIN
TEXTRA_CAPI_INTERNAL textra_status_t TEXTRA_CALL
textra_canvas_create(const textra_canvas_options_t*, textra_canvas_t**);
TEXTRA_CAPI_INTERNAL textra_status_t TEXTRA_CALL
textra_typeface_create(const textra_typeface_options_t*, textra_typeface_t**);
TEXTRA_EXTERN_C_END
#undef TEXTRA_CAPI_INTERNAL
#endif  // SRC_CAPI_TEXTRA_PLATFORM_INTERNAL_H_
