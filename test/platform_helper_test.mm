// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// This consumer intentionally includes only public Textra headers and native
// graphics SDK headers. Concrete backend types must never be needed here.
#import <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/platform_helper.h>
#include <textra/tttext_context.h>

#include <cstdio>
#include <cstdlib>

#define CHECK(condition)                                                   \
  do {                                                                     \
    if (!(condition)) {                                                    \
      std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      std::abort();                                                        \
    }                                                                      \
  } while (0)

int main() {
  using ttoffice::tttext::PlatformHelper;
  using ttoffice::tttext::RenderingBackend;
  static_assert(sizeof(RenderingBackend) == sizeof(textra_rendering_backend_t));
  static_assert(static_cast<uint32_t>(RenderingBackend::kSkityCoreText) ==
                TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT);
  auto manager = PlatformHelper::CreateFontManager();
  auto native_font = CTFontCreateWithName(CFSTR("Helvetica"), 20, nullptr);
  auto font = PlatformHelper::CreateTypefaceHelper(native_font, manager);
  CHECK(font);
  CFRelease(native_font);
  const auto& metrics = font->GetFontInfo(20);
  CHECK(metrics.GetFontSize() == 20);
  CHECK(metrics.GetDescent() >= 0);

  unsigned char pixels[64 * 64 * 4]{};
  auto color_space = CGColorSpaceCreateDeviceRGB();
  auto native_canvas = CGBitmapContextCreate(
      pixels, 64, 64, 8, 64 * 4, color_space, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(color_space);
  CHECK(native_canvas);
  auto canvas = PlatformHelper::CreateCanvasHelper(native_canvas);
  CHECK(canvas);
  canvas->DrawColor(0xffff0000u);
  CHECK(pixels[0] == 255 && pixels[1] == 0 && pixels[2] == 0 &&
        pixels[3] == 255);
  canvas.reset();
  // Destroying the adapter must not release a borrowed CGContext.
  CGContextSetRGBFillColor(native_canvas, 0, 1, 0, 1);
  CGContextFillRect(native_canvas, CGRectMake(0, 0, 64, 64));
  CHECK(pixels[0] == 0 && pixels[1] == 255);
  CGContextRelease(native_canvas);
  for (auto backend :
       {RenderingBackend::kMacOS, RenderingBackend::kMacOSSelfRendering}) {
    CHECK(PlatformHelper::IsBackendAvailable(backend));
    CHECK(ttoffice::tttext::TextLayout::Preload(backend));
    auto fonts = PlatformHelper::CreateFontManager();
    ttoffice::tttext::FontmgrCollection collection(fonts);
    auto layout = PlatformHelper::CreateTextLayout(backend, &collection);
    CHECK(layout && layout->IsValid());
    auto paragraph = ttoffice::tttext::Paragraph::Create(
        ttoffice::tttext::ParagraphType::kSingleLine);
    paragraph->AddTextRun(nullptr, "Backend ligatures ffi", 21);
    ttoffice::tttext::TTTextContext context;
    CHECK(layout->LayoutSingleLine(paragraph.get(), context));
  }
  ttoffice::tttext::FontmgrCollection empty_collection;
  CHECK(!PlatformHelper::CreateTextLayout(RenderingBackend::kMacOS, nullptr));
  CHECK(!PlatformHelper::CreateTextLayout(RenderingBackend::kInvalid,
                                          &empty_collection));
  std::puts("Public host platform adapter tests passed");
}
