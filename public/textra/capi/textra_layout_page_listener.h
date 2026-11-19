// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_PAGE_LISTENER_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_PAGE_LISTENER_H_

#include <textra/capi/textra_types.h>

/* Frozen horizontal interval record in region coordinates. */
typedef struct textra_horizontal_range {
  float start;
  float end;
} textra_horizontal_range_t;

/* A library-provided writer used ONLY during get_ranges. Intervals must be
 * finite, ordered, disjoint and have start < end. Writer copies each array
 * during the call; a failed write must be propagated by get_ranges. */
typedef textra_status_t(TEXTRA_CALL* textra_range_writer_t)(
    void* writer_context, const textra_horizontal_range_t* ranges,
    uint32_t count);

typedef struct textra_line_layout_event {
  uint32_t struct_size;
  uint32_t line_index;
  textra_bool_t last_line_in_paragraph;
  float available_height;
} textra_line_layout_event_t;

/* Optional callbacks copied by region_create. version=1. NULL table means no
 * callbacks. Each NULL callback uses default rectangular/normal-flow behavior.
 * userdata follows the common callback ownership rules; destroy runs exactly
 * once if present on final region destruction. clear preserves callbacks/data.
 * Calls are synchronous on the layout thread, non-throwing, without reentrant
 * mutation. region is borrowed and allows read-only queries of appended lines.
 * get_ranges may invoke the supplied writer, but no other layout operations.
 * Input top is proposed line top; output top must be finite and >= input top.
 * No emitted ranges means no horizontal space at that top. on_line initializes
 * inout_flow to NORMAL; callback may set a defined flow value, independently of
 * its returned API status. Unknown flow values return UNSUPPORTED.
 */
typedef struct textra_layout_page_callbacks {
  uint32_t struct_size;
  uint32_t version;
  textra_status_t(TEXTRA_CALL* on_line)(void* userdata,
                                        const textra_layout_region_t* region,
                                        const textra_line_layout_event_t* event,
                                        textra_layout_flow_t* inout_flow);
  textra_status_t(TEXTRA_CALL* get_ranges)(void* userdata, float* inout_top,
                                           float height, float start, float end,
                                           textra_range_writer_t writer,
                                           void* writer_context);
  void(TEXTRA_CALL* destroy)(void* userdata);
} textra_layout_page_callbacks_t;

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_PAGE_LISTENER_H_
