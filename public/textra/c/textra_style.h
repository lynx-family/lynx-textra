// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_C_TEXTRA_STYLE_H_
#define PUBLIC_TEXTRA_C_TEXTRA_STYLE_H_

#include <textra/c/textra_types.h>

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_create(textra_style_t** out)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_destroy(textra_style_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_copy(
    const textra_style_t* source, textra_style_t** out) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_reset(textra_style_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_text_size(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_style_set_text_size(textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_text_scale(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_style_set_text_scale(textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_letter_spacing(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_letter_spacing(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_word_spacing(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_word_spacing(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_baseline_offset(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_baseline_offset(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_text_skew(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_style_set_text_skew(textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_style_get_decoration_thickness_multiplier(
    const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_thickness_multiplier(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_style_get_decoration_element_length(
    const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_element_length(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_style_get_decoration_gap_length(
    const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_gap_length(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL textra_style_get_decoration_side_margin(
    const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_side_margin(
    textra_style_t* value, float input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_foreground_color(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_foreground_color(
    textra_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_background_color(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_background_color(
    textra_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_decoration_color(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_color(
    textra_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_bold(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_style_set_bold(textra_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_italic(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL
textra_style_set_italic(textra_style_t* value, uint32_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_decoration_type_t TEXTRA_CALL
textra_style_get_decoration_type(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_type(
    textra_style_t* value, textra_decoration_type_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_line_type_t TEXTRA_CALL
textra_style_get_decoration_style(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_decoration_style(
    textra_style_t* value, textra_line_type_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_character_vertical_alignment_t TEXTRA_CALL
textra_style_get_vertical_alignment(const textra_style_t* value)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_vertical_alignment(
    textra_style_t* value,
    textra_character_vertical_alignment_t input) TEXTRA_NOEXCEPT;

TEXTRA_API textra_word_break_type_t TEXTRA_CALL
textra_style_get_word_break(const textra_style_t* value) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_word_break(
    textra_style_t* value, textra_word_break_type_t input) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_text_stroke_style(
    textra_style_t* style, uint32_t color, float thickness) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_text_stroke_width(const textra_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL
textra_style_get_text_stroke_color(const textra_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API float TEXTRA_CALL
textra_style_get_scaled_text_size(const textra_style_t* style) TEXTRA_NOEXCEPT;

/* Borrowed, read-only. */
TEXTRA_API const textra_font_descriptor_t* TEXTRA_CALL
textra_style_get_font_descriptor(const textra_style_t* style) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_font_descriptor(
    textra_style_t* style,
    const textra_font_descriptor_t* descriptor) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_style_has_attribute(
    const textra_style_t* style, textra_attribute_type_t type) TEXTRA_NOEXCEPT;

TEXTRA_API uint32_t TEXTRA_CALL textra_style_has_style_attribute(
    const textra_style_t* style, uint32_t flag) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_text_shadow_list(
    textra_style_t* style, const textra_text_shadow_t* shadows,
    size_t count) TEXTRA_NOEXCEPT;

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_text_shadow_list(
    const textra_style_t* style, textra_text_shadow_t* shadows, size_t capacity,
    size_t* required_count) TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_set_foreground_gradient(
    textra_style_t* style,
    textra_resolved_gradient_t* gradient) TEXTRA_NOEXCEPT;

/* Owned shared result, or NULL. */
TEXTRA_API textra_resolved_gradient_t* TEXTRA_CALL
textra_style_get_foreground_gradient(const textra_style_t* style)
    TEXTRA_NOEXCEPT;

/* Borrowed immutable singleton. */
TEXTRA_API const textra_style_t* TEXTRA_CALL textra_style_default_style(void)
    TEXTRA_NOEXCEPT;

TEXTRA_API void TEXTRA_CALL textra_style_assign(
    textra_style_t* value, const textra_style_t* source) TEXTRA_NOEXCEPT;

/* Borrowed. */
TEXTRA_API textra_painter_t* TEXTRA_CALL textra_style_get_foreground_painter(
    const textra_style_t* style) TEXTRA_NOEXCEPT;

/* Painter is borrowed and must outlive all users of the style. NULL clears. */
TEXTRA_API void TEXTRA_CALL textra_style_set_foreground_painter(
    textra_style_t* style, textra_painter_t* painter) TEXTRA_NOEXCEPT;

/* Borrowed. */
TEXTRA_API textra_painter_t* TEXTRA_CALL textra_style_get_background_painter(
    const textra_style_t* style) TEXTRA_NOEXCEPT;

/* Painter is borrowed and must outlive all users of the style. NULL clears. */
TEXTRA_API void TEXTRA_CALL textra_style_set_background_painter(
    textra_style_t* style, textra_painter_t* painter) TEXTRA_NOEXCEPT;

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_C_TEXTRA_STYLE_H_
