// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_H_

#include <textra/capi/textra_paragraph_style.h>
#include <textra/capi/textra_run_delegate.h>

/* Paragraphs copy UTF-8/style input and retain delegate/typeface resources.
 * A region retains every paragraph it references. Editing or relaying out such
 * a paragraph returns BUSY until all referring regions are cleared/destroyed
 * and any active continuation context is reset or completes.
 * Destroying the caller's paragraph reference is allowed during that time.
 * Queries returning styles produce owned copies, never mutable internal views.
 * Range editing supports paint attributes. Font descriptor, size, scale,
 * vertical alignment, word/letter spacing edits return UNSUPPORTED because the
 * underlying C++ range editor cannot reshape them. Rebuild a paragraph instead.
 *
 * SINGLE_LINE accepts one nonempty text run and layout_single_line only.
 * Shape runs, paragraph-style access, range editing and save/restore style
 * return UNSUPPORTED before entering C++ and leave the object unchanged.
 * A second nonempty text append returns UNSUPPORTED. General layout rejects
 * SINGLE_LINE; no implicit promotion to GENERAL occurs.
 */
typedef struct textra_paragraph_options {
  uint32_t struct_size;
  textra_paragraph_type_t type; /* Default GENERAL. */
} textra_paragraph_options_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_create(const textra_paragraph_options_t* options,
                        textra_paragraph_t** out_paragraph);
TEXTRA_API void TEXTRA_CALL
textra_paragraph_destroy(textra_paragraph_t* paragraph);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_type(
    const textra_paragraph_t* paragraph, textra_paragraph_type_t* out_type);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_set_paragraph_style(
    textra_paragraph_t* paragraph, const textra_paragraph_style_t* style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_paragraph_style(
    const textra_paragraph_t* paragraph, textra_paragraph_style_t** out_style);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_add_text_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    const char* utf8, uint32_t byte_count);
/* NULL style selects the paragraph default style. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_add_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate, textra_bool_t is_float);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_add_ghost_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_apply_style(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    const textra_text_range_t* range);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_save_style(textra_paragraph_t* paragraph);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_restore_style(textra_paragraph_t* paragraph);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_char_count(
    const textra_paragraph_t* paragraph, uint32_t* out_count);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_run_count(
    const textra_paragraph_t* paragraph, uint32_t* out_count);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_copy_text(
    const textra_paragraph_t* paragraph, const textra_text_range_t* range,
    char* buffer, uint32_t capacity, uint32_t* out_size);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_query_style(const textra_paragraph_t* paragraph,
                             uint32_t char_index, textra_style_t** out_style);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_paragraph_get_metrics(const textra_paragraph_t* paragraph,
                             textra_paragraph_metrics_t* out_metrics);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_intrinsic_widths(
    const textra_paragraph_t* paragraph, float* out_min, float* out_max);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_word_boundary(
    const textra_paragraph_t* paragraph, uint32_t char_index,
    textra_text_range_t* out_range);
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_grapheme_boundary(
    const textra_paragraph_t* paragraph, uint32_t char_index,
    textra_text_range_t* out_range);
/* Boundary query out of range returns OUT_OF_RANGE. Word boundary requires
 * prior layout; it is not ideographic word segmentation. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_paragraph_get_resolved_direction(
    const textra_paragraph_t* paragraph,
    textra_write_direction_t* out_direction);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_PARAGRAPH_H_
