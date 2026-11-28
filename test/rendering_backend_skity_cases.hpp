// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Standalone integration client: link a library with Skity/HarfBuzz and
// Skity/CoreText enabled, plus a Skity library built with CoreText support.
#ifndef TEST_RENDERING_BACKEND_SKITY_CASES_HPP_
#define TEST_RENDERING_BACKEND_SKITY_CASES_HPP_

#include <CoreText/CoreText.h>
#include <textra/capi/textra.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <skity/graphic/bitmap.hpp>
#include <skity/render/canvas.hpp>
#include <skity/text/typeface.hpp>
#include <vector>

#include "src/capi/textra_internal.h"

#define CHECK(condition)                                                   \
  do {                                                                     \
    if (!(condition)) {                                                    \
      std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      std::abort();                                                        \
    }                                                                      \
  } while (0)
#define OK(expression) CHECK((expression) == TEXTRA_STATUS_OK)

// Exercise the native pointer through callbacks and engine ownership. In
// particular, adapter destruction must leave the borrowed Skity canvas usable.
inline void RunSkityCanvasCallbacksTest(
    textra_status_t(TEXTRA_CALL* initialize)(textra_canvas_options_t*),
    textra_rendering_backend_t backend) {
  CHECK(initialize(nullptr) == TEXTRA_STATUS_INVALID_ARGUMENT);
  skity::Bitmap bitmap(32, 32);
  auto native = skity::Canvas::MakeSoftwareCanvas(&bitmap);
  CHECK(native);
  int userdata = 0;
  textra_canvas_options_t options{};
  options.struct_size = sizeof(options);
  options.native_canvas = native.get();
  options.userdata = &userdata;
  options.flags = 1;
  CHECK(initialize(&options) == TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(options.native_canvas == native.get() && options.backend == 0);
  options.flags = 0;
  CHECK(initialize(&options) == TEXTRA_STATUS_OK);
  CHECK(options.native_canvas == native.get() && options.userdata == &userdata);
  CHECK(options.backend == backend && options.callbacks);
  CHECK(initialize(&options) == TEXTRA_STATUS_OK);
  std::memset(bitmap.GetPixelAddr(), 0, bitmap.RowBytes() * bitmap.Height());
  CHECK(options.callbacks->draw_color(options.userdata, options.native_canvas,
                                      0xffff0000u) == TEXTRA_STATUS_OK);
  CHECK(bitmap.GetPixelAddr()[0] || bitmap.GetPixelAddr()[3]);
  for (int i = 0; i < 2; ++i) {
    textra_layout_drawer_options_t draw{sizeof(draw), &options};
    textra_layout_drawer_t* drawer = nullptr;
    CHECK(textra_layout_drawer_create(&draw, &drawer) == TEXTRA_STATUS_OK);
    textra_layout_drawer_destroy(drawer);
    native->DrawColor(0xff00ff00u);
  }
}

inline int RunSkityBackendTest(
    int argc, char** argv,
    textra_status_t(TEXTRA_CALL* initialize_manager)(
        textra_font_manager_options_t*),
    textra_status_t(TEXTRA_CALL* initialize_face)(textra_typeface_options_t*),
    textra_status_t(TEXTRA_CALL* initialize_canvas)(textra_canvas_options_t*),
    textra_rendering_backend_t backend) {
  CHECK(argc == 2);
  auto hb_font = skity::Typeface::MakeFromFile(argv[1]);
  CHECK(hb_font);
  auto ct_font = CTFontCreateWithName(CFSTR("Helvetica"), 20, nullptr);
  CHECK(ct_font);
  OK(textra_text_layout_preload(backend));
  textra_font_manager_options_t mo{};
  mo.struct_size = sizeof(mo);

  OK(initialize_manager(&mo));
  textra_text_layout_options_t lo{};
  lo.struct_size = sizeof(lo);
  lo.backend = backend;
  lo.default_font_manager = &mo;
  textra_text_layout_t* layout = nullptr;
  OK(textra_text_layout_create(&lo, &layout));
  textra_typeface_options_t fo{};
  fo.struct_size = sizeof(fo);
  fo.backend = backend;
  fo.native_font = backend == TEXTRA_RENDERING_BACKEND_SKITY
                       ? static_cast<const void*>(hb_font.get())
                       : ct_font;
  OK(initialize_face(&fo));
  textra_font_descriptor_t* descriptor = nullptr;
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_set_typeface(descriptor, &fo));
  auto face = descriptor->typeface.get();
  auto engine_manager = layout->fonts.GetDefaultFontManager();
  const void* token = engine_manager->getPlatformFontFromTypeface(face->impl);
  CHECK(token);
  auto roundtrip = engine_manager->createTypefaceFromPlatformFont(token);
  CHECK(roundtrip);
  if (backend == TEXTRA_RENDERING_BACKEND_SKITY) CHECK(roundtrip == face->impl);
  textra_style_t* style = nullptr;
  OK(textra_style_create(&style));
  OK(textra_style_set_font_descriptor(style, descriptor));
  OK(textra_style_set_text_size(style, 20));
  OK(textra_style_set_foreground_color(style, 0xff000000u));
  textra_paragraph_options_t po{};
  po.struct_size = sizeof(po);
  po.type = TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE;
  textra_paragraph_t* paragraph = nullptr;
  OK(textra_paragraph_create(&po, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, style, "office ffi", 10));
  textra_text_context_t* context = nullptr;
  OK(textra_text_context_create(&context));
  OK(textra_text_layout_layout_single_line(layout, paragraph, context));
  skity::Bitmap bitmap(256, 80);
  auto native_canvas = skity::Canvas::MakeSoftwareCanvas(&bitmap);
  CHECK(native_canvas);
  textra_canvas_options_t co{};
  co.struct_size = sizeof(co);
  co.backend = backend;
  co.native_canvas = native_canvas.get();
  OK(initialize_canvas(&co));
  textra_layout_drawer_options_t opt{sizeof(opt), &co};
  textra_layout_drawer_t* drawer = nullptr;
  OK(textra_layout_drawer_create(&opt, &drawer));
  const size_t size = bitmap.RowBytes() * bitmap.Height();
  std::memset(bitmap.GetPixelAddr(), 0, size);
  OK(textra_layout_drawer_draw_single_line(drawer, paragraph, 10, 40));
  std::vector<unsigned char> pixels(bitmap.GetPixelAddr(),
                                    bitmap.GetPixelAddr() + size);
  bool nonzero = false;
  for (auto pixel : pixels) nonzero |= pixel != 0;
  CHECK(nonzero);
  std::memset(bitmap.GetPixelAddr(), 0, size);
  OK(textra_layout_drawer_draw_single_line(drawer, paragraph, 10, 40));
  CHECK(!std::memcmp(pixels.data(), bitmap.GetPixelAddr(), size));
  // Explicit CTFont imports and manager matching both cover fallback scripts.
  const char* samples[] = {"office ffi", "中文字体回退", "مرحبا بالعالم",
                           "Latin 中文 مرحبا"};
  for (const char* sample : samples) {
    for (bool explicit_font : {false, true}) {
      OK(textra_text_context_reset(context));
      textra_paragraph_t* fallback = nullptr;
      OK(textra_paragraph_create(&po, &fallback));
      OK(textra_paragraph_add_text_run(
          fallback, explicit_font ? style : nullptr, sample,
          static_cast<uint32_t>(std::strlen(sample))));
      OK(textra_text_layout_layout_single_line(layout, fallback, context));
      std::memset(bitmap.GetPixelAddr(), 0, size);
      OK(textra_layout_drawer_draw_single_line(drawer, fallback, 10, 40));
      pixels.assign(bitmap.GetPixelAddr(), bitmap.GetPixelAddr() + size);
      nonzero = false;
      for (auto pixel : pixels) nonzero |= pixel != 0;
      CHECK(nonzero);
      std::memset(bitmap.GetPixelAddr(), 0, size);
      OK(textra_layout_drawer_draw_single_line(drawer, fallback, 10, 40));
      CHECK(!std::memcmp(pixels.data(), bitmap.GetPixelAddr(), size));
      textra_paragraph_destroy(fallback);
    }
  }
  textra_layout_drawer_destroy(drawer);
  textra_text_context_destroy(context);
  textra_text_layout_destroy(layout);
  textra_paragraph_destroy(paragraph);
  textra_style_destroy(style);
  textra_font_descriptor_destroy(descriptor);
  CFRelease(ct_font);
  std::puts(backend == TEXTRA_RENDERING_BACKEND_SKITY
                ? "Skity/HarfBuzz layout and repeated draw tests passed"
                : "Skity/CoreText layout and repeated draw tests passed");
  return 0;
}

#endif  // TEST_RENDERING_BACKEND_SKITY_CASES_HPP_
