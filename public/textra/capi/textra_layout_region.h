// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_REGION_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_REGION_H_

#include <textra/capi/textra_layout_page_listener.h>

/* Region owns lines and retains all contributing paragraphs, engine resources
 * and delegates. It prevents paragraph edits and fresh layout of an already
 * referenced paragraph. Appending continuation with the same engine/context
 * is the only permitted layout of that paragraph while regions reference it.
 * Clear drops lines/dependency references and resets measured size/overflow,
 * retaining dimensions, line pitch and callbacks. A continuation context can
 * still retain/lock its paragraph until reset or completion.
 *
 * Layout errors may leave partial lines in a region and advance its context;
 * after failure clear the region and reset the context before reuse. Previously
 * returned metric copies remain ordinary values. No borrowed TextLine objects
 * or mutable C++ region internals are exposed.
 */
typedef struct textra_layout_region_options {
  uint32_t struct_size;
  float width;                      /* Default 0; finite, nonnegative. */
  float height;                     /* Default 0; finite, nonnegative. */
  textra_layout_mode_t width_mode;  /* Default INDEFINITE. */
  textra_layout_mode_t height_mode; /* Default INDEFINITE. */
  float line_pitch;                 /* Default 0 (no grid). */
  const textra_layout_page_callbacks_t* callbacks;
  void* userdata;
} textra_layout_region_options_t;

typedef struct textra_layout_region_metrics {
  uint32_t struct_size;
  float width;
  float height;
  uint32_t line_count;
  textra_bool_t full;
  textra_bool_t exceeded_max_lines;
} textra_layout_region_metrics_t;

TEXTRA_EXTERN_C_BEGIN

TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout_region_create(const textra_layout_region_options_t* options,
                            textra_layout_region_t** out_region);
TEXTRA_API void TEXTRA_CALL
textra_layout_region_destroy(textra_layout_region_t* region);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout_region_clear(textra_layout_region_t* region);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout_region_get_metrics(const textra_layout_region_t* region,
                                 textra_layout_region_metrics_t* out_metrics);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_REGION_H_
