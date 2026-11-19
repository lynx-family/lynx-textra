// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_STYLE_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_STYLE_H_

#include <textra/capi/textra_font.h>
#include <textra/capi/textra_gradient.h>

/* Style preserves attribute presence. Explicit zero/default values differ
 * from unset. Getters return the effective default when unset; has_attribute
 * reports presence. Reset/unset removes presence. Clone and setters copy values
 * (including descriptors), retaining referenced immutable font/gradient
 * objects. Font size/spacing/offsets are layout units. text_scale is a factor;
 * text_skew is a shear factor. Internal AttributeType values/bit masks are not
 * exported. V1 defaults: size=40/3, scale=1, foreground=opaque black;
 * background and decoration color=transparent black; decoration=NONE/SOLID,
 * thickness=1, element length=2.5, gap=1.5, side margin=2.5; bold/italic=false,
 * alignment=BASELINE, word break=NORMAL, spacing/offset/skew=0, stroke width=1,
 * stroke color=transparent black; no gradient or shadows. Descriptor defaults
 * are documented in textra_font.h. All attributes initially have no presence.
 */
typedef uint32_t textra_style_attribute_t;
#define TEXTRA_STYLE_ATTRIBUTE_FONT_DESCRIPTOR ((textra_style_attribute_t)1)
#define TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE ((textra_style_attribute_t)2)
#define TEXTRA_STYLE_ATTRIBUTE_TEXT_SCALE ((textra_style_attribute_t)3)
#define TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR ((textra_style_attribute_t)4)
#define TEXTRA_STYLE_ATTRIBUTE_BACKGROUND_COLOR ((textra_style_attribute_t)5)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_COLOR ((textra_style_attribute_t)6)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_TYPE ((textra_style_attribute_t)7)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_STYLE ((textra_style_attribute_t)8)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_THICKNESS_MULTIPLIER \
  ((textra_style_attribute_t)9)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_ELEMENT_LENGTH \
  ((textra_style_attribute_t)10)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_GAP_LENGTH \
  ((textra_style_attribute_t)11)
#define TEXTRA_STYLE_ATTRIBUTE_DECORATION_SIDE_MARGIN \
  ((textra_style_attribute_t)12)
#define TEXTRA_STYLE_ATTRIBUTE_BOLD ((textra_style_attribute_t)13)
#define TEXTRA_STYLE_ATTRIBUTE_ITALIC ((textra_style_attribute_t)14)
#define TEXTRA_STYLE_ATTRIBUTE_VERTICAL_ALIGNMENT ((textra_style_attribute_t)15)
#define TEXTRA_STYLE_ATTRIBUTE_WORD_SPACING ((textra_style_attribute_t)16)
#define TEXTRA_STYLE_ATTRIBUTE_LETTER_SPACING ((textra_style_attribute_t)17)
#define TEXTRA_STYLE_ATTRIBUTE_WORD_BREAK ((textra_style_attribute_t)18)
#define TEXTRA_STYLE_ATTRIBUTE_BASELINE_OFFSET ((textra_style_attribute_t)19)
#define TEXTRA_STYLE_ATTRIBUTE_TEXT_SKEW ((textra_style_attribute_t)20)
#define TEXTRA_STYLE_ATTRIBUTE_TEXT_STROKE ((textra_style_attribute_t)21)
#define TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_GRADIENT \
  ((textra_style_attribute_t)22)
#define TEXTRA_STYLE_ATTRIBUTE_TEXT_SHADOWS ((textra_style_attribute_t)23)

/* Frozen shadow record; blur radius is nonnegative layout units. */
typedef struct textra_text_shadow {
  uint32_t argb;
  float offset_x;
  float offset_y;
  double blur_radius;
} textra_text_shadow_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_create(textra_style_t** out_style);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_clone(const textra_style_t* source, textra_style_t** out_style);
TEXTRA_API void TEXTRA_CALL textra_style_destroy(textra_style_t* style);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_reset(textra_style_t* style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_has_attribute(
    const textra_style_t* style, textra_style_attribute_t attribute,
    textra_bool_t* out_is_set);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_unset_attribute(
    textra_style_t* style, textra_style_attribute_t attribute);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_font_descriptor(
    textra_style_t* style, const textra_font_descriptor_t* descriptor);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_font_descriptor(
    const textra_style_t* style, textra_font_descriptor_t** out_descriptor);
/* get_font_descriptor returns an owned copy, never a mutable internal alias. */

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_text_size(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_text_size(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_text_scale(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_text_scale(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_foreground_color(textra_style_t* style, uint32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_foreground_color(
    const textra_style_t* style, uint32_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_background_color(textra_style_t* style, uint32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_background_color(
    const textra_style_t* style, uint32_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_decoration_color(textra_style_t* style, uint32_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_decoration_color(
    const textra_style_t* style, uint32_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_decoration_type(
    textra_style_t* style, textra_decoration_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_decoration_type(
    const textra_style_t* style, textra_decoration_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_decoration_style(
    textra_style_t* style, textra_line_style_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_decoration_style(
    const textra_style_t* style, textra_line_style_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_decoration_thickness_multiplier(textra_style_t* style,
                                                 float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_decoration_thickness_multiplier(const textra_style_t* style,
                                                 float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_decoration_element_length(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_decoration_element_length(const textra_style_t* style,
                                           float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_decoration_gap_length(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_decoration_gap_length(
    const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_decoration_side_margin(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_decoration_side_margin(
    const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_bold(textra_style_t* style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_bold(const textra_style_t* style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_italic(textra_style_t* style, textra_bool_t value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_italic(const textra_style_t* style, textra_bool_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_vertical_alignment(
    textra_style_t* style, textra_character_alignment_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_vertical_alignment(
    const textra_style_t* style, textra_character_alignment_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_word_spacing(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_word_spacing(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_letter_spacing(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_letter_spacing(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_word_break(textra_style_t* style, textra_word_break_t value);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_word_break(
    const textra_style_t* style, textra_word_break_t* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_baseline_offset(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_baseline_offset(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_set_text_skew(textra_style_t* style, float value);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_style_get_text_skew(const textra_style_t* style, float* out_value);

TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_text_stroke(
    textra_style_t* style, uint32_t argb, float thickness);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_text_stroke(
    const textra_style_t* style, uint32_t* out_argb, float* out_thickness);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_foreground_gradient(
    textra_style_t* style, textra_resolved_gradient_t* gradient);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_get_foreground_gradient(
    const textra_style_t* style, textra_resolved_gradient_t** out_gradient);
/* NULL sets an explicit absent gradient; use unset_attribute for inheritance.
 * get_foreground_gradient returns owned reference or NOT_FOUND with NULL. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_set_text_shadows(
    textra_style_t* style, const textra_text_shadow_t* shadows, uint32_t count);
TEXTRA_API textra_status_t TEXTRA_CALL textra_style_copy_text_shadows(
    const textra_style_t* style, textra_text_shadow_t* buffer,
    uint32_t capacity, uint32_t* out_count);
/* Shadow capacity/count are elements; NULL/zero is a successful size query.
 * BUFFER_TOO_SMALL updates out_count and writes no array elements.
 * Arbitrary C++ Painter objects and internal ShapeStyle caches are
 * intentionally not part of the C ABI; use colors/gradients/shadows for
 * portable styling. */

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_STYLE_H_
