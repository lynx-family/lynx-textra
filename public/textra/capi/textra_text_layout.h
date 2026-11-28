// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LAYOUT_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LAYOUT_H_

#include <textra/capi/textra_font.h>
#include <textra/capi/textra_layout_region.h>
#include <textra/capi/textra_paragraph.h>
#include <textra/capi/textra_text_context.h>

typedef struct textra_text_layout_options {
  uint32_t struct_size;
  textra_rendering_backend_t backend; /* 0 = compiled native default. */
  uint32_t flags;                     /* Must be zero. */
  const textra_font_manager_options_t* default_font_manager;
  const textra_font_manager_options_t* asset_font_manager;
  const textra_font_manager_options_t* dynamic_font_manager;
  const textra_font_manager_options_t* test_font_manager;
  const char* default_font_family; /* Optional UTF-8, copied at creation. */
  uint32_t default_font_family_length;
  textra_bool_t disable_font_fallback; /* Zero preserves default fallback. */
} textra_text_layout_options_t;

TEXTRA_EXTERN_C_BEGIN

/* Managers are searched dynamic > asset > test > default. Missing optional
 * slots are skipped. A missing default requests the built-in native manager;
 * callback backends must supply a default. All sources share layout.backend.
 * Configuration/tables are copied. Creation success adopts one reference per
 * configured manager; failure adopts none. Each slot needs its own reference.
 * Managers/fonts remain alive through dependent layout results and caches.
 */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_layout_create(const textra_text_layout_options_t* options,
                          textra_text_layout_t** out_layout);
TEXTRA_API void TEXTRA_CALL
textra_text_layout_destroy(textra_text_layout_t* layout);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_layout_preload(textra_rendering_backend_t backend);
/* Clears the layout's font-manager lookup cache. */
TEXTRA_API textra_status_t TEXTRA_CALL
textra_text_layout_clear_font_cache(textra_text_layout_t* layout);

/* Starts at a reset context or resumes its opaque continuation state. Layout
 * appends lines to the region and retains dependencies. It never draws.
 * out_flow is required and receives layout control flow on OK only; unsupported
 * content returns UNSUPPORTED as an API status, not a successful flow value.
 * A context with active continuation is bound to the same engine/paragraph.
 * NORMAL means the paragraph completed; other flows require continuation or
 * caller-directed retry. Resuming into a different/cleared region starts at
 * bottom=0 unless set_layout_bottom explicitly overrides it for that call.
 * A fresh paragraph requires a reset context.
 */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_layout_layout(
    textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_layout_region_t* region, textra_text_context_t* context,
    textra_layout_flow_t* out_flow);

/* Only SINGLE_LINE paragraphs, unwrapped one-style LTR supported content.
 * Replaces the paragraph's previous single-line result and retains required
 * fonts/engine state until invalidation/destruction. Successful mutation
 * invalidates it. Unsupported operations leave the paragraph unchanged.
 * Draw/get_metrics require a successful, still-valid result. */
TEXTRA_API textra_status_t TEXTRA_CALL textra_text_layout_layout_single_line(
    textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_text_context_t* context);

TEXTRA_EXTERN_C_END

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_TEXT_LAYOUT_H_
