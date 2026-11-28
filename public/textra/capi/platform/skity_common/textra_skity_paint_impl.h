// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_PAINT_IMPL_H_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_PAINT_IMPL_H_

#include <textra/capi/platform/skity_common/textra_skity_common.h>

TEXTRA_EXTERN_C_BEGIN
static inline skity_shader textra_skity_impl_make_gradient_shader(
    const textra_backend_gradient_view_t* fill) {
  skity_color4f* colors;
  skity_shader result = NULL;
  skity_matrix matrix = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
  uint32_t i;
  if (!fill->stop_count || fill->stop_count > INT32_MAX || !fill->colors ||
      !fill->positions)
    return NULL;
  colors = (skity_color4f*)textra_skity_impl_allocate(fill->stop_count,
                                                      sizeof(*colors));
  if (!colors) return NULL;
  for (i = 0; i < fill->stop_count; ++i) {
    uint32_t c = fill->colors[i];
    colors[i].x = ((c >> 16) & 255) / 255.f;
    colors[i].y = ((c >> 8) & 255) / 255.f;
    colors[i].z = (c & 255) / 255.f;
    colors[i].w = (c >> 24) / 255.f;
  }
  switch (fill->type) {
    case TEXTRA_GRADIENT_TYPE_LINEAR: {
      const skity_point points[2] = {{{{fill->start.x, fill->start.y, 0, 1}}},
                                     {{{fill->end.x, fill->end.y, 0, 1}}}};
      result = skity_shader_create_linear(points, colors, fill->positions,
                                          (int32_t)fill->stop_count,
                                          SKITY_TILE_MODE_CLAMP, 0);
      break;
    }
    case TEXTRA_GRADIENT_TYPE_RADIAL: {
      const float ratio = fill->radius_x == 0 || fill->radius_y == 0
                              ? 1
                              : fill->radius_x / fill->radius_y;
      const skity_point center = {{{fill->center.x, fill->center.y, 0, 1}}};
      result = skity_shader_create_radial(
          center, fmaxf(fill->radius_x, 1.f), colors, fill->positions,
          (int32_t)fill->stop_count, SKITY_TILE_MODE_CLAMP, 0);
      matrix.m[5] = 1 / ratio;
      matrix.m[13] = fill->center.y * (1 - matrix.m[5]);
      if (result) skity_shader_set_local_matrix(result, &matrix);
      break;
    }
    case TEXTRA_GRADIENT_TYPE_CONIC: {
      const float angle =
          (fill->start_angle - 90) * 3.14159265358979323846f / 180;
      const float c = cosf(angle), s = sinf(angle);
      result = skity_shader_create_sweep(
          fill->center.x, fill->center.y, 0, 360, colors, fill->positions,
          (int32_t)fill->stop_count, SKITY_TILE_MODE_CLAMP, 0);
      matrix.m[0] = matrix.m[5] = c;
      matrix.m[1] = s;
      matrix.m[4] = -s;
      matrix.m[12] = fill->center.x * (1 - c) + fill->center.y * s;
      matrix.m[13] = fill->center.y * (1 - c) - fill->center.x * s;
      if (result) skity_shader_set_local_matrix(result, &matrix);
      break;
    }
  }
  free(colors);
  return result;
}
static inline skity_paint textra_skity_impl_make_paint(
    const textra_backend_paint_t* paint) {
  skity_paint native_paint = skity_paint_create();
  if (!native_paint) return NULL;
  skity_paint_set_text_size(native_paint, paint->text_size);
  skity_paint_set_stroke_width(native_paint, paint->stroke_width);
  skity_paint_set_stroke_miter(native_paint, paint->stroke_miter);
  skity_paint_set_stroke_cap(native_paint, (skity_paint_cap)(paint->cap));
  skity_paint_set_stroke_join(native_paint, (skity_paint_join)(paint->join));
  if (paint->fill_color != 0u) {
    skity_paint_set_fill_color(native_paint, paint->fill_color);
    skity_paint_set_style(native_paint, SKITY_PAINT_STYLE_FILL);
  }
  if (paint->stroke_color != 0u) {
    skity_paint_set_stroke_color(native_paint, paint->stroke_color);
    skity_paint_set_style(native_paint, paint->fill_color != 0u
                                            ? SKITY_PAINT_STYLE_STROKE_AND_FILL
                                            : SKITY_PAINT_STYLE_STROKE);
  }
  if (paint->gradient) {
    skity_paint_set_style(native_paint, SKITY_PAINT_STYLE_FILL);
    skity_paint_set_color(native_paint, 0xffffffff);
    skity_shader shader =
        textra_skity_impl_make_gradient_shader(paint->gradient);
    if (!shader) {
      skity_paint_destroy(native_paint);
      return NULL;
    }
    skity_paint_set_shader(native_paint, shader);
    skity_shader_destroy(shader);
  }
  return native_paint;
}

/* Draw a gradient fill first and, when present, its separate solid stroke. */
static inline textra_bool_t textra_skity_impl_stroke_pass(
    const textra_backend_paint_t* paint_options, skity_paint native_paint) {
  if (!paint_options->gradient || !paint_options->stroke_color)
    return TEXTRA_FALSE;
  skity_paint_set_shader(native_paint, NULL);
  skity_paint_set_color(native_paint, paint_options->stroke_color);
  skity_paint_set_style(native_paint, SKITY_PAINT_STYLE_STROKE);
  return TEXTRA_TRUE;
}
TEXTRA_EXTERN_C_END
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKITY_COMMON_TEXTRA_SKITY_PAINT_IMPL_H_
