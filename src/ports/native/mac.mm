// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#import <AppKit/AppKit.h>

#include "src/ports/native/ios/ios_font_manager.h"
#include "src/ports/native/mac/mac_canvas_base.h"
#include "src/ports/native/native_helpers.h"
namespace ttoffice {
namespace tttext {
namespace native {
FontManagerRef MacOSFontManager() {
  return std::make_shared<FontManagerCoreText>();
}
std::unique_ptr<ICanvasHelper> MacOSCanvas(void* target,
                                           const FontManagerRef&) {
  return std::make_unique<IOSCanvasBase>(static_cast<CGContextRef>(target));
}
TypefaceRef MacOSTypeface(const void* font, const FontManagerRef& manager) {
  return manager->createTypefaceFromPlatformFont(font);
}
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
