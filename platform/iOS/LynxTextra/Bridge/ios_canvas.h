// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_IOS_CANVAS_H_
#define TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_IOS_CANVAS_H_

#include <memory>

#import "TTTextDrawerDelegate.h"
#include "ios_canvas_base.h"

using namespace ttoffice;
class IOSCanvas : public IOSCanvasBase {
 public:
  IOSCanvas();
  explicit IOSCanvas(CGContextRef context);
  explicit IOSCanvas(CGContextRef context,
                     id<TTTextDrawerDelegate> run_delegate_drawer);
  ~IOSCanvas();

 public:
  void DrawRunDelegate(const RunDelegate* run_delegate, float left, float top,
                       float right, float bottom, TTPainter* painter) override;
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              TTPainter* painter) override;
  void DrawBlockRegion(textlayout::BlockRegion* block) override;

 private:
  id<TTTextDrawerDelegate> run_delegate_drawer_;
};

#endif  // TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_TEXTLAYOUTDEMO_IOS_CANVAS_H_
