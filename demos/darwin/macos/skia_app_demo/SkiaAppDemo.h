/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAAPPDEMO_H_
#define DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAAPPDEMO_H_

#include <string>

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

class SkCanvas;
class SkiaAppDemo : public sk_app::Application, sk_app::Window::Layer {
 public:
  SkiaAppDemo(int argc, char** argv, void* platformData);
  ~SkiaAppDemo() override;

  void onIdle() override;

  void onBackendCreated() override;
  void onPaint(SkSurface*) override;
  bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;
  std::string readFile(char* filename);

 private:
  void updateTitle() const;

  bool onTouch(intptr_t owner, skui::InputState state, float x,
               float y) override;
  bool onMouseWheel(float delta, int x, int y, skui::ModifierKey) override;
  bool onMouse(int x, int y, skui::InputState state,
               skui::ModifierKey key) override;

  sk_app::Window* fWindow;
  sk_app::Window::BackendType fBackendType;

  std::string file_name_;
  std::string content_;
  float test_width_ = 200;
  uint32_t test_id_ = 21;
};

#endif  // DEMOS_DARWIN_MACOS_SKIA_APP_DEMO_SKIAAPPDEMO_H_
