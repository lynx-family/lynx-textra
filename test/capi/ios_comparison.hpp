// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.

#ifndef TEST_CAPI_IOS_COMPARISON_HPP_
#define TEST_CAPI_IOS_COMPARISON_HPP_

#include <CoreText/CoreText.h>
#include <ImageIO/ImageIO.h>
#include <textra/platform_helper.h>

#include <string>
#include <vector>

#include "demos/capi/textra_demo_cpp.hpp"
namespace textra_demo {
inline bool SaveIosPng(CGContextRef cg, const std::string& path) {
  auto image = CGBitmapContextCreateImage(cg);
  auto url = CFURLCreateFromFileSystemRepresentation(
      nullptr, reinterpret_cast<const UInt8*>(path.data()), path.size(), false);
  auto dest =
      CGImageDestinationCreateWithURL(url, CFSTR("public.png"), 1, nullptr);
  bool ok = false;
  if (dest) {
    CGImageDestinationAddImage(dest, image, nullptr);
    ok = CGImageDestinationFinalize(dest);
    CFRelease(dest);
  }
  CFRelease(url);
  CGImageRelease(image);
  return ok;
}
/* Both paths use the same native font and fresh, identically configured
 * bitmaps. Compare raw pixels, geometry and a second C render. Save all images.
 */
inline bool RunIosComparison(const std::string& directory) {
  using namespace ttoffice::tttext;
  constexpr auto backend = TEXTRA_RENDERING_BACKEND_IOS;
  auto status = TEXTRA_STATUS_OK;
  if (status) return false;
  auto native_font = CTFontCreateWithName(CFSTR("Helvetica"), 24, nullptr);
  textra_typeface_options_t face{sizeof(face), backend, 0, native_font};
  auto cpp_manager = PlatformHelper::CreateFontManager();
  FILE* report = fopen((directory + "/results.json").c_str(), "w");
  bool all = !status && report;
  if (report) fputs("[\n", report);
  for (uint32_t i = 0; all && i < TEXTRA_DEMO_CASE_COUNT; ++i) {
    std::vector<uint8_t> cpp(320 * 200 * 4), c(cpp.size()), repeat(cpp.size());
    auto space = CGColorSpaceCreateDeviceRGB();
    auto make = [&](std::vector<uint8_t>& pixels) {
      auto context =
          CGBitmapContextCreate(pixels.data(), 320, 200, 8, 320 * 4, space,
                                kCGImageAlphaPremultipliedLast);
      // Match UIKit's downward Y axis when drawing into an offscreen bitmap.
      if (context) {
        CGContextTranslateCTM(context, 0, 200);
        CGContextScaleCTM(context, 1, -1);
      }
      return context;
    };
    auto a = make(cpp), b = make(c), r = make(repeat);
    CGColorSpaceRelease(space);
    textra_demo_metrics_t ma{}, mb{}, mr{};
    const void* font = i == TEXTRA_DEMO_CASE_COUNT - 1 ? nullptr : native_font;
    bool old_ok = false;
    if (a && b && r) {
      auto old_canvas = PlatformHelper::CreateCanvasHelper(a);
      old_ok = RenderCpp(static_cast<RenderingBackend>(backend), cpp_manager,
                         font, old_canvas.get(), i, 320, 200, &ma);
      for (int pass = 0; pass < 2; ++pass) {
        textra_canvas_options_t co{sizeof(co), backend, 0, pass ? r : b};
        textra_canvas_options_t canvas = co;
        if (!status)
          status = textra_demo_render(backend, nullptr, font ? &face : nullptr,
                                      &canvas, i, 320, 200, pass ? &mr : &mb);
        if (status) break;
      }
    } else
      status = TEXTRA_STATUS_OUT_OF_MEMORY;
    bool nonempty =
        std::any_of(c.begin(), c.end(), [](uint8_t v) { return v != 0; });
    bool pixels = cpp == c, repeated = c == repeat;
    bool metrics = EqualMetrics(ma, mb) && EqualMetrics(mb, mr);
    std::string prefix = directory + "/" + textra_demo_cases[i].name;
    bool saved = a && b && r && SaveIosPng(a, prefix + "-cpp.png") &&
                 SaveIosPng(b, prefix + "-capi.png") &&
                 SaveIosPng(r, prefix + "-repeat.png");
    if (report)
      fprintf(report,
              "%s{\"case\":\"%s\",\"status\":%d,\"cpp_ok\":%s,\"pixels_equal\":"
              "%s,\"repeat_equal\":%s,\"metrics_equal\":%s,\"nonempty\":%s}",
              i ? ",\n" : "", textra_demo_cases[i].name, status,
              old_ok ? "true" : "false", pixels ? "true" : "false",
              repeated ? "true" : "false", metrics ? "true" : "false",
              nonempty ? "true" : "false");
    all =
        old_ok && !status && pixels && repeated && metrics && nonempty && saved;
    if (a) CGContextRelease(a);
    if (b) CGContextRelease(b);
    if (r) CGContextRelease(r);
  }
  if (report) {
    fputs("\n]\n", report);
    fclose(report);
  }
  CFRelease(native_font);
  return all;
}
}  // namespace textra_demo
#endif  // TEST_CAPI_IOS_COMPARISON_HPP_
