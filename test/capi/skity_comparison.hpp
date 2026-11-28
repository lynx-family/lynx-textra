// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef TEST_CAPI_SKITY_COMPARISON_HPP_
#define TEST_CAPI_SKITY_COMPARISON_HPP_
#include "demos/capi/skity_capi_utils.hpp"
#include "demos/capi/textra_skity_demo.hpp"

namespace textra_demo {
inline bool SaveSkityImage(const std::string& path,
                           const std::vector<uint8_t>& pixels) {
  // PAM stores the unmodified RGBA bytes; the runner converts it to PNG.
  auto file = fopen(path.c_str(), "wb");
  if (!file) return false;
  fputs(
      "P7\nWIDTH 320\nHEIGHT 200\nDEPTH 4\nMAXVAL 255\nTUPLTYPE "
      "RGB_ALPHA\nENDHDR\n",
      file);
  bool ok = fwrite(pixels.data(), 1, pixels.size(), file) == pixels.size();
  fclose(file);
  return ok;
}
inline bool RunSkityComparison(const std::string& directory) {
  using namespace ttoffice::tttext;
  FILE* report = fopen((directory + "/results.json").c_str(), "w");
  if (!report) return false;
  fputs("[\n", report);
  bool all = true;
  auto manager = std::make_shared<SkityFontManager>();
  for (uint32_t i = 0; i < TEXTRA_DEMO_CASE_COUNT; ++i) {
    skity::Bitmap a(320, 200);
    auto old = skity::Canvas::MakeSoftwareCanvas(&a);
    auto b = skity_c::MakeBitmap(320, 200), r = skity_c::MakeBitmap(320, 200);
    auto current = skity_c::MakeCanvas(b.get()),
         repeated = skity_c::MakeCanvas(r.get());
    size_t size = a.RowBytes() * a.Height();
    if (skity_bitmap_get_row_bytes(b.get()) *
            skity_bitmap_get_height(b.get()) !=
        size) {
      fclose(report);
      return false;
    }
    memset(a.GetPixelAddr(), 0, size);
    memset(static_cast<uint8_t*>(skity_bitmap_get_pixels(b.get())), 0, size);
    memset(static_cast<uint8_t*>(skity_bitmap_get_pixels(r.get())), 0, size);
    textra_demo_metrics_t ma{}, mb{}, mr{};
    SkityCanvasHelper helper(old.get());
    bool old_ok = RenderCpp(RenderingBackend::kSkity, manager, nullptr, &helper,
                            i, 320, 200, &ma);
    auto status = TEXTRA_STATUS_OK;
    textra_font_manager_options_t fonts{};
    fonts.struct_size = sizeof(fonts);
    if (!status)
      status = textra_skity_font_manager_callbacks_initialize(&fonts);
    for (int pass = 0; pass < 2 && !status; ++pass) {
      textra_canvas_options_t canvas{};
      canvas.struct_size = sizeof(canvas);
      canvas.native_canvas = pass ? repeated.get() : current.get();
      status = textra_skity_canvas_callbacks_initialize(&canvas);
      if (!status)
        status =
            textra_demo_render(TEXTRA_RENDERING_BACKEND_SKITY, &fonts, nullptr,
                               &canvas, i, 320, 200, pass ? &mr : &mb);
    }
    std::vector<uint8_t> pa(a.GetPixelAddr(), a.GetPixelAddr() + size),
        pb(static_cast<uint8_t*>(skity_bitmap_get_pixels(b.get())),
           static_cast<uint8_t*>(skity_bitmap_get_pixels(b.get())) + size),
        pr(static_cast<uint8_t*>(skity_bitmap_get_pixels(r.get())),
           static_cast<uint8_t*>(skity_bitmap_get_pixels(r.get())) + size);
    bool pixels = pa == pb, repeat = pb == pr,
         metrics = EqualMetrics(ma, mb) && EqualMetrics(mb, mr),
         nonempty = std::any_of(pb.begin(), pb.end(),
                                [](uint8_t v) { return v != 0; });
    std::string prefix = directory + "/" + textra_demo_cases[i].name;
    bool saved = SaveSkityImage(prefix + "-cpp.pam", pa) &&
                 SaveSkityImage(prefix + "-capi.pam", pb) &&
                 SaveSkityImage(prefix + "-repeat.pam", pr);
    fprintf(report,
            "%s{\"case\":\"%s\",\"status\":%d,\"cpp_ok\":%s,\"pixels_equal\":%"
            "s,\"metrics_equal\":%s,\"repeat_equal\":%s,\"nonempty\":%s}",
            i ? ",\n" : "", textra_demo_cases[i].name, status,
            old_ok ? "true" : "false", pixels ? "true" : "false",
            metrics ? "true" : "false", repeat ? "true" : "false",
            nonempty ? "true" : "false");
    if (fonts.manager_callbacks)
      fonts.manager_callbacks->destroy(fonts.userdata,
                                       const_cast<void*>(fonts.font_manager));
    textra_text_layout_clear_shape_cache();
    all &=
        old_ok && !status && pixels && repeat && metrics && nonempty && saved;
  }
  fputs("\n]\n", report);
  fclose(report);
  return all;
}
}  // namespace textra_demo
#endif  // TEST_CAPI_SKITY_COMPARISON_HPP_
