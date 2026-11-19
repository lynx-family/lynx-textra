// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#import <UIKit/UIKit.h>

#include "src/ports/native/ios/ios_canvas_base.h"
#include "src/ports/native/ios/ios_font_manager.h"
#include "src/ports/native/native_helpers.h"
namespace ttoffice {
namespace tttext {
namespace native {
FontManagerRef IOSFontManager() {
  return std::make_shared<FontManagerCoreText>();
}
std::unique_ptr<ICanvasHelper> IOSCanvas(void* target, const FontManagerRef&) {
  return std::make_unique<IOSCanvasBase>(static_cast<CGContextRef>(target));
}
TypefaceRef IOSTypeface(const void* font, const FontManagerRef& manager) {
  return manager->createTypefaceFromPlatformFont(font);
}
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
