// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef EXAMPLE_UTILS_GL_APP_HPP
#define EXAMPLE_UTILS_GL_APP_HPP

#include <glad/gl.h>
// this should after glad
#include <GLFW/glfw3.h>

#include <optional>
#include <skity/gpu/gpu_context_gl.hpp>
#include <skity/skity.hpp>
#include <string>

namespace example {

class GLApp {
 public:
  GLApp(int32_t width, int32_t height, std::string name,
        const skity::Vec4& clear_color = {0.3f, 0.4f, 0.5f, 1.f},
        bool visible = true);
  virtual ~GLApp();

  void Run();

  int32_t ScreenWidth() const { return width_; }
  int32_t ScreenHeight() const { return height_; }
  const std::string& GetName() const { return name_; }

  void SetShouldCloseWindow(int value) const;
  std::string& GetScreenShotPath() { return screenshot_path_; }

 protected:
  virtual int OnPreInit() { return 0; }
  virtual void OnStart() {}
  virtual void OnUpdate(float) {}
  virtual void OnDestroy() {}

  void GetCursorPos(double& x, double& y);

  skity::GPUContext* GetGPUContext() const { return ctx_.get(); }

  skity::Canvas* GetCanvas() const;

  float GetScreenScale() const { return screen_scale_; }

 private:
  void Init();
  void RunLoop();
  void Destroy();

 protected:
  int32_t width_ = {};
  int32_t height_ = {};
  float screen_scale_ = {};
  std::string name_ = {};
  GLFWwindow* window_ = {};
  skity::Vec4 clear_color_ = {};
  std::unique_ptr<skity::GPUContext> ctx_ = {};
  std::unique_ptr<skity::GPUSurface> surface_ = {};
  skity::Canvas* canvas_ = {};
  bool visible_;
  std::string screenshot_path_;
};

}  // namespace example

#endif  // EXAMPLE_UTILS_GL_APP_HPP
