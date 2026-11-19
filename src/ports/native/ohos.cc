// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include "src/ports/native/ark_graphics/ag_canvas_helper.h"
#include "src/ports/native/ark_graphics/ag_font_manager.h"
#include "src/ports/native/native_helpers.h"
namespace ttoffice {
namespace tttext {
namespace native {
FontManagerRef OHOSFontManager() { return std::make_shared<AGFontManager>(); }
std::unique_ptr<ICanvasHelper> OHOSCanvas(void* target, const FontManagerRef&) {
  return std::make_unique<AGCanvasHelper>(target);
}
TypefaceRef OHOSTypeface(const void* font, const FontManagerRef& manager) {
  return manager->createTypefaceFromPlatformFont(font);
}
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
