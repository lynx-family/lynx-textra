// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/fontmgr_collection.h>
#include <textra/platform/skity/skity_canvas_helper.h>

#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "mtl_app.h"
#include "skity_adaptor.h"
using namespace tttext;

class MTLExample : public example::MTLApp {
 public:
  MTLExample()
      : example::MTLApp(800, 600, "MTL Shape Example", {1.f, 1.f, 1.f, 1.f}),
        font_collection_(nullptr) {
    auto font_mgr = std::make_shared<SkityTestFontManager>();
    font_collection_.SetDefaultFontManager(font_mgr);
  }

  ~MTLExample() override = default;

 protected:
  void OnUpdate(float) override {
    GetCanvas()->Save();
    GetCanvas()->DrawColor(0xFFFFFF00);
    SkityCanvasHelper canvas(GetCanvas());
    canvas.DrawColor(0xFFFFFFFF);
    skity::Paint paint;
    paint.SetFillColor(1.0, 0, 0, 1.0f);
    paint.SetStyle(skity::Paint::kFill_Style);
    ParagraphTest paragraph_test_(kSelfRendering, &font_collection_);
    paragraph_test_.TestWithId(&canvas, width_, test_id);
    GetCanvas()->Restore();
    GetCanvas()->Flush();
  }

  void OnHandleKey(int key, int scancode, int action, int mods) override {
    switch (key) {
      case GLFW_KEY_RIGHT:
        test_id++;
        break;
      case GLFW_KEY_LEFT:
        test_id--;
        break;
      case GLFW_KEY_RIGHT_BRACKET:
        width_ += mods == GLFW_MOD_SHIFT ? 50 : 10;
        break;
      case GLFW_KEY_LEFT_BRACKET:
        width_ -= mods == GLFW_MOD_SHIFT ? 50 : 10;
        break;
      default:
        example::MTLApp::OnHandleKey(key, scancode, action, mods);
    }
    printf("test_id: %d\n", test_id);
  }

 private:
  uint32_t test_id = 37;
  float width_ = 500;
  FontmgrCollection font_collection_;
};

int main(int, const char**) {
  MTLExample app;
  app.Run();
  return 0;
}
