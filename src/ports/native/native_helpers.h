// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef SRC_PORTS_NATIVE_NATIVE_HELPERS_H_
#define SRC_PORTS_NATIVE_NATIVE_HELPERS_H_
#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>

#include <memory>
namespace ttoffice {
namespace tttext {
namespace native {
FontManagerRef AndroidFontManager();
std::unique_ptr<ICanvasHelper> AndroidCanvas(void* target,
                                             const FontManagerRef& manager);
TypefaceRef AndroidTypeface(const void* font, const FontManagerRef& manager);
FontManagerRef IOSFontManager();
std::unique_ptr<ICanvasHelper> IOSCanvas(void* target,
                                         const FontManagerRef& manager);
TypefaceRef IOSTypeface(const void* font, const FontManagerRef& manager);
FontManagerRef MacOSFontManager();
std::unique_ptr<ICanvasHelper> MacOSCanvas(void* target,
                                           const FontManagerRef& manager);
TypefaceRef MacOSTypeface(const void* font, const FontManagerRef& manager);
FontManagerRef OHOSFontManager();
std::unique_ptr<ICanvasHelper> OHOSCanvas(void* target,
                                          const FontManagerRef& manager);
TypefaceRef OHOSTypeface(const void* font, const FontManagerRef& manager);
FontManagerRef LVGLFontManager();
std::unique_ptr<ICanvasHelper> LVGLCanvas(void* target,
                                          const FontManagerRef& manager);
TypefaceRef LVGLTypeface(const void* font, const FontManagerRef& manager);
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_NATIVE_NATIVE_HELPERS_H_
