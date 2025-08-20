// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_GLFW_GL_APP_DEMO_SKIA_H_
#define DEMOS_DARWIN_MACOS_GLFW_GL_APP_DEMO_SKIA_H_

#include "gl_app/gl_app.hpp"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "skia/include/core/SkCanvas.h"

namespace glapp {
class GLAppDemoSkia : public GLApp {
 public:
  GLAppDemoSkia() : GLApp(1000, 800, "GL Example") {}
  ~GLAppDemoSkia() override;

 public:
  void OnStart() override;
  void OnUpdate(float elapsed_time) override;
  void OnDestroy() override;

 public:
  SkCanvas* GetCanvas() { return &canvas_; }

 private:
  SkCanvas canvas_ = {};
  GrDirectContext* sContext = nullptr;
  SkSurface* sSurface = nullptr;
};
}  // namespace glapp
#endif  // DEMOS_DARWIN_MACOS_GLFW_GL_APP_DEMO_SKIA_H_
