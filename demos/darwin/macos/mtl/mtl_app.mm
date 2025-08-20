// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>
#import <Metal/Metal.h>

#include "mtl_app.h"
#include "skity/gpu/gpu_context_mtl.h"
#include "skity_adaptor.h"

static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

namespace example {
MTLApp::MTLApp(int32_t width, int32_t height, std::string name,
               const skity::Vec4& clear_color)
    : width_(width), height_(height), name_(name), clear_color_(clear_color) {}

MTLApp::~MTLApp() = default;

void MTLApp::Run() {
  Init();
  RunLoop();
  Destroy();
}

void MTLApp::KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  // Get the instance pointer from the window's user pointer
  printf("key:%d, scancode:%d, action:%d, mods:%d\n", key, scancode, action,
         mods);
  auto* app = static_cast<MTLApp*>(glfwGetWindowUserPointer(window));
  if (app && action == GLFW_RELEASE) {
    app->OnHandleKey(key, scancode, action, mods);
  }
}

void MTLApp::Init() {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ = glfwCreateWindow(width_, height_, name_.c_str(), NULL, NULL);

  if (!window_) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, KeyCallback);

  InitMetalLayer();

  id<MTLDevice> device = mtlLayer_.device;

  ctx_ = skity::MTLContextCreate(device, [device newCommandQueue]);

  auto font_mgr = skity::FontManager::RefDefault();
  skity::FontManager::RefDefault()->SetDefaultTypeface(TF_DEFAULT);

  OnStart();
}

void MTLApp::InitMetalLayer() {
  mtlLayer_ = [CAMetalLayer layer];

  mtlLayer_.device = MTLCreateSystemDefaultDevice();
  mtlLayer_.opaque = YES;
  mtlLayer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
  mtlLayer_.contentsScale = [[NSScreen mainScreen] backingScaleFactor];
  mtlLayer_.colorspace = CGColorSpaceCreateDeviceRGB();

  nsWindow_ = glfwGetCocoaWindow(window_);
  nsWindow_.contentView.layer = mtlLayer_;
  nsWindow_.contentView.wantsLayer = YES;
}

void MTLApp::CreateSurface() {
  skity::GPUSurfaceDescriptorMTL desc{};
  desc.backend = skity::GPUBackendType::kMetal;
  desc.width = mtlLayer_.frame.size.width;
  desc.height = mtlLayer_.frame.size.height;
  desc.content_scale = mtlLayer_.contentsScale;
  desc.sample_count = DeviceSampleCount();
  desc.surface_type = skity::MTLSurfaceType::kLayer;
  desc.layer = mtlLayer_;

  surface_ = ctx_->CreateSurface(&desc);
}

int32_t MTLApp::DeviceSampleCount() {
  // https://developer.apple.com/documentation/metal/mtldevice/1433355-supportstexturesamplecount
  int32_t sampleCount = 1;
#if defined(TARGET_OS_OSX)
  sampleCount = 8;
#elif defined(TARGET_IOS) || defined(TARGET_TVOS)
  sampleCount = 4;
#endif
  while (![MTLCreateSystemDefaultDevice()
      supportsTextureSampleCount:sampleCount]) {
    sampleCount /= 2;
  }
  return sampleCount;
}

void MTLApp::RunLoop() {
  while (!glfwWindowShouldClose(window_)) {
    @autoreleasepool {
      if (surface_ == nullptr) {
        CreateSurface();
      }

      // clear screen with user specified color
      auto canvas = GetCanvas();

      canvas->DrawColor(clear_color_, skity::BlendMode::kSrc);

      OnUpdate(0.f);

      surface_->Flush();

      canvas_ = nullptr;

      glfwPollEvents();
    }
  }
}

void MTLApp::Destroy() {
  OnDestroy();
  glfwDestroyWindow(window_);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void MTLApp::GetCursorPos(double& x, double& y) {
  double mx, my;
  glfwGetCursorPos(window_, &mx, &my);

  x = mx;
  y = my;
}

skity::Canvas* MTLApp::GetCanvas() {
  if (canvas_ == nullptr && surface_) {
    canvas_ = surface_->LockCanvas();
  }

  return canvas_;
}
}  // namespace example
