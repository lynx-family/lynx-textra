// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_CAPI_TEXTRA_CANVAS_H_
#define PUBLIC_TEXTRA_CAPI_TEXTRA_CANVAS_H_

#include <textra/capi/textra_types.h>

/* Canvas callback table and call-borrowed drawing views. Callback table
 * ownership and versioning follow textra_backend.h. All non-table views are
 * frozen V1 records; initialize struct_size and unused fields before calls. */

typedef struct textra_backend_gradient_view {
  uint32_t struct_size;
  textra_gradient_type_t type;
  textra_rect_t bounds; /* left/top/width/height */
  textra_point_t start, end, center;
  float radius_x, radius_y, start_angle;
  const uint32_t* colors;
  const float* positions;
  uint32_t stop_count;
} textra_backend_gradient_view_t;
typedef struct textra_backend_shadow {
  uint32_t color;
  float offset_x, offset_y;
  double blur_radius;
} textra_backend_shadow_t;
typedef struct textra_backend_paint {
  uint32_t struct_size;
  uint32_t fill_color, stroke_color;
  float stroke_width, stroke_miter, text_size, text_skew;
  const char* font_family;
  uint32_t bold, italic, underline, cap, join;
  const textra_backend_shadow_t* shadows;
  uint32_t shadow_count;
  const textra_backend_gradient_view_t* gradient;
} textra_backend_paint_t;
/* Fixed-width path kind values, matching the existing C ABI. */
typedef uint32_t textra_path_kind_t;
#define TEXTRA_PATH_KIND_LINES ((textra_path_kind_t)0)
#define TEXTRA_PATH_KIND_ARC ((textra_path_kind_t)1)
#define TEXTRA_PATH_KIND_BEZIER ((textra_path_kind_t)2)
#define TEXTRA_PATH_KIND_MOVE_TO ((textra_path_kind_t)3)
#define TEXTRA_PATH_KIND_MULTI_PATH ((textra_path_kind_t)4)

/* Arc paths use start/mid/end points plus radius. All nodes and point arrays
 * are call-borrowed. */
typedef struct textra_backend_path {
  uint32_t struct_size;
  textra_path_kind_t kind;
  const textra_point_t* points;
  uint32_t point_count;
  float radius;
  const struct textra_backend_path* children;
  uint32_t child_count;
} textra_backend_path_t;
/* Every callback corresponds to one ICanvasHelper method. Object/paint/path/
 * text/array arguments are borrowed for the synchronous call. Typeface means
 * the provider-owned opaque font object. Missing callbacks follow the no-op I*
 * behavior. Append optional callbacks at the end; never reorder published
 * fields. CreatePainter remains an engine-side value object; paint views carry
 * its resolved properties to each drawing callback. Inline-object drawing uses
 * textra_run_callbacks_t.draw independently of this Canvas callback table. */
#define TEXTRA_CANVAS_API_VERSION ((uint32_t)2)

typedef struct textra_backend_canvas_api {
  uint32_t struct_size, version;
  void(TEXTRA_CALL* destroy)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* start_paint)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* end_paint)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* save)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* restore)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* translate)(void* userdata, void* canvas,
                                          float dx, float dy);
  textra_status_t(TEXTRA_CALL* scale)(void* userdata, void* canvas, float sx,
                                      float sy);
  textra_status_t(TEXTRA_CALL* rotate)(void* userdata, void* canvas,
                                       float degrees);
  textra_status_t(TEXTRA_CALL* skew)(void* userdata, void* canvas, float sx,
                                     float sy);
  textra_status_t(TEXTRA_CALL* clip_rect)(void* userdata, void* canvas,
                                          float left, float top, float right,
                                          float bottom,
                                          textra_bool_t antialias);
  textra_status_t(TEXTRA_CALL* clear)(void* userdata, void* canvas);
  textra_status_t(TEXTRA_CALL* clear_rect)(void* userdata, void* canvas,
                                           float left, float top, float right,
                                           float bottom);
  textra_status_t(TEXTRA_CALL* fill_rect)(void* userdata, void* canvas,
                                          float left, float top, float right,
                                          float bottom, uint32_t color);
  textra_status_t(TEXTRA_CALL* draw_color)(void* userdata, void* canvas,
                                           uint32_t color);
  textra_status_t(TEXTRA_CALL* draw_line)(void* userdata, void* canvas,
                                          float x1, float y1, float x2,
                                          float y2,
                                          const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_rect)(void* userdata, void* canvas,
                                          float left, float top, float right,
                                          float bottom,
                                          const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_oval)(void* userdata, void* canvas,
                                          float left, float top, float right,
                                          float bottom,
                                          const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_circle)(
      void* userdata, void* canvas, float x, float y, float radius,
      const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_arc)(void* userdata, void* canvas,
                                         float left, float top, float right,
                                         float bottom, float start_angle,
                                         float sweep_angle,
                                         textra_bool_t use_center,
                                         const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_path)(void* userdata, void* canvas,
                                          const textra_backend_path_t* path,
                                          const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_arc_to)(
      void* userdata, void* canvas, float start_x, float start_y, float mid_x,
      float mid_y, float end_x, float end_y, float radius,
      const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_text)(void* userdata, void* canvas,
                                          void* typeface, const char* text,
                                          uint32_t text_bytes, float x, float y,
                                          const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_glyphs)(
      void* userdata, void* canvas, void* typeface, uint32_t glyph_count,
      const uint16_t* glyphs, const char* text, uint32_t text_bytes,
      float origin_x, float origin_y, float* x, float* y,
      const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_image)(void* userdata, void* canvas,
                                           const char* source, float left,
                                           float top, float right, float bottom,
                                           const textra_backend_paint_t* paint);
  textra_status_t(TEXTRA_CALL* draw_image_rect)(
      void* userdata, void* canvas, const char* source, float src_left,
      float src_top, float src_right, float src_bottom, float dst_left,
      float dst_top, float dst_right, float dst_bottom,
      const textra_backend_paint_t* paint, textra_bool_t source_rect_percent);
  textra_status_t(TEXTRA_CALL* draw_round_rect)(
      void* userdata, void* canvas, float left, float top, float right,
      float bottom, float radius, const textra_backend_paint_t* paint);
} textra_backend_canvas_api_t;

#endif  // PUBLIC_TEXTRA_CAPI_TEXTRA_CANVAS_H_
