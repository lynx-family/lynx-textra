// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// This provider and consumer compile as C11, without native SDK/C++ headers.
#include <textra/capi/textra.h>
#undef NDEBUG
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define OK(x) assert((x) == TEXTRA_STATUS_OK)
static int live_objects, token, fail_metrics;
static void destroy(void* userdata, void* object) {
  assert(userdata == &token && object == &token);
  --live_objects;
}
static textra_status_t create(void* userdata, void* unused, void** out) {
  (void)unused;
  assert(userdata == &token);
  *out = &token;
  ++live_objects;
  return TEXTRA_STATUS_OK;
}
static textra_status_t count(void* u, void* p, int32_t* out) {
  (void)u;
  (void)p;
  *out = 2;
  return 0;
}
static unsigned font_calls[8];
static textra_status_t match_family_style(void* u, void* p, const char* family,
                                          const textra_font_style_t* style,
                                          void** out) {
  ++font_calls[0];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t match_generic_family(void* u, void* p,
                                            const char* family,
                                            const textra_font_style_t* style,
                                            void** out) {
  ++font_calls[1];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t match_family_style_literal(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    void** out) {
  ++font_calls[2];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t match_family_style_character(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    const char* const* languages, uint32_t language_count, uint32_t character,
    void** out) {
  ++font_calls[3];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t make_from_file(void* u, void* p, const char* path,
                                      int32_t collection_index, void** out) {
  ++font_calls[4];
  return create(u, p, out);
}
static textra_status_t legacy_make_typeface(void* u, void* p,
                                            const char* family,
                                            const textra_font_style_t* style,
                                            void** out) {
  ++font_calls[5];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t legacy_make_typeface_literal(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    void** out) {
  ++font_calls[6];
  if (family && !strcmp(family, "missing")) {
    *out = NULL;
    return 0;
  }
  return create(u, p, out);
}
static textra_status_t create_typeface_from_platform_font(
    void* u, void* p, const void* native_font, void** out) {
  ++font_calls[7];
  return create(u, p, out);
}
static textra_status_t platform(void* u, void* p, void* f, const void** out) {
  assert(u == p && p == f);
  *out = f;
  return 0;
}
static textra_status_t advance(void* u, void* p, uint16_t g, float z,
                               float* out) {
  (void)u;
  (void)p;
  *out = g + z;
  return 0;
}
static textra_status_t advances(void* u, void* p, const uint16_t* g, uint32_t n,
                                float z, float* out) {
  for (uint32_t i = 0; i < n; ++i) advance(u, p, g[i], z, out + i);
  return 0;
}
static textra_status_t bound(void* u, void* p, uint16_t g, float z,
                             textra_rect_t* out) {
  (void)u;
  (void)p;
  (void)g;
  *out = (textra_rect_t){1, -z, 3, z};
  return 0;
}
static textra_status_t bounds(void* u, void* p, const uint16_t* g, uint32_t n,
                              float z, textra_rect_t* out) {
  (void)n;
  return bound(u, p, g ? *g : 0, z, out);
}
static unsigned data_calls, data_size_calls;
static textra_status_t data(void* u, void* p, const void** out) {
  (void)u;
  (void)p;
  ++data_calls;
  *out = "font";
  return 0;
}
static textra_status_t data_size(void* u, void* p, uint64_t* out) {
  (void)u;
  (void)p;
  ++data_size_calls;
  *out = 4;
  return 0;
}
static textra_status_t font_index(void* u, void* p, int32_t* out) {
  (void)u;
  (void)p;
  *out = 3;
  return 0;
}
static textra_status_t glyph(void* u, void* p, uint32_t c, uint32_t s,
                             uint16_t* out) {
  (void)u;
  (void)p;
  (void)s;
  *out = (uint16_t)c;
  return 0;
}
static textra_status_t glyphs(void* u, void* p, const uint32_t* c, uint32_t n,
                              uint16_t* out) {
  for (uint32_t i = 0; i < n; ++i) glyph(u, p, c[i], 0, out + i);
  return 0;
}
static textra_status_t units(void* u, void* p, uint32_t* out) {
  (void)u;
  (void)p;
  *out = 1000;
  return 0;
}
static textra_status_t metrics(void* u, void* p, float z,
                               textra_backend_font_metrics_t* out) {
  (void)u;
  (void)p;
  if (fail_metrics) return TEXTRA_STATUS_INTERNAL_ERROR;
  out->top = -z;
  out->ascent = -z * .8f;
  out->descent = z * .2f;
  out->bottom = z * .2f;
  out->leading = 0;
  out->font_size = z;
  return 0;
}
static textra_status_t style(void* u, void* p, textra_font_style_t* out) {
  (void)u;
  (void)p;
  *out =
      (textra_font_style_t){TEXTRA_FONT_WEIGHT_NORMAL, TEXTRA_FONT_WIDTH_NORMAL,
                            TEXTRA_FONT_SLANT_UPRIGHT};
  return 0;
}
static textra_status_t native(void* u, void* p, const void** out) {
  (void)u;
  *out = p;
  return 0;
}

int main(void) {
  textra_backend_canvas_api_t canvas_api = {0};
  canvas_api.struct_size = sizeof(canvas_api);
  canvas_api.version = TEXTRA_CANVAS_API_VERSION;
  canvas_api.destroy = destroy;
  textra_backend_typeface_api_t face_api = {sizeof(face_api),
                                            TEXTRA_TYPEFACE_API_VERSION,
                                            destroy,
                                            advance,
                                            advances,
                                            bound,
                                            bounds,
                                            data,
                                            data_size,
                                            font_index,
                                            glyph,
                                            glyphs,
                                            units,
                                            metrics,
                                            NULL,
                                            style,
                                            native};
  textra_backend_font_manager_api_t manager_api = {
      sizeof(manager_api),
      TEXTRA_FONT_MANAGER_API_VERSION,
      destroy,
      count,
      match_family_style,
      match_generic_family,
      match_family_style_literal,
      match_family_style_character,
      make_from_file,
      legacy_make_typeface,
      legacy_make_typeface_literal,
      create_typeface_from_platform_font,
      platform};
  textra_canvas_options_t co = {
      sizeof(co), TEXTRA_RENDERING_BACKEND_SKIA, 0, NULL, &canvas_api, &token};
  textra_font_manager_options_t mo = {sizeof(mo), NULL, &manager_api, &face_api,
                                      &token};
  textra_typeface_options_t fo = {
      sizeof(fo), TEXTRA_RENDERING_BACKEND_SKIA, 0, NULL, &face_api, &token};
  void* font = NULL;
  OK(create(&token, NULL, &co.native_canvas));
  OK(create(&token, NULL, &mo.font_manager));
  OK(create(&token, NULL, &font));
  fo.native_font = font;
  textra_text_layout_t* layout = NULL;
  textra_text_layout_options_t lo = {0};
  lo.struct_size = sizeof(lo);
  lo.backend = TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT;
  lo.default_font_manager = &mo;
  textra_font_descriptor_t* descriptor = NULL;
  textra_layout_drawer_t* drawer = NULL;
  OK(textra_font_descriptor_create(&descriptor));
  lo.flags = UINT32_MAX;
  assert(textra_text_layout_create(&lo, &layout) == TEXTRA_STATUS_UNSUPPORTED);
  lo.flags = 0;
  assert(textra_font_descriptor_set_typeface(NULL, &fo) ==
         TEXTRA_STATUS_INVALID_ARGUMENT);
  assert(live_objects == 3);
  OK(textra_text_layout_create(&lo, &layout));
  OK(textra_font_descriptor_set_typeface(descriptor, &fo));
  textra_layout_drawer_options_t draw = {sizeof(draw), &co, NULL, NULL};
  textra_layout_drawer_callbacks_t invalid = {sizeof(invalid), 99, NULL, NULL};
  draw.callbacks = &invalid;
  assert(textra_layout_drawer_create(&draw, &drawer) ==
         TEXTRA_STATUS_UNSUPPORTED);
  assert(drawer == NULL && live_objects == 3);
  draw.callbacks = NULL;
  OK(textra_layout_drawer_create(&draw, &drawer));
  // All owners copied their tables, including destruction callbacks.
  memset(&canvas_api, 0, sizeof(canvas_api));
  memset(&manager_api, 0, sizeof(manager_api));
  memset(&face_api, 0, sizeof(face_api));
  textra_font_descriptor_t* clone = NULL;
  OK(textra_font_descriptor_clone(descriptor, &clone));
  textra_text_layout_destroy(layout);
  assert(live_objects == 2);
  textra_font_descriptor_destroy(descriptor);
  assert(live_objects == 2);
  OK(textra_font_descriptor_set_typeface(clone, NULL));
  assert(live_objects == 1);
  textra_font_descriptor_destroy(clone);
  textra_layout_drawer_destroy(drawer);
  assert(live_objects == 0);
  puts(
      "Pure C owner initialization, failure ownership and copied-table "
      "lifetime tests passed");
  return 0;
}
