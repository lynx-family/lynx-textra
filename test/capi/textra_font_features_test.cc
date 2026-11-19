// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <textra/capi/textra.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform_helper.h>
#include <textra/style.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

#define CHECK(condition)                                                   \
  do {                                                                     \
    if (!(condition)) {                                                    \
      std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      std::abort();                                                        \
    }                                                                      \
  } while (0)
#define OK(expression) CHECK((expression) == TEXTRA_STATUS_OK)

namespace {
namespace tt = ttoffice::tttext;
constexpr uint32_t kWidth = 640, kHeight = 180;
constexpr char kText[] = "office ffi AVATAR To office ffi";
struct Output {
  std::vector<uint8_t> pixels = std::vector<uint8_t>(kWidth * kHeight * 4);
  float width = 0, height = 0;
  uint32_t lines = 0;
};
CGContextRef MakeCanvas(Output& out) {
  auto space = CGColorSpaceCreateDeviceRGB();
  auto canvas =
      CGBitmapContextCreate(out.pixels.data(), kWidth, kHeight, 8, kWidth * 4,
                            space, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(space);
  CHECK(canvas);
  CGContextTranslateCTM(canvas, 0, kHeight);
  CGContextScaleCTM(canvas, 1, -1);
  return canvas;
}
Output RenderCpp(tt::TextLayout& layout, tt::RenderingBackend backend,
                 CTFontRef font,
                 const std::vector<textra_font_feature_t>& features,
                 bool single) {
  Output out;
  auto target = MakeCanvas(out);
  auto canvas = tt::PlatformHelper::CreateCanvasHelper(backend, target);
  CHECK(canvas);
  tt::FontDescriptor descriptor;
  descriptor.platform_font_ = reinterpret_cast<uintptr_t>(font);
  for (const auto& f : features)
    descriptor.font_feature_settings_.push_back(
        {{f.tag[0], f.tag[1], f.tag[2], f.tag[3]}, f.value});
  tt::Style style;
  style.SetFontDescriptor(descriptor);
  style.SetTextSize(32);
  style.SetForegroundColor(tt::TTColor{0xff203040});
  auto paragraph = tt::Paragraph::Create(single ? tt::ParagraphType::kSingleLine
                                                : tt::ParagraphType::kGeneral);
  paragraph->AddTextRun(&style, kText, sizeof(kText) - 1);
  tt::TTTextContext context;
  tt::LayoutDrawer drawer(canvas.get());
  if (single) {
    CHECK(layout.LayoutSingleLine(paragraph.get(), context));
    out.width = paragraph->GetWidth();
    out.height = paragraph->GetHeight();
    out.lines = 1;
    CHECK(drawer.DrawSingleLine(*paragraph, 0, 0));
  } else {
    tt::LayoutRegion region(210, kHeight, tt::LayoutMode::kDefinite,
                            tt::LayoutMode::kDefinite);
    CHECK(layout.Layout(paragraph.get(), &region, context) ==
          tt::LayoutResult::kNormal);
    out.width = region.GetLayoutedWidth();
    out.height = region.GetLayoutedHeight();
    out.lines = region.GetLineCount();
    drawer.DrawLayoutPage(&region);
  }
  CGContextRelease(target);
  return out;
}
Output RenderC(textra_text_layout_t* layout, textra_rendering_backend_t backend,
               CTFontRef font,
               const std::vector<textra_font_feature_t>& features,
               bool single) {
  Output out;
  auto target = MakeCanvas(out);
  textra_font_descriptor_t* descriptor = nullptr;
  textra_style_t* style = nullptr;
  textra_paragraph_t* paragraph = nullptr;
  textra_text_context_t* context = nullptr;
  textra_layout_region_t* region = nullptr;
  textra_layout_drawer_t* drawer = nullptr;
  OK(textra_font_descriptor_create(&descriptor));
  textra_typeface_options_t face{sizeof(face), backend, 0, font};
  OK(textra_font_descriptor_set_typeface(descriptor, &face));
  OK(textra_font_descriptor_set_font_features(
      descriptor, features.data(), static_cast<uint32_t>(features.size())));
  OK(textra_style_create(&style));
  OK(textra_style_set_font_descriptor(style, descriptor));
  OK(textra_style_set_text_size(style, 32));
  OK(textra_style_set_foreground_color(style, 0xff203040));
  textra_paragraph_options_t po{};
  po.struct_size = sizeof(po);
  po.type = single ? TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE
                   : TEXTRA_PARAGRAPH_TYPE_GENERAL;
  OK(textra_paragraph_create(&po, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, style, kText, sizeof(kText) - 1));
  OK(textra_text_context_create(&context));
  textra_canvas_options_t co{sizeof(co), backend, 0, target};
  textra_layout_drawer_options_t draw{sizeof(draw), &co};
  OK(textra_layout_drawer_create(&draw, &drawer));
  if (single) {
    OK(textra_text_layout_layout_single_line(layout, paragraph, context));
    textra_paragraph_metrics_t metrics{};
    metrics.struct_size = sizeof(metrics);
    OK(textra_paragraph_get_metrics(paragraph, &metrics));
    out.width = metrics.width;
    out.height = metrics.height;
    out.lines = 1;
    OK(textra_layout_drawer_draw_single_line(drawer, paragraph, 0, 0));
  } else {
    textra_layout_region_options_t ro{};
    ro.struct_size = sizeof(ro);
    ro.width = 210;
    ro.height = kHeight;
    ro.width_mode = ro.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
    OK(textra_layout_region_create(&ro, &region));
    textra_layout_flow_t flow = TEXTRA_LAYOUT_FLOW_NORMAL;
    OK(textra_text_layout_layout(layout, paragraph, region, context, &flow));
    CHECK(flow == TEXTRA_LAYOUT_FLOW_NORMAL);
    textra_layout_region_metrics_t metrics{};
    metrics.struct_size = sizeof(metrics);
    OK(textra_layout_region_get_metrics(region, &metrics));
    out.width = metrics.width;
    out.height = metrics.height;
    out.lines = metrics.line_count;
    OK(textra_layout_drawer_draw_region(drawer, region));
  }
  textra_layout_drawer_destroy(drawer);
  textra_layout_region_destroy(region);
  textra_text_context_destroy(context);
  textra_paragraph_destroy(paragraph);
  textra_style_destroy(style);
  textra_font_descriptor_destroy(descriptor);
  CGContextRelease(target);
  return out;
}
void Equal(const Output& a, const Output& b) {
  CHECK(a.pixels == b.pixels);
  CHECK(std::abs(a.width - b.width) < 0.001f);
  CHECK(std::abs(a.height - b.height) < 0.001f);
  CHECK(a.lines == b.lines);
  bool nonempty = false;
  for (auto byte : a.pixels) nonempty |= byte != 0;
  CHECK(nonempty);
}
}  // namespace

int main() {
  auto font = CTFontCreateWithName(CFSTR("Baskerville"), 32, nullptr);
  CHECK(font);
  const std::vector<std::vector<textra_font_feature_t>> cases = {
      {},
      {{{'l', 'i', 'g', 'a'}, 0}},
      {{{'l', 'i', 'g', 'a'}, 1}},
      {{{'k', 'e', 'r', 'n'}, 0}},
      {{{'k', 'e', 'r', 'n'}, 1}},
      {{{'l', 'i', 'g', 'a'}, 0},
       {{'k', 'e', 'r', 'n'}, 0},
       {{'c', 'v', '0', '1'}, 2}}};
  uint32_t comparisons = 0;
  for (auto backend : {tt::RenderingBackend::kMacOS,
                       tt::RenderingBackend::kMacOSSelfRendering}) {
    auto manager = tt::PlatformHelper::CreateFontManager(backend);
    tt::FontmgrCollection fonts(manager);
    tt::TextLayout cpp(&fonts, backend);
    textra_text_layout_options_t options{};
    options.struct_size = sizeof(options);
    options.backend = static_cast<textra_rendering_backend_t>(backend);
    textra_text_layout_t* c = nullptr;
    OK(textra_text_layout_create(&options, &c));
    for (bool single : {false, true}) {
      std::vector<Output> outputs;
      // Alternate feature values on the same engines to exercise their caches.
      for (auto index : {0, 1, 2, 3, 4, 5, 1, 0, 2}) {
        auto a = RenderCpp(cpp, backend, font, cases[index], single);
        auto b = RenderC(c, options.backend, font, cases[index], single);
        auto repeated = RenderC(c, options.backend, font, cases[index], single);
        Equal(a, b);
        Equal(b, repeated);
        outputs.push_back(std::move(b));
        ++comparisons;
      }
      CHECK(outputs[1].pixels != outputs[2].pixels);  // liga is effective.
      Equal(outputs[1], outputs[6]);
      Equal(outputs[0], outputs[7]);
      Equal(outputs[2], outputs[8]);
    }
    textra_text_layout_destroy(c);
  }
  CFRelease(font);
  tt::TextLayout::ClearShapeCache();
  std::printf("OpenType C/C++ pixel, metric and cache comparisons passed: %u\n",
              comparisons);
}
