// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
#define SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
#include <lvgl/src/font/lv_font.h>

#include "src/textlayout/utils/float_comparison.h"
#include "textra/platform/lvgl/lvgl_typeface.h"

namespace ttoffice {
namespace tttext {
class LVGLTypefaceMontserrat : public LVGLTypeface {
 public:
  explicit LVGLTypefaceMontserrat(uint32_t unique_id)
      : LVGLTypeface(unique_id) {}
  ~LVGLTypefaceMontserrat() override = default;

 public:
  const lv_font_t* GetLvFont(float font_size) const override {
    return GetLVFontBySize(font_size);
  }

 private:
#define CHECK_MONTSERRAT_SIZE(size)             \
  if (!FloatsLarger(font_size, size)) {         \
    current_font_size_ = size;                  \
    current_font_ = &lv_font_montserrat_##size; \
    return current_font_;                       \
  }

  const lv_font_t* GetLVFontBySize(float font_size) const {
    if (current_font_ && FloatsEqual(font_size, current_font_size_))
      return current_font_;

    CHECK_MONTSERRAT_SIZE(12)
    CHECK_MONTSERRAT_SIZE(14)
    CHECK_MONTSERRAT_SIZE(16)
    CHECK_MONTSERRAT_SIZE(18)
    CHECK_MONTSERRAT_SIZE(20)
    CHECK_MONTSERRAT_SIZE(22)
    CHECK_MONTSERRAT_SIZE(24)
    CHECK_MONTSERRAT_SIZE(26)
    CHECK_MONTSERRAT_SIZE(28)
    CHECK_MONTSERRAT_SIZE(30)
    CHECK_MONTSERRAT_SIZE(32)
    CHECK_MONTSERRAT_SIZE(34)
    CHECK_MONTSERRAT_SIZE(36)
    CHECK_MONTSERRAT_SIZE(38)
    CHECK_MONTSERRAT_SIZE(40)
    CHECK_MONTSERRAT_SIZE(42)
    CHECK_MONTSERRAT_SIZE(44)
    CHECK_MONTSERRAT_SIZE(46)
    CHECK_MONTSERRAT_SIZE(48)
    return &lv_font_montserrat_48;
  }
#undef CHECK_MONTSERRAT_SIZE

 private:
  mutable float current_font_size_ = 0;
  mutable const lv_font_t* current_font_ = nullptr;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
