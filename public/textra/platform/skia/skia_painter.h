// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_PAINTER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_PAINTER_H_

#include <textra/painter.h>

#include <memory>

#include "include/core/SkPaint.h"
namespace ttoffice {
namespace tttext {
class SkiaPainter : public Painter {
 public:
  SkiaPainter() : Painter() {}

 public:
  std::unique_ptr<SkPaint> sk_paint_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_PAINTER_H_
