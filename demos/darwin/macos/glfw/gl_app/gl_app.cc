// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "demos/darwin/macos/glfw/gl_app/gl_app.hpp"

#include <chrono>
#include <cstring>

#include "demos/darwin/macos/glfw/skity_adaptor.h"

void pixmap_swizzle_rb(skity::Pixmap* pixmap);

namespace example {

GLFWwindow* init_glfw_window(uint32_t width, uint32_t height, const char* title,
                             bool visible) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  if (!visible) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  }

  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (window == nullptr) {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  // if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
  // {
  //   exit(-3);
  // }

  return window;
}

GLApp::GLApp(int32_t width, int32_t height, std::string name,
             const skity::Vec4& clear_color, bool visible)
    : width_(width),
      height_(height),
      name_(name),
      clear_color_(clear_color),
      visible_(visible) {}

GLApp::~GLApp() = default;

void GLApp::Run() {
  Init();
  RunLoop();
  Destroy();
}

void GLApp::Init() {
  window_ = init_glfw_window(width_, height_, name_.c_str(), visible_);

  int32_t pp_width, pp_height;
  glfwGetFramebufferSize(window_, &pp_width, &pp_height);

  float density =
      static_cast<float>(pp_width * pp_width + pp_height * pp_height) /
      static_cast<float>(width_ * width_ + height_ * height_);
  screen_scale_ = std::sqrt(density);
  auto fbo = OnPreInit();
  ctx_ = skity::GLContextCreate(reinterpret_cast<void*>(glfwGetProcAddress));

  skity::GPUSurfaceDescriptorGL desc{};
  desc.backend = skity::GPUBackendType::kOpenGL;
  desc.width = width_;
  desc.height = height_;
  desc.sample_count = 1;
  desc.content_scale = screen_scale_;

  desc.surface_type = skity::GLSurfaceType::kFramebuffer;
  desc.gl_id = fbo;
  desc.has_stencil_attachment = true;

  surface_ = ctx_->CreateSurface(&desc);

  skity::FontManager::RefDefault()->SetDefaultTypeface(TF_DEFAULT);
  OnStart();
}

void GLApp::RunLoop() {
  while (!glfwWindowShouldClose(window_)) {
    canvas_ = surface_->LockCanvas();

    canvas_->DrawColor(clear_color_, skity::BlendMode::kSrc);

    this->OnUpdate(0.f);

    surface_->Flush();

    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

void GLApp::Destroy() {
  OnDestroy();
  surface_.reset();
}

skity::Canvas* GLApp::GetCanvas() const { return canvas_; }

void GLApp::GetCursorPos(double& x, double& y) {
  double mx, my;
  glfwGetCursorPos(window_, &mx, &my);

  x = mx;
  y = my;
}

void GLApp::SetShouldCloseWindow(int value) const {
  glfwSetWindowShouldClose(window_, value);
}

}  // namespace example
