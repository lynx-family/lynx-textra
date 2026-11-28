// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_CANVAS_IMPL_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_CANVAS_IMPL_H_

#include <textra/capi/platform/skity_common/textra_skity_paint_impl.h>
#include <textra/capi/platform/skity_common/textra_skity_typeface_impl.h>

TEXTRA_EXTERN_C_BEGIN
static inline void TEXTRA_CALL textra_skity_impl_destroy_canvas(void* userdata,
                                                                void* canvas) {
  (void)userdata;
  (void)canvas; /* Canvas is borrowed. */
}
static inline textra_status_t textra_skity_impl_append_path(
    skity_path native_path, const textra_backend_path_t* path) {
  if (!path) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_point_t* points = path->points;
  switch (path->kind) {
    case TEXTRA_PATH_KIND_LINES:
      for (uint32_t i = 0; i < path->point_count; ++i)
        skity_path_line_to(native_path, points[i].x, points[i].y);
      break;
    case TEXTRA_PATH_KIND_ARC:
      if (path->point_count != 3) return TEXTRA_STATUS_INVALID_ARGUMENT;
      skity_path_line_to(native_path, points[0].x, points[0].y);
      skity_path_arc_to(native_path, points[1].x, points[1].y, points[2].x,
                        points[2].y, path->radius);
      break;
    case TEXTRA_PATH_KIND_BEZIER:
      if (path->point_count < 3 || path->point_count > 4)
        return TEXTRA_STATUS_UNSUPPORTED;
      skity_path_line_to(native_path, points[0].x, points[0].y);
      if (path->point_count == 3)
        skity_path_quad_to(native_path, points[1].x, points[1].y, points[2].x,
                           points[2].y);
      else
        skity_path_cubic_to(native_path, points[1].x, points[1].y, points[2].x,
                            points[2].y, points[3].x, points[3].y);
      break;
    case TEXTRA_PATH_KIND_MOVE_TO:
      if (path->point_count != 1) return TEXTRA_STATUS_INVALID_ARGUMENT;
      skity_path_move_to(native_path, points[0].x, points[0].y);
      break;
    case TEXTRA_PATH_KIND_MULTI_PATH:
      for (uint32_t i = 0; i < path->child_count; ++i) {
        textra_status_t status =
            textra_skity_impl_append_path(native_path, path->children + i);
        if (status) return status;
      }
      break;
    default:
      return TEXTRA_STATUS_UNSUPPORTED;
  }
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_start_paint(void* userdata, void* object) {
  (void)userdata;
  (void)object;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_end_paint(void* userdata, void* object) {
  (void)userdata;
  (void)object;
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_save(void* userdata,
                                                                 void* object) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_save(canvas);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_restore(void* userdata, void* object) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_restore(canvas);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_translate(void* userdata, void* object, float dx, float dy) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_translate(canvas, dx, dy);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_scale(void* userdata, void* object, float sx, float sy) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_scale(canvas, sx, sy);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_rotate(void* userdata, void* object, float degrees) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_rotate_deg(canvas, degrees, 0, 0);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_skew(void* userdata,
                                                                 void* object,
                                                                 float sx,
                                                                 float sy) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_skew(canvas, sx, sy);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_clip_rect(
    void* userdata, void* object, float left, float top, float right,
    float bottom, textra_bool_t antialias) {
  (void)userdata;
  (void)antialias;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_clip_rect(canvas, &rect, SKITY_CLIP_OP_INTERSECT);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_clear(void* userdata, void* object) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_draw_color(canvas, 0, SKITY_BLEND_MODE_CLEAR);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_clear_rect(void* userdata, void* object, float left,
                             float top, float right, float bottom) {
  (void)userdata;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_save(canvas);
  skity_canvas_clip_rect(canvas, &rect, SKITY_CLIP_OP_INTERSECT);
  skity_canvas_draw_color(canvas, 0, SKITY_BLEND_MODE_CLEAR);
  skity_canvas_restore(canvas);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_fill_rect(void* userdata, void* object, float left, float top,
                            float right, float bottom, uint32_t color) {
  (void)userdata;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  skity_paint fill = skity_paint_create();
  if (!fill) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_paint_set_color(fill, color);
  skity_canvas_draw_rect(canvas, &rect, fill);
  skity_paint_destroy(fill);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_draw_color(void* userdata, void* object, uint32_t color) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  skity_canvas_draw_color(canvas, color, SKITY_BLEND_MODE_SRC_OVER);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_line(
    void* userdata, void* object, float x1, float y1, float x2, float y2,
    const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  if (paint_options->gradient) {
    skity_paint_set_shader(native_paint, NULL);
    skity_paint_set_color(native_paint, paint_options->stroke_color);
  }
  skity_canvas_draw_line(canvas, x1, y1, x2, y2, native_paint);
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_rect(
    void* userdata, void* object, float left, float top, float right,
    float bottom, const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  {
    int stroke = 0;
    do {
      skity_canvas_draw_rect(canvas, &rect, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_oval(
    void* userdata, void* object, float left, float top, float right,
    float bottom, const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  {
    int stroke = 0;
    do {
      skity_canvas_draw_oval(canvas, &rect, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_circle(
    void* userdata, void* object, float x, float y, float radius,
    const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  {
    int stroke = 0;
    do {
      skity_canvas_draw_circle(canvas, x, y, radius, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_arc(
    void* userdata, void* object, float left, float top, float right,
    float bottom, float start_angle, float sweep_angle,
    textra_bool_t use_center, const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_rect rect = {left, top, right, bottom};
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  {
    int stroke = 0;
    do {
      skity_canvas_draw_arc(canvas, &rect, start_angle, sweep_angle,
                            use_center != TEXTRA_FALSE, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_path(
    void* userdata, void* object, const textra_backend_path_t* path,
    const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_path native_path = skity_path_create();
  if (!native_path) {
    skity_paint_destroy(native_paint);
    return TEXTRA_STATUS_OUT_OF_MEMORY;
  }
  textra_status_t status = textra_skity_impl_append_path(native_path, path);
  if (status) {
    skity_path_destroy(native_path);
    skity_paint_destroy(native_paint);
    return status;
  }
  {
    int stroke = 0;
    do {
      skity_canvas_draw_path(canvas, native_path, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_path_destroy(native_path);
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_arc_to(
    void* userdata, void* object, float start_x, float start_y, float mid_x,
    float mid_y, float end_x, float end_y, float radius,
    const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  skity_path native_path = skity_path_create();
  if (!native_path) {
    skity_paint_destroy(native_paint);
    return TEXTRA_STATUS_OUT_OF_MEMORY;
  }
  skity_path_move_to(native_path, start_x, start_y);
  skity_path_arc_to(native_path, mid_x, mid_y, end_x, end_y, radius);
  {
    int stroke = 0;
    do {
      skity_canvas_draw_path(canvas, native_path, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_path_destroy(native_path);
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL
textra_skity_impl_draw_text(void* userdata, void* object, void* typeface,
                            const char* text, uint32_t text_bytes, float x,
                            float y, const textra_backend_paint_t* paint) {
  (void)userdata;
  (void)object;
  (void)typeface;
  (void)text;
  (void)text_bytes;
  (void)x;
  (void)y;
  (void)paint;
  return TEXTRA_STATUS_UNSUPPORTED;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_image(
    void* userdata, void* object, const char* source, float left, float top,
    float right, float bottom, const textra_backend_paint_t* paint) {
  (void)userdata;
  (void)object;
  (void)source;
  (void)left;
  (void)top;
  (void)right;
  (void)bottom;
  (void)paint;
  return TEXTRA_STATUS_UNSUPPORTED;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_image_rect(
    void* userdata, void* object, const char* source, float src_left,
    float src_top, float src_right, float src_bottom, float dst_left,
    float dst_top, float dst_right, float dst_bottom,
    const textra_backend_paint_t* paint, textra_bool_t source_rect_percent) {
  (void)userdata;
  (void)object;
  (void)source;
  (void)src_left;
  (void)src_top;
  (void)src_right;
  (void)src_bottom;
  (void)dst_left;
  (void)dst_top;
  (void)dst_right;
  (void)dst_bottom;
  (void)paint;
  (void)source_rect_percent;
  return TEXTRA_STATUS_UNSUPPORTED;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_round_rect(
    void* userdata, void* object, float left, float top, float right,
    float bottom, float radius, const textra_backend_paint_t* paint) {
  (void)userdata;
  skity_canvas canvas = (skity_canvas)(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const textra_backend_paint_t* paint_options = paint;
  skity_paint native_paint = textra_skity_impl_make_paint(paint_options);
  if (!native_paint) return TEXTRA_STATUS_OUT_OF_MEMORY;
  {
    int stroke = 0;
    do {
      skity_rect rect = {left, top, right, bottom};
      skity_canvas_draw_round_rect(canvas, &rect, radius, radius, native_paint);
    } while (!stroke++ &&
             textra_skity_impl_stroke_pass(paint_options, native_paint));
  }
  skity_paint_destroy(native_paint);
  return TEXTRA_STATUS_OK;
}
static inline textra_status_t TEXTRA_CALL textra_skity_impl_draw_glyphs(
    void* userdata, void* object, void* typeface, uint32_t glyph_count,
    const uint16_t* glyphs, const char* text, uint32_t text_bytes,
    float origin_x, float origin_y, float* positions_x, float* glyph_y,
    const textra_backend_paint_t* paint_options) {
  skity_canvas canvas = (skity_canvas)object;
  skity_font font;
  skity_paint native_paint;
  skity_text_blob blob;
  float* positions_y;
  uint32_t i;
  int stroke = 0;
  textra_status_t status = TEXTRA_STATUS_OK;
  (void)userdata;
  (void)text;
  (void)text_bytes;
  if (!paint_options || !typeface) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!glyph_count) return TEXTRA_STATUS_OK;
  if (!glyphs || !positions_x || !glyph_y)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  positions_y = (float*)textra_skity_impl_allocate(glyph_count, sizeof(float));
  if (!positions_y) return TEXTRA_STATUS_OUT_OF_MEMORY;
  native_paint = textra_skity_impl_make_paint(paint_options);
  font =
      textra_skity_impl_make_typeface_font(typeface, paint_options->text_size);
  if (!native_paint || !font) {
    if (native_paint) skity_paint_destroy(native_paint);
    if (font) skity_font_destroy(font);
    free(positions_y);
    return TEXTRA_STATUS_OUT_OF_MEMORY;
  }
  skity_font_set_subpixel(font, TEXTRA_TRUE);
  skity_font_set_baseline_snap(font, TEXTRA_TRUE);
  skity_font_set_embolden(font, paint_options->bold != TEXTRA_FALSE);
  skity_font_set_skew_x(
      font, (paint_options->italic ? -0.25f : 0) + paint_options->text_skew);
  for (i = 0; i < glyph_count; ++i) positions_y[i] = -glyph_y[i];
  blob = skity_text_blob_create_from_glyphs(font, glyphs, positions_x,
                                            positions_y, glyph_count);
  free(positions_y);
  if (!blob) {
    status = TEXTRA_STATUS_OUT_OF_MEMORY;
    goto done;
  }
  for (i = 0; i < paint_options->shadow_count; ++i) {
    const textra_backend_shadow_t* value = paint_options->shadows + i;
    skity_paint shadow = skity_paint_clone(native_paint);
    skity_mask_filter filter = NULL;
    if (!shadow) {
      status = TEXTRA_STATUS_OUT_OF_MEMORY;
      goto done;
    }
    if (value->blur_radius) {
      filter = skity_mask_filter_create_blur(SKITY_BLUR_STYLE_NORMAL,
                                             (float)value->blur_radius);
      if (!filter) {
        skity_paint_destroy(shadow);
        status = TEXTRA_STATUS_OUT_OF_MEMORY;
        goto done;
      }
    }
    skity_paint_set_shader(shadow, NULL);
    skity_paint_set_color(shadow, value->color);
    skity_paint_set_mask_filter(shadow, filter);
    skity_canvas_draw_text_blob(canvas, blob, origin_x + value->offset_x,
                                origin_y + value->offset_y, shadow);
    if (filter) skity_mask_filter_destroy(filter);
    skity_paint_destroy(shadow);
  }
  do {
    skity_canvas_draw_text_blob(canvas, blob, origin_x, origin_y, native_paint);
  } while (!stroke++ &&
           textra_skity_impl_stroke_pass(paint_options, native_paint));
done:
  if (blob) skity_text_blob_destroy(blob);
  skity_font_destroy(font);
  skity_paint_destroy(native_paint);
  return status;
}
static const textra_backend_canvas_api_t textra_skity_impl_canvas_api = {
    sizeof(textra_backend_canvas_api_t),
    TEXTRA_CANVAS_API_VERSION,
    textra_skity_impl_destroy_canvas,
    textra_skity_impl_start_paint,
    textra_skity_impl_end_paint,
    textra_skity_impl_save,
    textra_skity_impl_restore,
    textra_skity_impl_translate,
    textra_skity_impl_scale,
    textra_skity_impl_rotate,
    textra_skity_impl_skew,
    textra_skity_impl_clip_rect,
    textra_skity_impl_clear,
    textra_skity_impl_clear_rect,
    textra_skity_impl_fill_rect,
    textra_skity_impl_draw_color,
    textra_skity_impl_draw_line,
    textra_skity_impl_draw_rect,
    textra_skity_impl_draw_oval,
    textra_skity_impl_draw_circle,
    textra_skity_impl_draw_arc,
    textra_skity_impl_draw_path,
    textra_skity_impl_draw_arc_to,
    textra_skity_impl_draw_text,
    textra_skity_impl_draw_glyphs,
    textra_skity_impl_draw_image,
    textra_skity_impl_draw_image_rect,
    textra_skity_impl_draw_round_rect};
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_CANVAS_IMPL_H_
