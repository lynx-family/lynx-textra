/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "demos/darwin/macos/skia_app_demo/SkiaAppDemo.h"

#include <textra/platform/skia/skia_canvas_helper.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "demos/darwin/macos/skia_app_demo/SkiaAdaptor.h"
#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "src/ports/shaper/skshaper/shaper_skshaper.h"

using namespace sk_app;

#define RECORD_PICTURE 0

// std::unique_ptr<TTShaper> TTShaper::GetInstance() {
//   auto fc = std::make_unique<FontCollection>();
//   fc->SetDefaultFontManager(SkiaFontManager::RefDefault());
//   return std::make_unique<ShaperSkShaper>(std::move(fc));
// }

Application* Application::Create(int argc, char** argv, void* platformData) {
  return reinterpret_cast<Application*>(
      new SkiaAppDemo(argc, argv, platformData));
}

SkiaAppDemo::SkiaAppDemo(int argc, char** argv, void* platformData)
    : fBackendType(Window::kNativeGL_BackendType) /*, fRotationAngle(0)*/ {
  SkGraphics::Init();

  fWindow = Windows::CreateNativeWindow(platformData);
  fWindow->setRequestedDisplayParams(DisplayParams());

  // register callbacks
  fWindow->pushLayer(this);

  fWindow->attach(fBackendType);
  fWindow->inval();
}

SkiaAppDemo::~SkiaAppDemo() {
  fWindow->detach();
  delete fWindow;
}

void SkiaAppDemo::updateTitle() const {
  if (!fWindow) {
    return;
  }

  char title[512];
  snprintf(
      title, 512, "test id:%d, width:%5.2f, %s", test_id_, test_width_,
      Window::kRaster_BackendType == fBackendType ? " [Raster]" : " [OpenGL]");
  fWindow->setTitle(title);
}

void SkiaAppDemo::onBackendCreated() {
  this->updateTitle();
  fWindow->show();
}
std::string SkiaAppDemo::readFile(char* filename) {
  std::ifstream ifile(filename);
  std::ostringstream buff;
  char ch;
  while (buff && ifile.get(ch)) {
    buff.put(ch);
  }
  return buff.str();
}
void SkiaAppDemo::onPaint(SkSurface* surface) {
  clock_t start;
  start = clock();
#if RECORD_PICTURE
  SkPictureRecorder recoder;
  auto surface_canvas = surface->getCanvas();
  auto rect = surface_canvas->getDeviceClipBounds();
  auto skcanvas = recoder.beginRecording(rect.width(), rect.height());
#else
  auto* sk_canvas = surface->getCanvas();
#endif

  sk_canvas->save();
  sk_canvas->scale(2.0f, 2.0f);
  sk_canvas->clear(SK_ColorWHITE);

  SkiaCanvasHelper skia_canvas_helper(sk_canvas);

  auto font_mgr = std::make_shared<TestFontMgr>(
      std::vector<std::shared_ptr<ITypefaceHelper>>{TFH_DEFAULT});
  tttext::FontmgrCollection font_collection(font_mgr);
  ParagraphTest paragraph_test(kSelfRendering, &font_collection);
  paragraph_test.TestWithId(&skia_canvas_helper, test_width_, test_id_);
  //  SkPaint paint;
  //  paint.setColor(0xFF00FF00);
  //  sk_canvas->drawLine(test_width_, 0, test_width_, fWindow->height(),
  //  paint);
  sk_canvas->restore();

#if RECORD_PICTURE
  auto skpicture = recoder.finishRecordingAsPicture();
  surface_canvas->drawPicture(skpicture);
  SkFILEWStream dst("out.skp");
  skpicture->serialize(&dst);
  printf("dump out.skp");
#endif
}

bool SkiaAppDemo::onTouch(intptr_t owner, skui::InputState state, float x,
                          float y) {
  return Layer::onTouch(owner, state, x, y);
}

void SkiaAppDemo::onIdle() {
  // Just re-paint continously
  // fWindow->inval();
}

bool SkiaAppDemo::onChar(SkUnichar c, skui::ModifierKey modifiers) {
  char title[512];
  auto width_delta = 10;
  if (' ' == c) {
    fBackendType = Window::kRaster_BackendType == fBackendType
                       ? Window::kNativeGL_BackendType
                       : Window::kRaster_BackendType;
    fWindow->detach();
    fWindow->attach(fBackendType);
  } else if ('>' == c) {
    test_width_ += width_delta;
  } else if ('<' == c) {
    test_width_ -= width_delta;
    test_width_ = std::fmax(test_width_, width_delta);
  } else if (63234 == c) {
    // left arrow key
    if (test_id_ > 0) test_id_--;
  } else if (63235 == c) {
    // right arrow key
    test_id_++;
  } else if (63232 == c) {
    // up arrow key
  } else if (63233 == c) {
    // bottom arrow key
  }
  updateTitle();
  fWindow->inval();
  return true;
}
bool SkiaAppDemo::onMouseWheel(float delta, int x, int y,
                               skui::ModifierKey key) {
  fWindow->inval();
  return Layer::onMouseWheel(delta, x, y, key);
}
bool SkiaAppDemo::onMouse(int x, int y, skui::InputState state,
                          skui::ModifierKey key) {
  return Layer::onMouse(x, y, state, key);
}
