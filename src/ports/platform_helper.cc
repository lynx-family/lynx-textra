// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <textra/platform_helper.h>

#include "src/capi/textra_backend_adapters.h"
#include "src/capi/textra_callback_utils.h"
#include "src/ports/native/native_helpers.h"
#include "src/ports/rendering_backend_internal.h"
namespace ttoffice {
namespace tttext {
bool PlatformHelper::IsBackendAvailable(RenderingBackend backend) {
  return internal::IsBackendCompiled(backend);
}
RenderingBackend PlatformHelper::GetDefaultBackend() {
#if defined(TEXTRA_ENABLE_LVGL_BACKEND)
  return RenderingBackend::kLVGL;
#elif defined(TEXTRA_ENABLE_ANDROID_BACKEND)
  return RenderingBackend::kAndroid;
#elif defined(TEXTRA_ENABLE_IOS_BACKEND)
  return RenderingBackend::kIOS;
#elif defined(TEXTRA_ENABLE_MACOS_BACKEND)
  return RenderingBackend::kMacOS;
#elif defined(TEXTRA_ENABLE_OHOS_BACKEND)
  return RenderingBackend::kOHOS;
#else
  return RenderingBackend::kInvalid;
#endif
}
FontManagerRef PlatformHelper::CreateFontManager(RenderingBackend backend) {
  switch (backend) {
#ifdef TEXTRA_ENABLE_LVGL_BACKEND
    case RenderingBackend::kLVGL:
      return native::LVGLFontManager();
#endif
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
    case RenderingBackend::kAndroid:
      return native::AndroidFontManager();
#endif
#ifdef TEXTRA_ENABLE_IOS_BACKEND
    case RenderingBackend::kIOS:
    case RenderingBackend::kIOSSelfRendering:
      return native::IOSFontManager();
#endif
#ifdef TEXTRA_ENABLE_MACOS_BACKEND
    case RenderingBackend::kMacOS:
    case RenderingBackend::kMacOSSelfRendering:
      return native::MacOSFontManager();
#endif
#ifdef TEXTRA_ENABLE_OHOS_BACKEND
    case RenderingBackend::kOHOS:
      return native::OHOSFontManager();
#endif
    default:
      return nullptr;
  }
}
std::unique_ptr<ICanvasHelper> PlatformHelper::CreateCanvasHelper(
    RenderingBackend backend, void* target, const FontManagerRef& manager) {
  if (!target) return nullptr;
  switch (backend) {
#ifdef TEXTRA_ENABLE_LVGL_BACKEND
    case RenderingBackend::kLVGL:
      return native::LVGLCanvas(target, manager);
#endif
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
    case RenderingBackend::kAndroid:
      return native::AndroidCanvas(target, manager);
#endif
#ifdef TEXTRA_ENABLE_IOS_BACKEND
    case RenderingBackend::kIOS:
    case RenderingBackend::kIOSSelfRendering:
      return native::IOSCanvas(target, manager);
#endif
#ifdef TEXTRA_ENABLE_MACOS_BACKEND
    case RenderingBackend::kMacOS:
    case RenderingBackend::kMacOSSelfRendering:
      return native::MacOSCanvas(target, manager);
#endif
#ifdef TEXTRA_ENABLE_OHOS_BACKEND
    case RenderingBackend::kOHOS:
      return native::OHOSCanvas(target, manager);
#endif
    default:
      return nullptr;
  }
}
TypefaceRef PlatformHelper::CreateTypefaceHelper(
    RenderingBackend backend, const void* font, const FontManagerRef& manager) {
  if (!font) return nullptr;
  auto fonts = manager ? manager : CreateFontManager(backend);
  if (!fonts) return nullptr;
  switch (backend) {
#ifdef TEXTRA_ENABLE_LVGL_BACKEND
    case RenderingBackend::kLVGL:
      return native::LVGLTypeface(font, fonts);
#endif
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
    case RenderingBackend::kAndroid:
      return native::AndroidTypeface(font, fonts);
#endif
#ifdef TEXTRA_ENABLE_IOS_BACKEND
    case RenderingBackend::kIOS:
    case RenderingBackend::kIOSSelfRendering:
      return native::IOSTypeface(font, fonts);
#endif
#ifdef TEXTRA_ENABLE_MACOS_BACKEND
    case RenderingBackend::kMacOS:
    case RenderingBackend::kMacOSSelfRendering:
      return native::MacOSTypeface(font, fonts);
#endif
#ifdef TEXTRA_ENABLE_OHOS_BACKEND
    case RenderingBackend::kOHOS:
      return native::OHOSTypeface(font, fonts);
#endif
    default:
      return nullptr;
  }
}

FontManagerRef PlatformHelper::CreateFontManager(
    const textra_font_manager_options_t& options, RenderingBackend selected) {
  if (!options.manager_callbacks) {
    if (options.typeface_callbacks || options.font_manager) return nullptr;
    return CreateFontManager(selected);
  }
  if (!backend::CallbackBackend(static_cast<uint32_t>(selected)) ||
      !options.typeface_callbacks)
    return nullptr;
  return std::make_shared<backend::CallbackFontManager>(options, selected);
}
std::unique_ptr<ICanvasHelper> PlatformHelper::CreateCanvasHelper(
    void* target, const FontManagerRef& manager) {
  return CreateCanvasHelper(GetDefaultBackend(), target, manager);
}
std::unique_ptr<ICanvasHelper> PlatformHelper::CreateCanvasHelper(
    const textra_canvas_options_t& options) {
  if (options.flags) return nullptr;
  auto selected = backend::ResolveBackend(options.backend);
  if (!options.callbacks)
    return CreateCanvasHelper(selected, options.native_canvas);
  if (!backend::CallbackBackend(static_cast<uint32_t>(selected)))
    return nullptr;
  return std::make_unique<backend::CallbackCanvas>(options);
}
TypefaceRef PlatformHelper::CreateTypefaceHelper(
    const void* native_font, const FontManagerRef& manager) {
  return CreateTypefaceHelper(GetDefaultBackend(), native_font, manager);
}
TypefaceRef PlatformHelper::CreateTypefaceHelper(
    const textra_typeface_options_t& options) {
  if (options.flags) return nullptr;
  auto selected = backend::ResolveBackend(options.backend);
  if (!options.callbacks)
    return CreateTypefaceHelper(selected, options.native_font);
  if (!backend::CallbackBackend(static_cast<uint32_t>(selected)))
    return nullptr;
  return backend::CreateTypeface(options);
}
}  // namespace tttext
}  // namespace ttoffice
