// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkStream.h>
#include <include/encode/SkPngEncoder.h>
#include <include/ports/SkFontMgr_empty.h>
#include <textra/platform/skia/skia_canvas_helper.h>
#include <textra/platform/skia/skia_typeface_helper.h>

#include <cstdio>
#include <cstring>
#include <vector>

// Exercise the prebuilt C++ ABI through Textra's font and painter wrappers.
int main(int argc, char** argv) {
  if (argc != 3) return 1;
  auto manager = SkFontMgr_New_Custom_Empty();
  if (!manager) return 2;
  auto native = manager->makeFromFile(argv[1]);
  if (!native) return 3;
  tttext::SkiaTypefaceHelper face(native);
  native.reset();
  auto glyph = face.UnicharToGlyph('A');
  if (!glyph || face.GetHorizontalAdvance(glyph, 24) <= 0) return 4;
  SkBitmap bitmap;
  if (!bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(128, 64))) return 5;
  SkCanvas canvas(bitmap);
  SkiaCanvasHelper helper(&canvas);
  auto paint = helper.CreatePainter();
  paint->SetTextSize(24);
  paint->SetFillColor(tttext::TTColor::BLACK);
  const auto bytes = bitmap.computeByteSize();
  std::vector<unsigned char> first(bytes);
  for (int pass = 0; pass < 2; ++pass) {
    canvas.clear(SK_ColorWHITE);
    float x = 0, y = 0;
    helper.DrawGlyphs(&face, 1, &glyph, "A", 1, 4, 32, &x, &y, paint.get());
    if (!pass) {
      bool ink = false;
      for (int row = 0; row < bitmap.height(); ++row)
        for (int column = 0; column < bitmap.width(); ++column)
          ink |= bitmap.getColor(column, row) != SK_ColorWHITE;
      if (!ink) return 6;
      std::memcpy(first.data(), bitmap.getPixels(), bytes);
    } else if (std::memcmp(first.data(), bitmap.getPixels(), bytes)) {
      return 7;
    }
  }
  SkFILEWStream output(argv[2]);
  if (!SkPngEncoder::Encode(&output, bitmap.pixmap(), {})) return 8;
  std::puts(
      "PASS: prebuilt Skia font lifetime, metrics, repeated Textra draws and "
      "PNG encoding");
  return 0;
}
