// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_PAINTER_H_
#define PUBLIC_TEXTRA_C_TEXTRA_PAINTER_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_painter_create(textra_painter_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_destroy(textra_painter_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_painter_get_fill_color(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_fill_color(
    textra_painter_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_painter_get_stroke_color(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_stroke_color(
    textra_painter_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_painter_get_stroke_width(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_stroke_width(
    textra_painter_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_painter_get_stroke_miter(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_stroke_miter(
    textra_painter_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_painter_get_text_size(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_text_size(
    textra_painter_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_painter_get_text_skew(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_text_skew(
    textra_painter_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_painter_is_bold(const textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_bold(
    textra_painter_t* painter, uint32_t value) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_painter_is_italic(const textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_italic(
    textra_painter_t* painter, uint32_t value) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_painter_is_under_line(const textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_under_line(
    textra_painter_t* painter, uint32_t value) TEXTRA_NOEXCEPT;

TEXTRA_API textra_cap_t TEXTRA_CALL
textra_painter_get_cap(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_cap(
    textra_painter_t* value, textra_cap_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_join_t TEXTRA_CALL
textra_painter_get_join(const textra_painter_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_join(
    textra_painter_t* value, textra_join_t input) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_font_family(
    textra_painter_t* painter, const char* family) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_painter_get_font_family(
    const textra_painter_t* painter, char* buffer, size_t capacity,
    size_t* required_bytes) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_fill_gradient(
    textra_painter_t* painter,
    textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

/* Owned shared handle, or NULL. */
TEXTRA_API textra_resolved_gradient_t* TEXTRA_CALL
textra_painter_get_fill_gradient(const textra_painter_t* painter)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_painter_set_shadow_list(
    textra_painter_t* painter, const textra_text_shadow_t* shadows,
    size_t count) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_painter_get_shadow_list(
    const textra_painter_t* painter, textra_text_shadow_t* shadows,
    size_t capacity, size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_PAINTER_H_
