// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_DRAWER_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_DRAWER_H_

#include <textra/capi/textra_paragraph.h>
#include <textra/capi/textra_platform.h>
#include <textra/capi/textra_text_line.h>

typedef uint8_t textra_theme_category_t;
#define TEXTRA_THEME_CATEGORY_NORMAL ((textra_theme_category_t)0)
#define TEXTRA_THEME_CATEGORY_LINK ((textra_theme_category_t)1)
#define TEXTRA_THEME_CATEGORY_BACKGROUND ((textra_theme_category_t)2)
#define TEXTRA_THEME_CATEGORY_BORDER ((textra_theme_category_t)3)
#define TEXTRA_THEME_CATEGORY_FOOTNOTE ((textra_theme_category_t)4)

/* Optional table, copied at create; version=1. NULL resolve_color preserves
 * original_argb. Synchronous/non-reentrant callbacks must not throw. Creation
 * success adopts or borrows userdata per the common callback ownership rules.
 * Final drawer destruction invokes destroy once if present. Failure leaves
 * userdata with the caller.
 */
typedef struct textra_layout_drawer_callbacks {
  uint32_t struct_size;
  uint32_t version;
  textra_status_t(TEXTRA_CALL* resolve_color)(void* userdata,
                                              textra_theme_category_t category,
                                              uint32_t original_argb,
                                              uint32_t* out_argb);
  void(TEXTRA_CALL* destroy)(void* userdata);
} textra_layout_drawer_callbacks_t;

typedef struct textra_layout_drawer_options {
  uint32_t struct_size;
  const textra_canvas_options_t*
      canvas; /* Required; copied/adopted on success. */
  const textra_layout_drawer_callbacks_t* callbacks;
  void* userdata;
} textra_layout_drawer_options_t;

TEXTRA_EXTERN_C_BEGIN
TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout_drawer_create(const textra_layout_drawer_options_t* options,
                            textra_layout_drawer_t** out_drawer);
TEXTRA_API void TEXTRA_CALL
textra_layout_drawer_destroy(textra_layout_drawer_t* drawer);
/* Drawing does not mutate layout coordinates. Backend compatibility and known
 * required drawing capabilities are checked before emitting commands;
 * missing capabilities return UNSUPPORTED, incompatible resource backends
 * return BACKEND_MISMATCH. No silent no-op fallback for unsupported effects.
 * Backend errors inside C++ drawing methods are not translated into the
 * returned status and can leave partial pixels/commands; drawing is not rolled
 * back. Repeated successful draws preserve results. Regions/paragraphs are
 * borrowed for each call; the caller must hold a reference until it returns.
 */
TEXTRA_API textra_status_t TEXTRA_CALL textra_layout_drawer_draw_region(
    textra_layout_drawer_t* drawer, const textra_layout_region_t* region);
/* Range is line-relative codepoint [start,end); NULL draws the whole line. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_layout_drawer_draw_line(
    textra_layout_drawer_t* drawer, const textra_layout_region_t* region,
    uint32_t line_index, const textra_text_range_t* range);
/* x/y identify the single-line layout-box top, not its baseline. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_layout_drawer_draw_single_line(
    textra_layout_drawer_t* drawer, const textra_paragraph_t* paragraph,
    float x, float y);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_LAYOUT_DRAWER_H_
