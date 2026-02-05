// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_CJK_H_
#define SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_CJK_H_
#include "lvgl/src/font/lv_font.h"
#include "textra/platform/lvgl/lvgl_typeface.h"

namespace ttoffice {
namespace tttext {
class LVGLTypefaceCJK : public LVGLTypeface {
 public:
  explicit LVGLTypefaceCJK(uint32_t unique_id) : LVGLTypeface(unique_id) {}
  ~LVGLTypefaceCJK() override = default;

 public:
  const lv_font_t* GetLvFont(float font_size) const override {
    return &lv_font_source_han_sans_sc_16_cjk;
  }

 private:
};

}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_CJK_H_
