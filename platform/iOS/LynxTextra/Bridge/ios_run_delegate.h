// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TEXTLAYOUTDEMO_TTTEXT_BRIDGE_IOS_RUN_DELEGATE_H_
#define TEXTLAYOUTDEMO_TTTEXT_BRIDGE_IOS_RUN_DELEGATE_H_

#include <CoreText/CoreText.h>

#import "TTTextRunDelegate.h"
#include "i_canvas_helper.h"
#include "run_delegate.h"

class IOSRunDelegate : public TTRunDelegate {
 public:
  IOSRunDelegate() = delete;
  IOSRunDelegate(id<TTTextRunDelegate> run_delegate);

 public:
  float GetAscent() const override;
  float GetDescent() const override;
  float GetAdvance() const override;
  id<TTTextRunDelegate> GetRunDelegate() const;
  void Draw(ttoffice::ICanvasHelper* canvas, float x, float y) override;
  bool NeedPlaceHolder() const;
  bool Equals(RunDelegate* other) override;

 private:
  id<TTTextRunDelegate> run_delegate_;
  float ascent_;
  float descent_;
  float advance_;
};

#endif  // TEXTLAYOUTDEMO_TTTEXT_BRIDGE_IOS_RUN_DELEGATE_H_
