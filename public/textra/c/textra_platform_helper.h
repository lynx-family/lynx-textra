// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_PLATFORM_HELPER_H_
#define PUBLIC_TEXTRA_C_TEXTRA_PLATFORM_HELPER_H_

#include <textra/c/textra_types.h>

#if defined(__APPLE__)
typedef struct CGContext* CGContextRef;
#endif

TEXTRA_EXTERN_C_BEGIN

/* Creates the serialized Java canvas when ENABLE_JAVASHAPER is built in;
 * otherwise returns UNSUPPORTED. No JVM is needed to write/read its buffer. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_canvas_helper_create_java(textra_canvas_helper_t** out) TEXTRA_NOEXCEPT;

/* Borrowed stream. canvas must be a Java canvas returned by create_java,
 * rather than a callback adapter that delegates to one. */
TEXTRA_API textra_buffer_output_stream_t* TEXTRA_CALL
textra_canvas_helper_get_java_buffer(textra_canvas_helper_t* canvas)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_platform_helper_create_font_manager(
    textra_platform_type_t type, textra_font_manager_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_platform_helper_create_canvas_helper(
    textra_platform_type_t type, void* handler,
    textra_canvas_helper_t** out) TEXTRA_NOEXCEPT;

#if defined(__APPLE__)
/* Creates the platform's existing CoreText font manager. Release the returned
 * shared handle with textra_font_manager_release. Available on macOS/iOS. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_font_manager_create_coretext(
    textra_font_manager_t** out) TEXTRA_NOEXCEPT;

/* The CGContext is borrowed and must outlive the helper. Destroy the returned
 * helper with textra_canvas_helper_destroy. Canvas transforms follow the
 * existing platform adapter. Available on macOS/iOS. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_canvas_helper_create_coregraphics(
    CGContextRef context, textra_canvas_helper_t** out) TEXTRA_NOEXCEPT;
#endif

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_PLATFORM_HELPER_H_
