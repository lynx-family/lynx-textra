// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef I_DOCUMENT_VIEW_H
#define I_DOCUMENT_VIEW_H
#include <cstdint>

#include "i_canvas_helper.h"
#include "layout_definition.h"

namespace ttoffice {
class IDocumentView {
 public:
  virtual ~IDocumentView() = default;

 public:
  virtual int32_t OnLayout(float width, float height,
                           tttext::LayoutMode width_mode,
                           tttext::LayoutMode height_mode) {
    return 0;
  }
  virtual int32_t OnDraw(tttext::ICanvasHelper& helper) { return 0; }

 public:
  void SetDensity(float density) { density_ = density; }
  float GetDensity() const { return density_; }

 protected:
  float density_ = 1.0f;
};
}  // namespace ttoffice
#endif  // I_DOCUMENT_VIEW_H
