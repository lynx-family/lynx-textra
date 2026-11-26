// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef PUBLIC_TEXTRA_PLATFORM_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_HELPER_H_
#include <textra/capi/textra_platform.h>
#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>
#include <textra/macro.h>
#include <textra/rendering_backend.h>
#include <textra/text_layout.h>

#include <memory>
namespace ttoffice {
namespace tttext {
// Native platform implementations are constructed inside the engine.
// Skia/Skity drawing implementations remain host supplied.
class L_EXPORT PlatformHelper {
 public:
  static bool IsBackendAvailable(RenderingBackend backend);
  // Select a host-drawn shaping backend, preferring kSkia or kSkity when
  // SkShaper is compiled, then CoreText self-rendering when available.
  // The host supplies compatible canvas and font adapters (including CoreText
  // font access for the fallback). Other inputs or no compiled implementation
  // return kInvalid. Explicit backend factories never apply this fallback.
  static RenderingBackend GetSelfRenderingBackend(RenderingBackend preferred);
  // Concrete native backend compiled into this engine, or kInvalid.
  static RenderingBackend GetDefaultBackend();
  static FontManagerRef CreateFontManager(
      RenderingBackend backend = GetDefaultBackend());
  // Empty callback tables select the built-in native implementation. A custom
  // Skia/Skity implementation supplies an explicit backend and callback lists.
  static FontManagerRef CreateFontManager(
      const textra_font_manager_options_t& options,
      RenderingBackend backend = GetDefaultBackend());
  // target: CGContextRef, Android Canvas jobject, OH_Drawing_Canvas* or
  // LVGL lv_layer_t*.
  // Android retains its target; other targets are borrowed. Pass the layout's
  // Java manager when supplying a non-default Android font collection.
  static std::unique_ptr<ICanvasHelper> CreateCanvasHelper(
      RenderingBackend backend, void* target,
      const FontManagerRef& manager = nullptr);
  static std::unique_ptr<ICanvasHelper> CreateCanvasHelper(
      void* target, const FontManagerRef& manager = nullptr);
  static std::unique_ptr<ICanvasHelper> CreateCanvasHelper(
      const textra_canvas_options_t& options);
  // Follows original platform ownership: CoreText retains, Java registers,
  // OHOS/LVGL borrow their native fonts until all dependents/caches are
  // released.
  static TypefaceRef CreateTypefaceHelper(
      RenderingBackend backend, const void* native_font,
      const FontManagerRef& manager = nullptr);
  static TypefaceRef CreateTypefaceHelper(
      const void* native_font, const FontManagerRef& manager = nullptr);
  static TypefaceRef CreateTypefaceHelper(
      const textra_typeface_options_t& options);
  static std::unique_ptr<TextLayout> CreateTextLayout(RenderingBackend backend,
                                                      FontmgrCollection* fonts);
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_HELPER_H_
