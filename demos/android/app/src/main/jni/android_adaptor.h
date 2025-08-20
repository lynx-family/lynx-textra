// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_ANDROID_ADAPTOR_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_ANDROID_ADAPTOR_H_
#include <textra/painter.h>
#include <textra/run_delegate.h>
/**
 * A concrete implementation of LayoutObject for Mac platform
 * Mainly responds to external calls, returns the size of the object,
 * then draws the corresponding content in Draw. Here the image
 * is simplified to draw a rectangle of the same size
 */
class ImageObject : public TTRunDelegate {
 public:
  void SetSize(float w, float h) {
    w_ = w;
    h_ = h;
  };
  float GetAscent() const override { return -h_; }
  float GetDescent() const override { return 0; }
  float GetAdvance() const override { return w_; }
  void Layout() override {}
  void Draw(ttoffice::ICanvasHelper* canvas, float x, float y) override {
    TTPainter painter;
    painter.SetColor(0xFF00FF00);
    painter.SetFillStyle(ttoffice::FillStyle::kStrokeAndFill);
    // canvas->DrawRect(0, 0, w_, h_, &painter);
  }

 private:
  float w_;
  float h_;
};

class TestShape : public ttoffice::RunDelegate {
 private:
  float width_ = 20;
  float height_ = 20;

 public:
  TestShape() = default;
  TestShape(float width, float height) : width_(width), height_(height) {}
  float GetAscent() const override { return -height_; }
  float GetDescent() const override { return 0; }
  float GetAdvance() const override { return width_; }
  void Draw(ttoffice::ICanvasHelper* canvas, float x, float y) override {
    auto paint = std::make_unique<TTPainter>();
    paint->SetFillStyle(ttoffice::FillStyle::kFill);
    paint->SetColor(0xFF00FF00);
    float left = 0;
    float top = 0;
    canvas->DrawRect(left, top, left + width_, top + height_, paint.get());
  }
};
#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_ANDROID_ADAPTOR_H_
