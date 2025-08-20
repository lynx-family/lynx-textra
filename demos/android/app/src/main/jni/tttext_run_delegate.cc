// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "tttext_run_delegate.h"

#include <textra/i_canvas_helper.h>

TTTextRunDelegate::TTTextRunDelegate(int id, float ascent, float descent,
                                     float advance)
    : id_(id), ascent_(ascent), descent_(descent), advance_(advance) {}

void TTTextRunDelegate::Draw(tttext::ICanvasHelper* canvas, float x, float y) {
  tttext::Painter painter;
  canvas->DrawRunDelegate(this, x, y + ascent_, x + advance_, y + descent_,
                          &painter);
}
