// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_LVGL_LVGL_SHAPER_H_
#define SRC_PORTS_SHAPER_LVGL_LVGL_SHAPER_H_
#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class LVGLShaper : public TTShaper {
 public:
  explicit LVGLShaper(FontmgrCollection font_collection) noexcept;
  ~LVGLShaper() override = default;
  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_LVGL_LVGL_SHAPER_H_
