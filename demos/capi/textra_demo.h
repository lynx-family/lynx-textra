// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef DEMOS_CAPI_TEXTRA_DEMO_H_
#define DEMOS_CAPI_TEXTRA_DEMO_H_
#include <string.h>
#include <textra/capi/textra.h>

/* Shared inputs for the Android, iOS and Skity demos and their pixel checks.
 * No SDK or C++ headers are required by this example. */
typedef struct textra_demo_case {
  const char* name;
  const char* text;
  uint32_t single_line, bold, italic;
} textra_demo_case_t;
static const textra_demo_case_t textra_demo_cases[] = {
    {"latin", "Hello office ffi world", 1, 0, 0},
    {"cjk", "中文排版测试 Hello", 1, 0, 0},
    {"wrapped",
     "Hello office ffi world. Text wraps across several lines. 中文排版测试。",
     0, 0, 0},
    {"rtl", "ABC العربية 123 עברית", 0, 0, 0},
    {"bold", "Bold office ffi 中文", 0, 1, 0},
    {"italic", "Italic office ffi", 0, 0, 1},
    {"multiline", "First line\n第二行 中文\nThird office ffi", 0, 0, 0},
    {"fallback", "Font fallback 中文 العربية 🎉", 0, 0, 0},
};
#define TEXTRA_DEMO_CASE_COUNT \
  (sizeof(textra_demo_cases) / sizeof(textra_demo_cases[0]))

typedef struct textra_demo_metrics {
  float width, height;
  uint32_t lines;
} textra_demo_metrics_t;

/* The host supplies borrowed native callback records. face may be NULL.
 * This example releases every engine owner and clears shaped-font caches before
 * returning, so the host can then release its borrowed native resources. */
static inline void TEXTRA_CALL textra_demo_borrowed_destroy(void* userdata,
                                                            void* object) {
  (void)userdata;
  (void)object;
}
static inline textra_status_t textra_demo_render_config(
    textra_rendering_backend_t backend,
    const textra_font_manager_options_t* manager,
    const textra_typeface_options_t* face,
    const textra_canvas_options_t* canvas, uint32_t case_index, float width,
    float height, textra_demo_metrics_t* metrics, textra_bool_t metrics_round,
    float font_size) {
  textra_status_t status = TEXTRA_STATUS_INVALID_ARGUMENT;
  textra_text_layout_t* engine = NULL;
  textra_font_descriptor_t* descriptor = NULL;
  textra_style_t* style = NULL;
  textra_paragraph_t* paragraph = NULL;
  textra_text_context_t* context = NULL;
  textra_layout_region_t* region = NULL;
  textra_layout_drawer_t* drawer = NULL;
  textra_text_layout_options_t lo = {0};
  textra_paragraph_options_t po = {0};
  textra_layout_region_options_t ro = {0};
  textra_layout_drawer_options_t draw = {0};
  textra_backend_font_manager_api_t manager_api = {0};
  textra_font_manager_options_t manager_options = {0};
  manager_options.struct_size = sizeof(manager_options);

  if (manager) manager_options = *manager;
  if (manager && manager->manager_callbacks) {
    manager_api = *manager->manager_callbacks;
    manager_api.destroy = textra_demo_borrowed_destroy;
    manager_options.manager_callbacks = &manager_api;
  }
  textra_backend_canvas_api_t canvas_api = {0};
  textra_canvas_options_t canvas_options = *canvas;
  if (canvas->callbacks) {
    canvas_api = *canvas->callbacks;
    canvas_api.destroy = textra_demo_borrowed_destroy;
    canvas_options.callbacks = &canvas_api;
  }
  textra_backend_typeface_api_t face_api = {0};
  textra_typeface_options_t face_options = {0};
  if (face) face_options = *face;
  if (face && face->callbacks) {
    face_api = *face->callbacks;
    face_api.destroy = textra_demo_borrowed_destroy;
    face_options.callbacks = &face_api;
  }
  textra_layout_flow_t flow = TEXTRA_LAYOUT_FLOW_NORMAL;
  const textra_demo_case_t* sample;
  if (case_index >= TEXTRA_DEMO_CASE_COUNT || !metrics) return status;
  sample = &textra_demo_cases[case_index];
#define TEXTRA_DEMO_TRY(expr) \
  do {                        \
    status = (expr);          \
    if (status) goto cleanup; \
  } while (0)
  lo.struct_size = sizeof(lo);
  lo.default_font_manager = &manager_options;
  lo.backend = backend;
  TEXTRA_DEMO_TRY(textra_text_layout_create(&lo, &engine));
  TEXTRA_DEMO_TRY(textra_font_descriptor_create(&descriptor));
  if (face)
    TEXTRA_DEMO_TRY(
        textra_font_descriptor_set_typeface(descriptor, &face_options));
  TEXTRA_DEMO_TRY(textra_style_create(&style));
  TEXTRA_DEMO_TRY(textra_style_set_font_descriptor(style, descriptor));
  TEXTRA_DEMO_TRY(textra_style_set_text_size(style, font_size));
  TEXTRA_DEMO_TRY(textra_style_set_foreground_color(style, 0xff203040));
  TEXTRA_DEMO_TRY(textra_style_set_bold(style, sample->bold));
  TEXTRA_DEMO_TRY(textra_style_set_italic(style, sample->italic));
  po.struct_size = sizeof(po);
  po.type = sample->single_line ? TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE
                                : TEXTRA_PARAGRAPH_TYPE_GENERAL;
  TEXTRA_DEMO_TRY(textra_paragraph_create(&po, &paragraph));
  TEXTRA_DEMO_TRY(textra_paragraph_add_text_run(
      paragraph, style, sample->text, (uint32_t)strlen(sample->text)));
  TEXTRA_DEMO_TRY(textra_text_context_create(&context));
  if (metrics_round)
    TEXTRA_DEMO_TRY(textra_text_context_set_feature(
        context, TEXTRA_FEATURE_ANDROID_METRICS_ROUND, metrics_round));
  if (sample->single_line) {
    TEXTRA_DEMO_TRY(
        textra_text_layout_layout_single_line(engine, paragraph, context));
    textra_paragraph_metrics_t m = {0};
    m.struct_size = sizeof(m);
    TEXTRA_DEMO_TRY(textra_paragraph_get_metrics(paragraph, &m));
    metrics->width = m.width;
    metrics->height = m.height;
    metrics->lines = 1;
  } else {
    ro.struct_size = sizeof(ro);
    ro.width = width;
    ro.height = height;
    ro.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
    ro.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
    TEXTRA_DEMO_TRY(textra_layout_region_create(&ro, &region));
    TEXTRA_DEMO_TRY(
        textra_text_layout_layout(engine, paragraph, region, context, &flow));
    if (flow != TEXTRA_LAYOUT_FLOW_NORMAL) {
      status = TEXTRA_STATUS_OUT_OF_RANGE;
      goto cleanup;
    }
    textra_layout_region_metrics_t m = {0};
    m.struct_size = sizeof(m);
    TEXTRA_DEMO_TRY(textra_layout_region_get_metrics(region, &m));
    metrics->width = m.width;
    metrics->height = m.height;
    metrics->lines = m.line_count;
  }
  draw.struct_size = sizeof(draw);
  draw.canvas = &canvas_options;
  TEXTRA_DEMO_TRY(textra_layout_drawer_create(&draw, &drawer));
  TEXTRA_DEMO_TRY(
      sample->single_line
          ? textra_layout_drawer_draw_single_line(drawer, paragraph, 0, 0)
          : textra_layout_drawer_draw_region(drawer, region));
cleanup:
  textra_layout_drawer_destroy(drawer);
  textra_layout_region_destroy(region);
  textra_text_context_destroy(context);
  textra_paragraph_destroy(paragraph);
  textra_style_destroy(style);
  textra_font_descriptor_destroy(descriptor);
  textra_text_layout_destroy(engine);
  textra_text_layout_clear_shape_cache();
#undef TEXTRA_DEMO_TRY
  return status;
}
static inline textra_status_t textra_demo_render(
    textra_rendering_backend_t backend,
    const textra_font_manager_options_t* manager,
    const textra_typeface_options_t* face,
    const textra_canvas_options_t* canvas, uint32_t case_index, float width,
    float height, textra_demo_metrics_t* metrics) {
  return textra_demo_render_config(backend, manager, face, canvas, case_index,
                                   width, height, metrics, TEXTRA_FALSE, 24);
}
#endif  // DEMOS_CAPI_TEXTRA_DEMO_H_
