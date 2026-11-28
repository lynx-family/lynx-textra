// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <math.h>
#include <skity/c/skity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <textra/capi/platform/textra_capi_skity.h>
#include <textra/capi/platform/textra_capi_skity_coretext.h>

#define CHECK(condition)                                              \
  do {                                                                \
    if (!(condition)) {                                               \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      abort();                                                        \
    }                                                                 \
  } while (0)
#define OK(expression) CHECK((expression) == TEXTRA_STATUS_OK)

textra_status_t skity_header_peer_initialize(
    textra_typeface_options_t* options);

static void draw_scene(textra_canvas_options_t* c,
                       textra_typeface_options_t* f) {
  const textra_backend_canvas_api_t* a = c->callbacks;
  textra_backend_paint_t paint;
  textra_backend_gradient_view_t gradient;
  textra_backend_shadow_t shadow = {0x80000000u, 2, 2, 1.5};
  const uint32_t colors[] = {0xffff0000u, 0xff0000ffu};
  const float stops[] = {0, 1};
  uint16_t glyph;
  float x = 0, y = 0;
  void* canvas = c->native_canvas;
  void* face = (void*)f->native_font;
  memset(&paint, 0, sizeof(paint));
  paint.struct_size = sizeof(paint);
  paint.fill_color = 0xff123456u;
  paint.stroke_color = 0xff00ff00u;
  paint.stroke_width = 2;
  paint.stroke_miter = 4;
  paint.text_size = 24;
  memset(&gradient, 0, sizeof(gradient));
  gradient.struct_size = sizeof(gradient);
  gradient.start.x = 10;
  gradient.end.x = 100;
  gradient.center.x = gradient.center.y = 50;
  gradient.radius_x = 40;
  gradient.radius_y = 20;
  gradient.colors = colors;
  gradient.positions = stops;
  gradient.stop_count = 2;
  OK(a->clear(NULL, canvas));
  OK(a->start_paint(NULL, canvas));
  OK(a->save(NULL, canvas));
  OK(a->translate(NULL, canvas, 1, 1));
  OK(a->scale(NULL, canvas, 1, 1));
  OK(a->rotate(NULL, canvas, 0));
  OK(a->skew(NULL, canvas, 0, 0));
  OK(a->clip_rect(NULL, canvas, 0, 0, 128, 128, 1));
  OK(a->fill_rect(NULL, canvas, 0, 0, 128, 128, 0xffffffffu));
  OK(a->draw_line(NULL, canvas, 5, 5, 120, 5, &paint));
  paint.gradient = &gradient;
  gradient.type = TEXTRA_GRADIENT_TYPE_LINEAR;
  OK(a->draw_rect(NULL, canvas, 10, 10, 60, 30, &paint));
  gradient.type = TEXTRA_GRADIENT_TYPE_RADIAL;
  OK(a->draw_oval(NULL, canvas, 65, 10, 115, 30, &paint));
  gradient.type = TEXTRA_GRADIENT_TYPE_CONIC;
  gradient.start_angle = 45;
  OK(a->draw_circle(NULL, canvas, 25, 50, 15, &paint));
  OK(a->draw_arc(NULL, canvas, 55, 35, 110, 65, 0, 200, 1, &paint));
  OK(a->draw_round_rect(NULL, canvas, 5, 70, 55, 95, 5, &paint));
  OK(a->draw_arc_to(NULL, canvas, 60, 70, 105, 70, 105, 95, 6, &paint));
  {
    const textra_point_t points[] = {{5, 100}, {40, 100}, {45, 120}};
    textra_backend_path_t path;
    memset(&path, 0, sizeof(path));
    path.struct_size = sizeof(path);
    path.kind = TEXTRA_PATH_KIND_BEZIER;
    path.points = points;
    path.point_count = 3;
    OK(a->draw_path(NULL, canvas, &path, &paint));
  }
  OK(f->callbacks->unichar_to_glyph(NULL, face, 'A', 0, &glyph));
  paint.bold = paint.italic = 1;
  paint.shadows = &shadow;
  paint.shadow_count = 1;
  OK(a->draw_glyphs(NULL, canvas, face, 1, &glyph, "A", 1, 65, 120, &x, &y,
                    &paint));
  OK(a->restore(NULL, canvas));
  OK(a->end_paint(NULL, canvas));
}

int main(int argc, char** argv) {
  textra_font_manager_options_t manager;
  textra_typeface_options_t face, failed_face;
  textra_canvas_options_t canvas;
  textra_font_style_t style = {400, 5, 0};
  skity_font_manager native_manager = skity_font_manager_ref_default();
  skity_typeface native_face;
  const skity_font_style native_style = {400, 5, SKITY_FONT_SLANT_UPRIGHT};
  skity_bitmap bitmap;
  skity_canvas native_canvas;
  const void* data;
  const void* native;
  void* matched = NULL;
  float advance, advances[2];
  textra_rect_t bound, bounds;
  textra_backend_em_height_t em;
  textra_backend_font_metrics_t metrics;
  uint16_t glyph, glyphs[2];
  const uint32_t chars[] = {'A', 'B'};
  uint64_t data_size;
  int32_t count;
  uint32_t units;
  size_t bytes;
  unsigned char* pixels;
  unsigned char* first;
  int userdata = 42;
  CHECK(native_manager);
  memset(&manager, 0, sizeof(manager));
  manager.struct_size = sizeof(manager);
  manager.font_manager = native_manager;
  manager.userdata = &userdata;
  OK(textra_skity_font_manager_callbacks_initialize(&manager));
  CHECK(manager.userdata == &userdata);
  CHECK(textra_skity_font_manager_callbacks_initialize(&manager) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  OK(manager.manager_callbacks->count_families(NULL, manager.font_manager,
                                               &count));
  CHECK(count > 0);
  OK(manager.manager_callbacks->match_family_style(NULL, manager.font_manager,
                                                   NULL, &style, &matched));
  CHECK(matched);
  manager.typeface_callbacks->destroy(NULL, matched);
  native_face =
      skity_font_manager_get_default_typeface(native_manager, native_style);
  CHECK(native_face);
  memset(&face, 0, sizeof(face));
  face.struct_size = sizeof(face);
  face.native_font = native_face;
  face.userdata = &userdata;
  OK(skity_header_peer_initialize(&face));
  CHECK(face.userdata == &userdata);
  skity_typeface_destroy(native_face);
  failed_face = face;
  CHECK(textra_skity_typeface_callbacks_initialize(&face) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(memcmp(&failed_face, &face, sizeof(face)) == 0);
  OK(face.callbacks->unichar_to_glyph(NULL, (void*)face.native_font, 'A', 0,
                                      &glyph));
  CHECK(glyph);
  OK(face.callbacks->unichars_to_glyphs(NULL, (void*)face.native_font, chars, 2,
                                        glyphs));
  CHECK(glyphs[0] == glyph && glyphs[1]);
  OK(face.callbacks->horizontal_advance(NULL, (void*)face.native_font, glyph,
                                        24, &advance));
  OK(face.callbacks->horizontal_advances(NULL, (void*)face.native_font, glyphs,
                                         2, 24, advances));
  CHECK(advance > 0 && advance == advances[0]);
  OK(face.callbacks->width_bound(NULL, (void*)face.native_font, glyph, 24,
                                 &bound));
  OK(face.callbacks->width_bounds(NULL, (void*)face.native_font, glyphs, 2, 24,
                                  &bounds));
  CHECK(bound.right > 0 && bounds.right > bounds.left);
  OK(face.callbacks->units_per_em(NULL, (void*)face.native_font, &units));
  CHECK(units > 0);
  OK(face.callbacks->font_info(NULL, (void*)face.native_font, 24, &metrics));
  OK(face.callbacks->em_height(NULL, (void*)face.native_font, 24, &em));
  CHECK(fabsf(em.ascent + em.descent - 24) < 0.001f);
  OK(face.callbacks->font_data(NULL, (void*)face.native_font, &data));
  OK(face.callbacks->font_data_size(NULL, (void*)face.native_font, &data_size));
  CHECK(data && data_size);
  OK(face.callbacks->native_font(NULL, (void*)face.native_font, &native));
  CHECK(native);
  bitmap = skity_bitmap_create(128, 128, SKITY_ALPHA_TYPE_PREMUL,
                               SKITY_COLOR_TYPE_RGBA);
  CHECK(bitmap);
  native_canvas = skity_canvas_make_software_canvas(bitmap);
  CHECK(native_canvas);
  memset(&canvas, 0, sizeof(canvas));
  canvas.struct_size = sizeof(canvas);
  canvas.native_canvas = native_canvas;
  OK(textra_skity_canvas_callbacks_initialize(&canvas));
  draw_scene(&canvas, &face);
  bytes = skity_bitmap_get_row_bytes(bitmap) * skity_bitmap_get_height(bitmap);
  pixels = (unsigned char*)skity_bitmap_get_pixels(bitmap);
  first = (unsigned char*)malloc(bytes);
  CHECK(first);
  memcpy(first, pixels, bytes);
  draw_scene(&canvas, &face);
  CHECK(memcmp(first, pixels, bytes) == 0);
  if (argc == 2) {
    FILE* output = fopen(argv[1], "wb");
    CHECK(output);
    CHECK(fwrite(pixels, 1, bytes, output) == bytes);
    CHECK(fclose(output) == 0);
  }
  free(first);
  canvas.callbacks->destroy(NULL, native_canvas);
  skity_canvas_draw_color(native_canvas, 0xff00ff00u, SKITY_BLEND_MODE_SRC);
  CHECK(pixels[1] == 255 && pixels[3] == 255);
  OK(textra_skity_coretext_canvas_callbacks_initialize(&canvas));
  CHECK(canvas.backend == TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
  face.callbacks->destroy(NULL, (void*)face.native_font);
  manager.manager_callbacks->destroy(NULL, manager.font_manager);
  CHECK(skity_font_manager_count_families(native_manager) == count);
  memset(&manager, 0, sizeof(manager));
  manager.struct_size = sizeof(manager);
  manager.font_manager = native_manager;
  CHECK(textra_skity_coretext_font_manager_callbacks_initialize(&manager) ==
        TEXTRA_STATUS_UNSUPPORTED);
  CHECK(manager.font_manager == native_manager && !manager.manager_callbacks);
  OK(textra_skity_font_manager_callbacks_initialize(&manager));
  manager.manager_callbacks->destroy(NULL, manager.font_manager);
  memset(&manager, 0, sizeof(manager));
  manager.struct_size = sizeof(manager);
  OK(textra_skity_font_manager_callbacks_initialize(&manager));
  manager.manager_callbacks->destroy(NULL, manager.font_manager);
  skity_font_manager_destroy(native_manager);
  skity_canvas_destroy(native_canvas);
  skity_bitmap_destroy(bitmap);
  puts("PASS: C header-only multi-TU ownership, metrics and repeated drawing");
  return 0;
}
