// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "ios_run_delegate.h"

#include "ios_canvas.h"
IOSRunDelegate::IOSRunDelegate(id<TTTextRunDelegate> run_delegate)
    : run_delegate_(run_delegate) {
  ascent_ = -[run_delegate_ GetAscent];
  descent_ = [run_delegate_ GetDescent];
  advance_ = [run_delegate_ GetWidth];
}
float IOSRunDelegate::GetAscent() const { return ascent_; }
float IOSRunDelegate::GetDescent() const { return descent_; }
float IOSRunDelegate::GetAdvance() const { return advance_; }
id<TTTextRunDelegate> IOSRunDelegate::GetRunDelegate() const {
  return run_delegate_;
}
void IOSRunDelegate::Draw(ttoffice::ICanvasHelper* canvas, float x, float y) {
  canvas->DrawRunDelegate(this, x, y, x + GetAdvance(),
                          y + GetDescent() - GetAscent(), nullptr);
}
bool IOSRunDelegate::NeedPlaceHolder() const {
  if ([run_delegate_ respondsToSelector:@selector(NeedPlaceHolder)]) {
    return run_delegate_.NeedPlaceHolder;
  }
  return true;
}
bool IOSRunDelegate::Equals(RunDelegate* other) {
  return run_delegate_ == ((IOSRunDelegate*)other)->run_delegate_;
}
