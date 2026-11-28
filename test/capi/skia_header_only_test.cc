// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkTypeface.h>
#include <include/ports/SkFontMgr_empty.h>
#include <textra/capi/platform/textra_capi_skia.h>

#include <cstdio>
#include <cstring>
#include <vector>

textra_status_t InitializeSkiaCanvasInPeer(textra_canvas_options_t* options);
textra_status_t InitializeSkiaTypefaceInPeer(
    textra_typeface_options_t* options);

#define CHECK(condition)                                      \
  do {                                                        \
    if (!(condition)) {                                       \
      std::fprintf(stderr, "%d: %s\n", __LINE__, #condition); \
      return 1;                                               \
    }                                                         \
  } while (0)

int main(int argc, char** argv) {
  CHECK(argc == 2);
  auto manager = SkFontMgr_New_Custom_Empty();
  CHECK(manager && manager->unique());
  textra_font_manager_options_t manager_options{};
  manager_options.struct_size = sizeof(manager_options);
  manager_options.font_manager = manager.get();
  CHECK(textra_skia_font_manager_callbacks_initialize(&manager_options) == 0);
  CHECK(!manager->unique());
  auto font = manager->makeFromFile(argv[1]);
  CHECK(font);
  textra_typeface_options_t face{};
  face.struct_size = sizeof(face);
  face.native_font = font.get();
  CHECK(InitializeSkiaTypefaceInPeer(&face) == 0);
  font.reset();
  void* typeface = const_cast<void*>(face.native_font);
  uint16_t glyph = 0;
  float advance = 0;
  CHECK(face.callbacks->unichar_to_glyph(nullptr, typeface, 'H', 0, &glyph) ==
        0);
  CHECK(glyph);
  CHECK(face.callbacks->horizontal_advance(nullptr, typeface, glyph, 32,
                                           &advance) == 0);
  CHECK(advance > 0);
  SkBitmap bitmap;
  CHECK(bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(128, 64)));
  SkCanvas canvas(bitmap);
  textra_canvas_options_t canvas_options{};
  canvas_options.struct_size = sizeof(canvas_options);
  canvas_options.native_canvas = &canvas;
  CHECK(InitializeSkiaCanvasInPeer(&canvas_options) == 0);
  textra_backend_paint_t paint{};
  paint.struct_size = sizeof(paint);
  paint.fill_color = 0xff202020;
  paint.text_size = 32;
  paint.stroke_miter = 4;
  paint.bold = 1;
  paint.italic = 1;
  std::vector<unsigned char> first(bitmap.computeByteSize());
  float x = 0, y = 0;
  for (int pass = 0; pass < 2; ++pass) {
    canvas.clear(SK_ColorWHITE);
    CHECK(canvas_options.callbacks->draw_glyphs(nullptr, &canvas, typeface, 1,
                                                &glyph, "H", 1, 10, 45, &x, &y,
                                                &paint) == 0);
    if (!pass) {
      bool ink = false;
      for (int row = 0; row < bitmap.height(); ++row)
        for (int column = 0; column < bitmap.width(); ++column)
          ink |= bitmap.getColor(column, row) != SK_ColorWHITE;
      CHECK(ink);
      std::memcpy(first.data(), bitmap.getPixels(), first.size());
    } else {
      CHECK(std::memcmp(first.data(), bitmap.getPixels(), first.size()) == 0);
    }
    canvas_options.callbacks->destroy(nullptr, &canvas);
  }
  face.callbacks->destroy(nullptr, typeface);
  manager_options.manager_callbacks->destroy(nullptr,
                                             manager_options.font_manager);
  CHECK(manager->unique());
  std::puts(
      "PASS: Skia header-only multi-TU ownership, metrics and repeated draws");
  return 0;
}
