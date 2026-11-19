// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone host test: link Skia and a Textra core with HarfBuzz enabled.
#include <examples/capi/platform/skia/textra_skia_backend.h>
#include <include/core/SkFontMgr.h>
#include <textra/capi/textra.h>
#include <textra/platform/skia/skia_canvas_helper.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "include/core/SkSurface.h"
#include "include/ports/SkFontMgr_directory.h"
#include "test/capi_canvas_test_calls.h"
#define CHECK(x)                                      \
  do {                                                \
    if (!(x)) {                                       \
      std::fprintf(stderr, "%d: %s\n", __LINE__, #x); \
      std::abort();                                   \
    }                                                 \
  } while (0)
#define OK(x) CHECK((x) == TEXTRA_STATUS_OK)
int main(int argc, char** argv) {
  CHECK(argc == 2);
  auto manager = SkFontMgr_New_Custom_Directory(argv[1]);
  CHECK(manager);
  auto native = manager->legacyMakeTypeface(nullptr, SkFontStyle());
  CHECK(native);
  auto old_surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(320, 80));
  auto new_surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(320, 80));
  CHECK(old_surface && new_surface);
  // Skia Canvas stays borrowed; the manager acquires exactly one SDK reference.
  int userdata = 0;
  textra_canvas_options_t canvas_probe{};
  canvas_probe.struct_size = sizeof(canvas_probe);
  canvas_probe.native_canvas = new_surface->getCanvas();
  canvas_probe.userdata = &userdata;
  OK(textra_skia_canvas_callbacks_initialize(&canvas_probe));
  CHECK(canvas_probe.native_canvas == new_surface->getCanvas());
  CHECK(canvas_probe.userdata == &userdata);
  canvas_probe.callbacks->destroy(canvas_probe.userdata,
                                  canvas_probe.native_canvas);
  new_surface->getCanvas()->clear(0);
  CHECK(manager->unique());
  textra_font_manager_options_t manager_probe{};
  manager_probe.struct_size = sizeof(manager_probe);
  manager_probe.font_manager = manager.get();
  manager_probe.userdata = &userdata;
  OK(textra_skia_font_manager_callbacks_initialize(&manager_probe));
  CHECK(manager_probe.font_manager == manager.get());
  CHECK(manager_probe.userdata == &userdata && !manager->unique());
  CHECK(textra_skia_font_manager_callbacks_initialize(&manager_probe) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  manager_probe.manager_callbacks->destroy(manager_probe.userdata,
                                           manager_probe.font_manager);
  CHECK(manager->unique());
  textra_canvas_options_t co{sizeof(co), TEXTRA_RENDERING_BACKEND_SKIA, 0,
                             new_surface->getCanvas()};
  OK(textra_skia_canvas_callbacks_initialize(&co));
  textra_layout_drawer_options_t draw_options{sizeof(draw_options), &co};
  textra_layout_drawer_t* drawer = nullptr;
  OK(textra_layout_drawer_create(&draw_options, &drawer));
  auto canvas = drawer->canvas.get();
  textra_typeface_options_t fo{sizeof(fo), TEXTRA_RENDERING_BACKEND_SKIA, 0,
                               native.get()};
  OK(textra_skia_typeface_callbacks_initialize(&fo));
  textra_font_descriptor_t* descriptor = nullptr;
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_set_typeface(descriptor, &fo));
  auto face = descriptor->typeface.get();
  textra_font_manager_options_t mo{sizeof(mo), manager.get()};
  OK(textra_skia_font_manager_callbacks_initialize(&mo));
  textra_text_layout_options_t lo{};
  lo.struct_size = sizeof(lo);
  lo.backend = TEXTRA_RENDERING_BACKEND_SKIA;
  lo.default_font_manager = &mo;
  textra_text_layout_t* engine = nullptr;
  OK(textra_text_layout_create(&lo, &engine));
  auto engine_manager = engine->fonts.GetDefaultFontManager();
  TypefaceRef engine_face = face->impl;
  const void* pointer =
      engine_manager->getPlatformFontFromTypeface(engine_face);
  auto imported = engine_manager->createTypefaceFromPlatformFont(pointer);
  CHECK(imported == face->impl);
  const uint32_t chars[]{'o', 'f', 'f', 'i', 'c', 'e'};
  uint16_t glyphs[6]{};
  engine_face->UnicharsToGlyphs(chars, 6, glyphs);
  float advances[6]{};
  engine_face->GetHorizontalAdvances(glyphs, 6, advances, 20);
  float x[6]{}, y[6]{};
  for (int i = 1; i < 6; ++i) x[i] = x[i - 1] + advances[i - 1];
  textra_backend_paint_t paint{};
  paint.struct_size = sizeof(paint);
  paint.fill_color = 0xff202020;
  paint.text_size = 20;
  paint.stroke_miter = 4;
  SkiaCanvasHelper old_canvas(old_surface->getCanvas());
  auto old_face =
      std::make_shared<ttoffice::tttext::SkiaTypefaceHelper>(native);
  auto old_paint = old_canvas.CreatePainter();
  old_paint->SetFillColor(paint.fill_color);
  old_paint->SetTextSize(20);
  old_paint->SetStrokeMiter(4);
  for (int variant = 0; variant < 2; ++variant) {
    paint.bold = variant;
    paint.italic = variant;
    old_paint->SetBold(variant != 0);
    old_paint->SetItalic(variant != 0);
    old_surface->getCanvas()->clear(0);
    new_surface->getCanvas()->clear(0);
    old_canvas.DrawGlyphs(old_face.get(), 6, glyphs, "office", 6, 10, 40, x, y,
                          old_paint.get());
    textra_test_canvas_call_t call{};
    call.struct_size = sizeof(call);
    call.operation = TEXTRA_TEST_CANVAS_DRAW_GLYPHS;
    call.values[0] = 10;
    call.values[1] = 40;
    call.paint = &paint;
    call.glyphs = glyphs;
    call.glyph_count = 6;
    call.x = x;
    call.y = y;
    call.text = "office";
    call.text_bytes = 6;
    OK(textra_test_canvas_call(canvas, &call, face));
    SkPixmap a, b;
    CHECK(old_surface->peekPixels(&a) && new_surface->peekPixels(&b));
    CHECK(a.computeByteSize() == b.computeByteSize());
    CHECK(!memcmp(a.addr(), b.addr(), a.computeByteSize()));
    std::vector<uint8_t> first(
        static_cast<const uint8_t*>(b.addr()),
        static_cast<const uint8_t*>(b.addr()) + b.computeByteSize());
    new_surface->getCanvas()->clear(0);
    OK(textra_test_canvas_call(canvas, &call, face));
    CHECK(!memcmp(first.data(), b.addr(), first.size()));
  }
  // Exercise font bytes/index/metrics through the real HarfBuzz engine.
  textra_style_t* style = nullptr;
  OK(textra_style_create(&style));
  OK(textra_style_set_font_descriptor(style, descriptor));
  OK(textra_style_set_text_size(style, 20));
  OK(textra_style_set_foreground_color(style, 0xff202020));
  textra_paragraph_options_t po{};
  po.struct_size = sizeof(po);
  po.type = TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE;
  textra_paragraph_t* paragraph = nullptr;
  OK(textra_paragraph_create(&po, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, style, "office ffi", 10));
  textra_text_context_t* context = nullptr;
  OK(textra_text_context_create(&context));
  OK(textra_text_layout_layout_single_line(engine, paragraph, context));
  new_surface->getCanvas()->clear(0);
  OK(textra_layout_drawer_draw_single_line(drawer, paragraph, 10, 40));
  SkPixmap pixels;
  CHECK(new_surface->peekPixels(&pixels));
  auto bytes = static_cast<const uint8_t*>(pixels.addr());
  std::vector<uint8_t> first(bytes, bytes + pixels.computeByteSize());
  CHECK(std::any_of(first.begin(), first.end(),
                    [](uint8_t b) { return b != 0; }));
  new_surface->getCanvas()->clear(0);
  OK(textra_layout_drawer_draw_single_line(drawer, paragraph, 10, 40));
  CHECK(!memcmp(first.data(), pixels.addr(), first.size()));
  textra_layout_drawer_destroy(drawer);
  textra_text_context_destroy(context);
  textra_paragraph_destroy(paragraph);
  textra_style_destroy(style);
  textra_font_descriptor_destroy(descriptor);
  textra_text_layout_destroy(engine);
  textra_text_layout_clear_shape_cache();
  std::puts(
      "Independent Skia C callbacks match C++ drawing; HarfBuzz layout, "
      "repeated draws and "
      "native font round trips passed");
}
