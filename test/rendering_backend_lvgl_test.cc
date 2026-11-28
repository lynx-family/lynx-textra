// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone consumer: use only public Textra headers and the host LVGL SDK.
// Link Textra's LVGL backend and the same LVGL configuration/library as the
// host.
#include <lvgl/lvgl.h>
#include <textra/capi/textra.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform_helper.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#define CHECK(x)                                      \
  do {                                                \
    if (!(x)) {                                       \
      std::fprintf(stderr, "%d: %s\n", __LINE__, #x); \
      std::abort();                                   \
    }                                                 \
  } while (0)
#define OK(x) CHECK((x) == TEXTRA_STATUS_OK)
using namespace ttoffice::tttext;
namespace {
struct Bitmap {
  lv_draw_buf_t* buffer =
      lv_draw_buf_create(64, 48, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
  lv_layer_t layer{};
  Bitmap() {
    CHECK(buffer);
    layer.draw_buf = buffer;
    layer.buf_area = {0, 0, 63, 47};
    layer._clip_area = layer.buf_area;
    layer.phy_clip_area = layer.buf_area;
    layer.color_format = LV_COLOR_FORMAT_ARGB8888;
    Clear();
  }
  ~Bitmap() {
    Drain();
    lv_draw_buf_destroy(buffer);
  }
  void Clear() { std::memset(buffer->data, 0, buffer->data_size); }
  void Drain() {
    for (int i = 0; layer.draw_task_head && i < 1000; ++i) {
      lv_draw_dispatch_layer(nullptr, &layer);
      lv_draw_wait_for_finish();
    }
    CHECK(!layer.draw_task_head);
  }
  std::vector<uint8_t> Pixels() {
    Drain();
    return {buffer->data, buffer->data + buffer->data_size};
  }
};
struct Metrics {
  float width, height;
  uint32_t lines;
};
Metrics RenderCpp(Bitmap& bitmap, const char* text, bool imported) {
  auto manager = PlatformHelper::CreateFontManager();
  CHECK(manager);
  FontmgrCollection fonts(manager);
  auto layout =
      PlatformHelper::CreateTextLayout(RenderingBackend::kLVGL, &fonts);
  CHECK(layout);
  auto paragraph = Paragraph::Create();
  Style style;
  style.SetTextSize(16);
  style.SetForegroundColor(TTColor(0xff202020));
  TypefaceRef face;
  if (imported) {
    face = PlatformHelper::CreateTypefaceHelper(LV_FONT_DEFAULT, manager);
    CHECK(face);
    FontDescriptor descriptor;
    descriptor.platform_font_ =
        reinterpret_cast<uintptr_t>(manager->getPlatformFontFromTypeface(face));
    CHECK(manager->createTypefaceFromPlatformFont(
              reinterpret_cast<void*>(descriptor.platform_font_)) == face);
    style.SetFontDescriptor(descriptor);
  }
  paragraph->AddTextRun(&style, text);
  TTTextContext context;
  LayoutRegion region(64, 48, LayoutMode::kDefinite, LayoutMode::kDefinite);
  CHECK(layout->Layout(paragraph.get(), &region, context) ==
        LayoutResult::kNormal);
  auto canvas = PlatformHelper::CreateCanvasHelper(&bitmap.layer, manager);
  CHECK(canvas);
  canvas->StartPaint();
  LayoutDrawer drawer(canvas.get());
  drawer.DrawLayoutPage(&region);
  canvas->EndPaint();
  bitmap.Drain();
  return {region.GetLayoutedWidth(), region.GetLayoutedHeight(),
          region.GetLineCount()};
}
Metrics RenderC(Bitmap& bitmap, const char* text, bool imported,
                bool explicit_manager) {
  textra_font_manager_options_t options{};
  options.struct_size = sizeof(options);
  textra_text_layout_options_t lo{};
  lo.struct_size = sizeof(lo);
  lo.default_font_manager = explicit_manager ? &options : nullptr;
  textra_text_layout_t* layout = nullptr;
  OK(textra_text_layout_create(&lo, &layout));
  textra_font_descriptor_t* descriptor = nullptr;
  OK(textra_font_descriptor_create(&descriptor));
  if (imported) {
    textra_typeface_options_t fo{};
    fo.struct_size = sizeof(fo);
    fo.native_font = LV_FONT_DEFAULT;
    OK(textra_font_descriptor_set_typeface(descriptor, &fo));
  }
  textra_style_t* style = nullptr;
  OK(textra_style_create(&style));
  OK(textra_style_set_text_size(style, 16));
  OK(textra_style_set_foreground_color(style, 0xff202020));
  OK(textra_style_set_font_descriptor(style, descriptor));
  textra_paragraph_options_t po{};
  po.struct_size = sizeof(po);
  po.type = TEXTRA_PARAGRAPH_TYPE_GENERAL;
  textra_paragraph_t* paragraph = nullptr;
  OK(textra_paragraph_create(&po, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, style, text,
                                   static_cast<uint32_t>(std::strlen(text))));
  textra_text_context_t* context = nullptr;
  OK(textra_text_context_create(&context));
  textra_layout_region_options_t ro{};
  ro.struct_size = sizeof(ro);
  ro.width = 64;
  ro.height = 48;
  ro.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  ro.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  textra_layout_region_t* region = nullptr;
  OK(textra_layout_region_create(&ro, &region));
  textra_layout_flow_t flow;
  OK(textra_text_layout_layout(layout, paragraph, region, context, &flow));
  CHECK(flow == TEXTRA_LAYOUT_FLOW_NORMAL);
  textra_layout_region_metrics_t metrics{};
  metrics.struct_size = sizeof(metrics);
  OK(textra_layout_region_get_metrics(region, &metrics));
  textra_canvas_options_t co{};
  co.struct_size = sizeof(co);
  co.native_canvas = &bitmap.layer;
  textra_layout_drawer_options_t draw{sizeof(draw), &co};
  textra_layout_drawer_t* drawer = nullptr;
  OK(textra_layout_drawer_create(&draw, &drawer));
  OK(textra_layout_drawer_draw_region(drawer, region));
  auto first = bitmap.Pixels();
  bitmap.Clear();
  OK(textra_layout_drawer_draw_region(drawer, region));
  CHECK(first == bitmap.Pixels());
  textra_layout_drawer_destroy(drawer);
  textra_layout_region_destroy(region);
  textra_text_context_destroy(context);
  textra_paragraph_destroy(paragraph);
  textra_style_destroy(style);
  textra_font_descriptor_destroy(descriptor);
  textra_text_layout_destroy(layout);
  // The borrowed layer/buffer is still usable after all engine owners release.
  bitmap.Clear();
  std::memcpy(bitmap.buffer->data, first.data(), first.size());
  return {metrics.width, metrics.height, metrics.line_count};
}
}  // namespace
int main() {
  lv_init();
  CHECK(PlatformHelper::GetDefaultBackend() == RenderingBackend::kLVGL);
  CHECK(PlatformHelper::IsBackendAvailable(RenderingBackend::kLVGL));
  CHECK(!PlatformHelper::CreateFontManager(RenderingBackend::kInvalid));
  CHECK(!PlatformHelper::CreateCanvasHelper(static_cast<void*>(nullptr)));
  CHECK(
      !PlatformHelper::CreateTypefaceHelper(static_cast<const void*>(nullptr)));
  auto manager = PlatformHelper::CreateFontManager();
  CHECK(manager && manager->countFamilies() > 0);
  auto face = PlatformHelper::CreateTypefaceHelper(LV_FONT_DEFAULT, manager);
  auto other = PlatformHelper::CreateTypefaceHelper(LV_FONT_DEFAULT, manager);
  CHECK(face && other && face->GetUniqueId() != other->GetUniqueId());
  lv_font_glyph_dsc_t d{};
  CHECK(lv_font_get_glyph_dsc(LV_FONT_DEFAULT, &d, 'A', 0));
  CHECK(face->UnicharToGlyph('A') == 'A');
  CHECK(std::fabs(face->GetHorizontalAdvance('A', 16) - d.adv_w) < .001f);
  {
    Bitmap bitmap;
    auto canvas = PlatformHelper::CreateCanvasHelper(&bitmap.layer);
    CHECK(canvas);
    std::memset(bitmap.buffer->data, 0x7f, bitmap.buffer->data_size);
    canvas->Clear();
    auto pixels = bitmap.Pixels();
    CHECK(std::all_of(pixels.begin(), pixels.end(),
                      [](uint8_t p) { return p == 0; }));
    canvas->Save();
    canvas->ClipRect(2, 3, 10, 12, false);
    CHECK(bitmap.layer._clip_area.x1 == 2 && bitmap.layer._clip_area.y1 == 3);
    canvas->Restore();
    CHECK(bitmap.layer._clip_area.x1 == 0 && bitmap.layer._clip_area.y1 == 0);
  }
  int count = 0;
  for (const char* text : {"AV office ffi", "First\nSecond", "Ab12"})
    for (bool imported : {false, true})
      for (bool explicit_manager : {false, true}) {
        Bitmap cpp, c;
        auto a = RenderCpp(cpp, text, imported);
        auto b = RenderC(c, text, imported, explicit_manager);
        CHECK(std::fabs(a.width - b.width) < .001f &&
              std::fabs(a.height - b.height) < .001f && a.lines == b.lines);
        auto pixels = cpp.Pixels();
        CHECK(pixels == c.Pixels());
        CHECK(std::any_of(pixels.begin(), pixels.end(),
                          [](uint8_t p) { return p != 0; }));
        ++count;
      }
  std::printf(
      "Built-in LVGL factories, native font import and borrowed ownership "
      "passed; %d C++/C layout, pixel and repeat cases matched\n",
      count);
}
