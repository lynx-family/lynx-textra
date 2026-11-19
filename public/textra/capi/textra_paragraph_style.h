// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_STYLE_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_STYLE_H_

#include <textra/capi/textra_style.h>

/* Paragraph style setters copy their inputs. Getters producing style handles
 * return independent copies. Spacing/indents use layout units unless explicitly
 * named _in_chars (font-relative character counts). Max lines = 0 is unlimited.
 * V1 defaults: LEFT/CENTER/ALPHABETIC alignment, AUTO direction, ANYWHERE wrap,
 * no indents/spacing/trim/ellipsis/punctuation compression, LEGACY inline
 * alignment, unlimited lines, automatic line height with factor=1. All boolean
 * options are false, including break_around_punctuation_allowed.
 */
typedef struct textra_punctuation_compress_config {
  uint32_t struct_size;
  uint32_t codepoint;
  textra_punctuation_type_t type; /* Default NONE. */
  float default_ratio;
  float line_edge_ratio;
  float adjacent_ratio;
} textra_punctuation_compress_config_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_create(textra_paragraph_style_t** out_style);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_clone(const textra_paragraph_style_t* source,
                             textra_paragraph_style_t** out_paragraph_style);
TEXTRA_API void TEXTRA_CALL
textra_paragraph_style_destroy(textra_paragraph_style_t* paragraph_style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_default_style(
    textra_paragraph_style_t* style, const textra_style_t* default_style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_default_style(
    const textra_paragraph_style_t* style, textra_style_t** out_default_style);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_horizontal_alignment(
    textra_paragraph_style_t* paragraph_style,
    textra_horizontal_alignment_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_horizontal_alignment(
    const textra_paragraph_style_t* paragraph_style,
    textra_horizontal_alignment_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_vertical_alignment(
    textra_paragraph_style_t* paragraph_style,
    textra_vertical_alignment_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_vertical_alignment(
    const textra_paragraph_style_t* paragraph_style,
    textra_vertical_alignment_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_dominant_baseline(
    textra_paragraph_style_t* paragraph_style,
    textra_dominant_baseline_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_dominant_baseline(
    const textra_paragraph_style_t* paragraph_style,
    textra_dominant_baseline_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_write_direction(
    textra_paragraph_style_t* paragraph_style, textra_write_direction_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_write_direction(
    const textra_paragraph_style_t* paragraph_style,
    textra_write_direction_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_spacing_before(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_spacing_before(
    const textra_paragraph_style_t* paragraph_style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_spacing_after(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_spacing_after(
    const textra_paragraph_style_t* paragraph_style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_line_height_trim(
    textra_paragraph_style_t* paragraph_style, textra_line_height_trim_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_line_height_trim(
    const textra_paragraph_style_t* paragraph_style,
    textra_line_height_trim_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_inline_alignment_mode(
    textra_paragraph_style_t* paragraph_style,
    textra_inline_alignment_mode_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_inline_alignment_mode(
    const textra_paragraph_style_t* paragraph_style,
    textra_inline_alignment_mode_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_line_space_before(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_line_space_before(
    const textra_paragraph_style_t* paragraph_style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_line_space_after(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_line_space_after(
    const textra_paragraph_style_t* paragraph_style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_max_lines(
    textra_paragraph_style_t* paragraph_style, uint32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_max_lines(
    const textra_paragraph_style_t* paragraph_style, uint32_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_line_height_override(
    textra_paragraph_style_t* paragraph_style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_line_height_override(
    const textra_paragraph_style_t* paragraph_style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_half_leading(
    textra_paragraph_style_t* paragraph_style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_half_leading(
    const textra_paragraph_style_t* paragraph_style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_text_bounds_enabled(
    textra_paragraph_style_t* paragraph_style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_text_bounds_enabled(
    const textra_paragraph_style_t* paragraph_style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_overflow_wrap(
    textra_paragraph_style_t* paragraph_style, textra_overflow_wrap_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_overflow_wrap(
    const textra_paragraph_style_t* paragraph_style,
    textra_overflow_wrap_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_break_around_punctuation_allowed(
    textra_paragraph_style_t* paragraph_style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_break_around_punctuation_allowed(
    const textra_paragraph_style_t* paragraph_style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_punctuation_compress(
    textra_paragraph_style_t* paragraph_style,
    textra_punctuation_compress_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_punctuation_compress(
    const textra_paragraph_style_t* paragraph_style,
    textra_punctuation_compress_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_ellipsis_uses_default_foreground(
    textra_paragraph_style_t* paragraph_style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_ellipsis_uses_default_foreground(
    const textra_paragraph_style_t* paragraph_style, textra_bool_t* out_value);

/* Setting either indent unit clears the other unit for this indent. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_start_indent(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_start_indent(
    const textra_paragraph_style_t* paragraph_style, float* out_value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_start_indent_in_chars(
    textra_paragraph_style_t* paragraph_style, int32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_start_indent_in_chars(
    const textra_paragraph_style_t* paragraph_style, int32_t* out_value);

/* Setting either indent unit clears the other unit for this indent. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_end_indent(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_end_indent(
    const textra_paragraph_style_t* paragraph_style, float* out_value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_end_indent_in_chars(
    textra_paragraph_style_t* paragraph_style, int32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_end_indent_in_chars(
    const textra_paragraph_style_t* paragraph_style, int32_t* out_value);

/* Setting either indent unit clears the other unit for this indent. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_first_line_indent(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_first_line_indent(
    const textra_paragraph_style_t* paragraph_style, float* out_value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_first_line_indent_in_chars(
    textra_paragraph_style_t* paragraph_style, int32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_first_line_indent_in_chars(
    const textra_paragraph_style_t* paragraph_style, int32_t* out_value);

/* Setting either indent unit clears the other unit for this indent. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_hanging_indent(
    textra_paragraph_style_t* paragraph_style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_hanging_indent(
    const textra_paragraph_style_t* paragraph_style, float* out_value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_hanging_indent_in_chars(
    textra_paragraph_style_t* paragraph_style, int32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_hanging_indent_in_chars(
    const textra_paragraph_style_t* paragraph_style, int32_t* out_value);

/* AT_LEAST/EXACT use absolute layout units. AUTO uses a relative factor
 * (1.0 = 100 percent). set_line_height_factor is shorthand for AUTO.
 * Setting either representation replaces the other. get_line_height returns
 * value, rule, and whether value is a relative factor. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_height(
    textra_paragraph_style_t* style, float height,
    textra_line_height_rule_t rule);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_line_height_factor(textra_paragraph_style_t* style,
                                              float factor);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_height(
    const textra_paragraph_style_t* style, float* out_value,
    textra_line_height_rule_t* out_rule, textra_bool_t* out_is_factor);

/* Text/delegate ellipsis setters replace one another. Empty text disables
 * ellipsis. The delegate is retained; copy_ellipsis returns NOT_FOUND when
 * delegate ellipsis is active. get_ellipsis_delegate returns an owned reference
 * or NOT_FOUND with NULL. Setting NULL delegate disables ellipsis. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_set_ellipsis(
    textra_paragraph_style_t* style, const char* utf8, uint32_t byte_count);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_style_copy_ellipsis(
    const textra_paragraph_style_t* style, char* buffer, uint32_t capacity,
    uint32_t* out_size);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_set_ellipsis_delegate(textra_paragraph_style_t* style,
                                             textra_run_delegate_t* delegate);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_get_ellipsis_delegate(
    const textra_paragraph_style_t* style,
    textra_run_delegate_t** out_delegate);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_style_update_punctuation_config(
    textra_paragraph_style_t* style,
    const textra_punctuation_compress_config_t* config);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_STYLE_H_
