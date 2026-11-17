// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_TEXT_LINE_H_
#define PUBLIC_TEXTRA_C_TEXTRA_TEXT_LINE_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API uint32_t TEXTRA_CALL textra_text_line_get_start_char_pos(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_text_line_get_end_char_pos(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_text_line_get_char_count(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_line_top(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_line_bottom(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_line_height(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_line_base_line(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_line_left(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_line_right(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_line_width(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_max_ascent(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_max_descent(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_content_top(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_content_baseline(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_content_bottom(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_content_height(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_text_line_get_start_indent(
    const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_text_line_get_end_indent(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

/* Borrowed. */
TEXTRA_API textra_paragraph_t* TEXTRA_CALL
textra_text_line_get_paragraph(const textra_text_line_t* line) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_text_line_get_char_pos_by_coordinate_x(
    const textra_text_line_t* line, float x) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_text_line_get_bounding_rect_by_char_range(
    const textra_text_line_t* line, uint32_t start_char, uint32_t end_char,
    textra_rect_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_text_line_get_tight_bounding_rect_by_char_range(
    const textra_text_line_t* line, uint32_t start_char, uint32_t end_char,
    textra_tight_bounds_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_text_line_modify_horizontal_alignment(
    textra_text_line_t* line,
    textra_paragraph_horizontal_alignment_t alignment) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_text_line_is_empty(const textra_text_line_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_text_line_is_last_line_of_paragraph(
    const textra_text_line_t* value) TEXTRA_NOEXCEPT;

/* NULL uses the same default behavior as TextLine::StripByEllipsis. */
TEXTRA_API void TEXTRA_CALL textra_text_line_strip_by_ellipsis(
    textra_text_line_t* line,
    const uint32_t* nul_terminated_utf32) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_text_line_get_char_bounding_rect(
    const textra_text_line_t* line, uint32_t char_pos,
    textra_rect_t* out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_text_line_get_bounding_rect_for_line(
    const textra_text_line_t* line, textra_rect_t* out) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_TEXT_LINE_H_
