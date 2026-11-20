// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <textra/platform_helper.h>

#include <cstdio>
#include <cstdlib>

#include "src/textlayout/tt_shaper.h"

#define CHECK(condition)            \
  do {                              \
    if (!(condition)) std::abort(); \
  } while (0)

int main() {
  using namespace ttoffice::tttext;
  auto manager = PlatformHelper::CreateFontManager();
  FontmgrCollection fonts(manager);
  auto native = TTShaper::CreateShaper(&fonts, RenderingBackend::kMacOS);
  auto self =
      TTShaper::CreateShaper(&fonts, RenderingBackend::kMacOSSelfRendering);
  CHECK(native && self);
  TTTextContext context;
  context.SetShaperCache(TTTextContext::CreateShaperCache());
  native->SetContext(context);
  self->SetContext(context);
  ShapeStyle style(FontDescriptor{}, 20);
  auto a = native->ShapeText(U"office ffi", 10, &style, false);
  auto b = self->ShapeText(U"office ffi", 10, &style, false);
  CHECK(a && b && a != b);
  CHECK(a == native->ShapeText(U"office ffi", 10, &style, false));
  CHECK(b == self->ShapeText(U"office ffi", 10, &style, false));
  // A C provider and a C++ adapter may share a backend but own different
  // typefaces. Independent collection generations must not exchange results.
  FontmgrCollection independent(manager);
  auto other = TTShaper::CreateShaper(&independent, RenderingBackend::kMacOS);
  other->SetContext(context);
  CHECK(a != other->ShapeText(U"office ffi", 10, &style, false));
  auto copy = TTShaper::CreateShaper(&fonts, RenderingBackend::kMacOS);
  copy->SetContext(context);
  CHECK(a == copy->ShapeText(U"office ffi", 10, &style, false));
  fonts.disableFontFallback();
  auto changed = TTShaper::CreateShaper(&fonts, RenderingBackend::kMacOS);
  changed->SetContext(context);
  CHECK(a != changed->ShapeText(U"office ffi", 10, &style, false));
  CHECK(!TTShaper::CreateShaper(&fonts, RenderingBackend::kInvalid));
  CHECK(!TextLayout::Preload(RenderingBackend::kInvalid));
  std::puts("Rendering backend cache isolation tests passed");
}
