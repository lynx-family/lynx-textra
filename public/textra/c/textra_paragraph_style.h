// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_STYLE_H_
#define PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_STYLE_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_create(textra_paragraph_style_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_destroy(textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_copy(const textra_paragraph_style_t* source,
                            textra_paragraph_style_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API textra_paragraph_horizontal_alignment_t TEXTRA_CALL
textra_paragraph_style_get_horizontal_align(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_horizontal_align(
    textra_paragraph_style_t* value,
    textra_paragraph_horizontal_alignment_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_paragraph_vertical_alignment_t TEXTRA_CALL
textra_paragraph_style_get_vertical_align(const textra_paragraph_style_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_vertical_align(
    textra_paragraph_style_t* value,
    textra_paragraph_vertical_alignment_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_dominant_baseline_t TEXTRA_CALL
textra_paragraph_style_get_dominant_baseline(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_dominant_baseline(
    textra_paragraph_style_t* value,
    textra_dominant_baseline_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_write_direction_t TEXTRA_CALL
textra_paragraph_style_get_write_direction(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_write_direction(
    textra_paragraph_style_t* value,
    textra_write_direction_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_line_height_trim_t TEXTRA_CALL
textra_paragraph_style_get_line_height_trim(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_height_trim(
    textra_paragraph_style_t* value,
    textra_line_height_trim_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_inline_vertical_alignment_mode_t TEXTRA_CALL
textra_paragraph_style_get_inline_vertical_alignment_mode(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_inline_vertical_alignment_mode(
    textra_paragraph_style_t* value,
    textra_inline_vertical_alignment_mode_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_overflow_wrap_t TEXTRA_CALL
textra_paragraph_style_get_overflow_wrap(const textra_paragraph_style_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_overflow_wrap(
    textra_paragraph_style_t* value,
    textra_overflow_wrap_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_punctuation_compress_option_t TEXTRA_CALL
textra_paragraph_style_get_punctuation_compress_options(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_punctuation_compress_options(
    textra_paragraph_style_t* value,
    textra_punctuation_compress_option_t input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_start_indent_in_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_start_indent_in_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_end_indent_in_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_end_indent_in_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_first_line_indent_in_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_first_line_indent_in_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_hanging_indent_in_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_hanging_indent_in_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_line_height_in_percent(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_height_in_percent(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_line_space_before_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_space_before_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_line_space_after_px(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_space_after_px(
    textra_paragraph_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API int32_t TEXTRA_CALL
textra_paragraph_style_get_start_indent_in_char_cnt(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_start_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API int32_t TEXTRA_CALL
textra_paragraph_style_get_end_indent_in_char_cnt(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_end_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API int32_t TEXTRA_CALL
textra_paragraph_style_get_first_line_indent_in_char_cnt(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_first_line_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API int32_t TEXTRA_CALL
textra_paragraph_style_get_hanging_indent_in_char_cnt(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_hanging_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_style_get_max_lines(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_max_lines(
    textra_paragraph_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_style_get_line_height_override(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_height_override(
    textra_paragraph_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_style_get_half_leading(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_half_leading(
    textra_paragraph_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_paragraph_style_get_enable_text_bounds(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_enable_text_bounds(
    textra_paragraph_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_paragraph_style_get_ellipsis_uses_default_foreground(
    const textra_paragraph_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_ellipsis_uses_default_foreground(
    textra_paragraph_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_default_style(
    textra_paragraph_style_t* style,
    const textra_style_t* default_style) TEXTRA_NOEXCEPT;

/* Borrowed, read-only. */
TEXTRA_API const textra_style_t* TEXTRA_CALL
textra_paragraph_style_get_default_style(const textra_paragraph_style_t* style)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_height_in_px(
    textra_paragraph_style_t* style, float height,
    textra_ruler_type_t ruler) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_paragraph_style_get_line_height_in_px(
    const textra_paragraph_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API textra_ruler_type_t TEXTRA_CALL
textra_paragraph_style_get_line_height_rule(
    const textra_paragraph_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_ellipsis_text(
    textra_paragraph_style_t* style,
    const char* nul_terminated_utf8) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_ellipsis_delegate(
    textra_paragraph_style_t* style,
    textra_run_delegate_t* delegate) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_allow_break_around_punctuation(
    textra_paragraph_style_t* style, uint32_t allow) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_assign(
    textra_paragraph_style_t* value,
    const textra_paragraph_style_t* source) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_line_height_in_px_exact(
    textra_paragraph_style_t* value, float px) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_set_line_height_in_px_at_least(
    textra_paragraph_style_t* value, float px) TEXTRA_NOEXCEPT;

/* Preserves the C++ UTF-32 result; counts are code points. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_ellipsis(
    const textra_paragraph_style_t* style, uint32_t* codepoints,
    size_t capacity, size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_paragraph_style_set_ellipsis_utf16(
    textra_paragraph_style_t* style, const uint16_t* text,
    size_t count) TEXTRA_NOEXCEPT;

/* Owned shared handle, or NULL. */
TEXTRA_API textra_run_delegate_t* TEXTRA_CALL
textra_paragraph_style_get_ellipsis_delegate(
    const textra_paragraph_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_update_punctuation_compress_config(
    textra_paragraph_style_t* style, uint32_t unicode,
    textra_punctuation_type_t type, float default_ratio, float line_edge_ratio,
    float adjacent_ratio) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_PARAGRAPH_STYLE_H_
