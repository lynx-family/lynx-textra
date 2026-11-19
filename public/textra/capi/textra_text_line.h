// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LINE_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LINE_H_

#include <textra/capi/textra_layout_region.h>

/* Lines are addressed by region/index; no standalone borrowed line handles.
 * Metrics are snapshots. Character ranges are paragraph-relative codepoints.
 * Content ascent/descent are nonnegative. top/baseline/bottom/left/right are
 * region coordinates; content_top/bottom are offsets from line top.
 */
typedef struct textra_text_line_metrics {
  uint32_t struct_size;
  uint32_t start_char;
  uint32_t end_char;
  float top;
  float baseline;
  float bottom;
  float left;
  float right;
  float content_top;
  float content_bottom;
  float ascent;
  float descent;
  float start_indent;
  float end_indent;
  textra_bool_t last_line_in_paragraph;
} textra_text_line_metrics_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL textra_text_line_get_metrics(
    const textra_layout_region_t* region, uint32_t line_index,
    textra_text_line_metrics_t* out_metrics);
/* Returns an owned reference to the contributing paragraph. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_line_get_paragraph(
    const textra_layout_region_t* region, uint32_t line_index,
    textra_paragraph_t** out_paragraph);
/* NULL range selects the full line; otherwise it must lie within that line.
 * Rectangles always use left/top/right/bottom in region coordinates, including
 * tight bounds (the adapter converts internal baseline-relative signed values).
 * Range bounding returns the enclosing rectangle, not disjoint bidi fragments.
 */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_line_get_bounds(
    const textra_layout_region_t* region, uint32_t line_index,
    const textra_text_range_t* range, textra_bool_t tight,
    textra_rect_t* out_bounds);
/* x is region-relative; output is paragraph-relative, clamped to the line's
 * [start_char, end_char] insertion positions. This is codepoint hit testing,
 * not a promise of grapheme-safe caret placement or bidi affinity. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_line_hit_test(
    const textra_layout_region_t* region, uint32_t line_index, float x,
    uint32_t* out_char_index);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LINE_H_
