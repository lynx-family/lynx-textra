// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_H_
#define PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API void TEXTRA_CALL
textra_paragraph_save_style(textra_paragraph_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_restore_style(textra_paragraph_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_create(
    textra_paragraph_type_t type, textra_paragraph_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_destroy(textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

/* Borrowed and mutable; do not destroy. General paragraphs only. */
TEXTRA_API textra_paragraph_style_t* TEXTRA_CALL
textra_paragraph_get_paragraph_style(textra_paragraph_t* paragraph)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_set_paragraph_style(
    textra_paragraph_t* paragraph,
    const textra_paragraph_style_t* style) TEXTRA_NOEXCEPT;

/* style may be NULL, using the paragraph default. utf8 is valid UTF-8. */
TEXTRA_API void TEXTRA_CALL textra_paragraph_add_text_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    const char* utf8, uint32_t byte_count) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_add_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate, uint32_t is_float) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_add_ghost_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_apply_style_in_range(
    const textra_paragraph_t* paragraph, const textra_style_t* style,
    uint32_t start_char, uint32_t char_count) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_query_style(textra_paragraph_t* paragraph, uint32_t char_index,
                             textra_style_t* out_style) TEXTRA_NOEXCEPT;

TEXTRA_API textra_paragraph_type_t TEXTRA_CALL
textra_paragraph_get_type(const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API textra_write_direction_t TEXTRA_CALL
textra_paragraph_get_resolved_write_direction(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_get_char_count(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_get_run_count(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_paragraph_get_width(const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_height(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_baseline(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_ascent(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_descent(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_min_intrinsic_width(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_get_max_intrinsic_width(
    const textra_paragraph_t* paragraph) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_content_string(
    const textra_paragraph_t* paragraph, uint32_t start_char,
    uint32_t char_count, char* buffer, size_t capacity,
    size_t* required_bytes) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_get_word_boundary(
    const textra_paragraph_t* paragraph, uint32_t offset,
    textra_char_range_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_get_grapheme_boundary(
    const textra_paragraph_t* paragraph, uint32_t offset,
    textra_char_range_t* out) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_H_
