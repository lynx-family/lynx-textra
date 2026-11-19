// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_TYPEFACE_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_TYPEFACE_H_

#include <textra/capi/textra_types.h>

/* Typeface callbacks used by the engine. Callback table ownership
 * and versioning follow textra_backend.h. Font data/native pointers returned
 * by getters are borrowed for the typeface lifetime. Non-table views are frozen
 * V1 records; initialize struct_size and all unused fields before calls. */

typedef struct textra_backend_font_metrics {
  uint32_t struct_size;
  float top, ascent, descent, bottom, leading, font_size;
} textra_backend_font_metrics_t;
typedef struct textra_backend_em_height {
  uint32_t struct_size;
  float ascent, descent;
} textra_backend_em_height_t;
#define TEXTRA_TYPEFACE_API_VERSION ((uint32_t)2)

/* One callback per ITypefaceHelper method; font_data and font_data_size
 * are independent. GetUniqueId is engine-owned and is not a provider callback.
 */
typedef struct textra_backend_typeface_api {
  uint32_t struct_size, version;
  void(TEXTRA_CALL* destroy)(void* userdata, void* typeface);
  textra_status_t(TEXTRA_CALL* horizontal_advance)(void*, void*, uint16_t,
                                                   float, float*);
  textra_status_t(TEXTRA_CALL* horizontal_advances)(void*, void*,
                                                    const uint16_t*, uint32_t,
                                                    float, float*);
  /* bound = left/top/width/height; bounds = left/top/right/bottom. */
  textra_status_t(TEXTRA_CALL* width_bound)(void*, void*, uint16_t, float,
                                            textra_rect_t*);
  textra_status_t(TEXTRA_CALL* width_bounds)(void*, void*, const uint16_t*,
                                             uint32_t, float, textra_rect_t*);
  textra_status_t(TEXTRA_CALL* font_data)(void*, void*, const void**);
  textra_status_t(TEXTRA_CALL* font_data_size)(void*, void*, uint64_t*);
  textra_status_t(TEXTRA_CALL* font_index)(void*, void*, int32_t*);
  textra_status_t(TEXTRA_CALL* unichar_to_glyph)(void*, void*, uint32_t,
                                                 uint32_t, uint16_t*);
  textra_status_t(TEXTRA_CALL* unichars_to_glyphs)(void*, void*,
                                                   const uint32_t*, uint32_t,
                                                   uint16_t*);
  textra_status_t(TEXTRA_CALL* units_per_em)(void*, void*, uint32_t*);
  textra_status_t(TEXTRA_CALL* font_info)(void*, void*, float,
                                          textra_backend_font_metrics_t*);
  /* Optional: defaults to -font_info.ascent / font_info.descent. */
  textra_status_t(TEXTRA_CALL* em_height)(void*, void*, float,
                                          textra_backend_em_height_t*);
  textra_status_t(TEXTRA_CALL* font_style)(void*, void*, textra_font_style_t*);
  textra_status_t(TEXTRA_CALL* native_font)(void*, void*, const void**);
} textra_backend_typeface_api_t;

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_TYPEFACE_H_
