// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_DARWIN_MACOS_MTL_MTL_APP_H_
#define DEMOS_DARWIN_MACOS_MTL_MTL_APP_H_

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "skity/skity.hpp"

namespace skity {
class GPUSurface;
}
@class NSWindow;
@class CAMetalLayer;
namespace example {

class MTLApp {
 public:
  MTLApp(int32_t width, int32_t height, std::string name,
         const skity::Vec4& clear_color = {0.3f, 0.4f, 0.5f, 1.f});
  virtual ~MTLApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }

  skity::GPUContext* GetGPUContext() const { return ctx_.get(); }

  // id<MTLDevice> GetDevice() const { return mtlLayer_.device; }

 protected:
  virtual void OnStart() {}
  virtual void OnUpdate(float elapsed_time) {}
  virtual void OnDestroy() {}
  virtual void OnHandleKey(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
  }

  void GetCursorPos(double& x, double& y);

  skity::Canvas* GetCanvas();

 public:
  void Init();
  void RunLoop();
  void Destroy();

 private:
  void InitMetalLayer();

  void CreateSurface();

  int32_t DeviceSampleCount();

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mods);

 private:
  int32_t width_ = {};
  int32_t height_ = {};

  std::string name_ = {};
  GLFWwindow* window_ = {};
  NSWindow* nsWindow_ = {};

  skity::Vec4 clear_color_ = {};
  CAMetalLayer* mtlLayer_ = {};

  std::unique_ptr<skity::GPUContext> ctx_ = {};
  std::unique_ptr<skity::GPUSurface> surface_ = {};
  skity::Canvas* canvas_ = {};
};

}  // namespace example
#endif  // DEMOS_DARWIN_MACOS_MTL_MTL_APP_H_
