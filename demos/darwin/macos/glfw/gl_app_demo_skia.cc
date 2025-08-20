// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "demos/darwin/macos/glfw/gl_app_demo_skia.h"

#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "src/ports/platform/skia/skia_canvas_helper.h"
#include "src/ports/platform/skia/skia_font_manager.h"
#include "src/ports/platform/skia/skia_typeface_helper.h"
namespace glapp {
GLAppDemoSkia::~GLAppDemoSkia() {}

void GLAppDemoSkia::OnStart() {
  GLApp::OnStart();
  auto interface = GrGLMakeNativeInterface();
  sContext = GrDirectContext::MakeGL(interface).release();

  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0;  // assume default framebuffer
  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  // (replace line below with this one to enable correct color spaces)
  // framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;

  SkColorType colorType = kRGBA_8888_SkColorType;
  GrBackendRenderTarget backendRenderTarget(width_, height_,
                                            0,  // sample count
                                            0,  // stencil bits
                                            framebufferInfo);

  // (replace line below with this one to enable correct color spaces) sSurface
  // = SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget,
  // kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(),
  // nullptr).release();
  sSurface = SkSurface::MakeFromBackendRenderTarget(
                 sContext, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                 colorType, nullptr, nullptr)
                 .release();
  if (sSurface == nullptr) abort();
}
void GLAppDemoSkia::OnUpdate(float elapsed_time) {
  GLApp::OnUpdate(elapsed_time);
  SkiaCanvasHelper canvas(GetCanvas());
  ParagraphTest paragraph_test;
  auto font_mgr = SkFontMgr::RefDefault();
  auto emoji_typeface = font_mgr->makeFromFile(EMOJI_FONT_PATH);
  paragraph_test.TestTTParagraph(
      &canvas,
      "💀🫱🏿‍🫲🏻🌴🐢🐐🍄⚽🫧👑📸👀🚨🏡🕊"
      "️"
      "🏆"
      "😻"
      "🌟"
      "🧿"
      "🍀🫶🏾🍜");
  GetCanvas()->flush();
}
void GLAppDemoSkia::OnDestroy() { GLApp::OnDestroy(); }
}  // namespace glapp

int main() {
  glapp::GLAppDemoSkia demo;
  demo.Run();
  return 0;
}
