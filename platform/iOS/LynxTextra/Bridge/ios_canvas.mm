// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "ios_canvas.h"

#include "ios_layout_drawer_listener.h"
#include "ios_run_delegate.h"
IOSCanvas::IOSCanvas() {
  context_ = UIGraphicsGetCurrentContext();
  run_delegate_drawer_ = nullptr;
}
IOSCanvas::IOSCanvas(CGContextRef context)
    : IOSCanvasBase(context), run_delegate_drawer_(nullptr) {}
IOSCanvas::IOSCanvas(CGContextRef context, id<TTTextDrawerDelegate> drawer)
    : IOSCanvasBase(context), run_delegate_drawer_(drawer) {}
IOSCanvas::~IOSCanvas() = default;
void IOSCanvas::DrawRunDelegate(const RunDelegate* run_delegate, float left,
                                float top, float right, float bottom,
                                TTPainter* painter) {
  if (!CheckValide()) return;
  auto* ios_run_delegate = (IOSRunDelegate*)run_delegate;
  if (run_delegate_drawer_ == nullptr) {
    TTTextRunDelegateImpl* impl =
        (TTTextRunDelegateImpl*)ios_run_delegate->GetRunDelegate();
    UIImage* image = (UIImage*)impl.GetAttachment;
    CGRect rect = CGRectMake(0, 0, right - left, bottom - top);
    CGContextSaveGState(context_);
    CGContextTranslateCTM(context_, 0, rect.size.height);
    CGContextScaleCTM(context_, image.size.width / rect.size.width,
                      -image.size.height / rect.size.height);
    CGContextDrawImage(context_, rect, image.CGImage);
    CGContextRestoreGState(context_);
  } else {
    // The rect passed here is the absolute rect relative to the screen
    CGRect rect = CGRectMake(left, top, right - left, bottom - top);
    [run_delegate_drawer_ DrawRunDelegate:ios_run_delegate->GetRunDelegate()
                                   InRect:rect];
  }
  CGContextResetClip(context_);
}
void IOSCanvas::DrawBackgroundDelegate(const RunDelegate* run_delegate,
                                       TTPainter* painter) {
  if (!CheckValide()) return;
  auto* ios_run_delegate = (IOSRunDelegate*)run_delegate;
  if (run_delegate_drawer_ != nullptr) {
    [run_delegate_drawer_
        DrawBackgroundDelegate:ios_run_delegate->GetRunDelegate()];
  }
  CGContextResetClip(context_);
}
void IOSCanvas::DrawBlockRegion(textlayout::BlockRegion* block) {}
