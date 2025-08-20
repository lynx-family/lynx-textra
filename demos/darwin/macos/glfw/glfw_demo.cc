// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

GrDirectContext* sContext = nullptr;
SkSurface* sSurface = nullptr;

void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void init_skia(int w, int h) {
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
  GrBackendRenderTarget backendRenderTarget(w, h,
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

void cleanup_skia() {
  delete sSurface;
  delete sContext;
}

const int kWidth = 960;
const int kHeight = 640;

int main(void) {
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // (uncomment to enable correct color spaces)
  // glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  window = glfwCreateWindow(kWidth, kHeight, "Simple example", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glfwMakeContextCurrent(window);
  // (uncomment to enable correct color spaces) glEnable(GL_FRAMEBUFFER_SRGB);

  init_skia(width, height);

  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);

  // Draw to the surface via its SkCanvas.
  SkCanvas* canvas =
      sSurface->getCanvas();  // We don't manage this pointer's lifetime.
  ParagraphTest paragraph_test;
  canvas->scale(2, 2);

  while (!glfwWindowShouldClose(window)) {
    glfwWaitEvents();
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    canvas->drawPaint(paint);
    //    paragraph_test.TestTTParagraph(canvas, "hello world");
    paragraph_test.TestDecoration(canvas, kWidth);
    sContext->flush();

    glfwSwapBuffers(window);
  }

  cleanup_skia();

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
