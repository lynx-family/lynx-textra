// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
#define SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
#include <lvgl/src/font/lv_font.h>

#include <cmath>

#include "src/ports/native/lvgl/lvgl_typeface.h"

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
  if (font_size - size < 0.0001f) {             \
    current_font_size_ = size;                  \
    current_font_ = &lv_font_montserrat_##size; \
    return current_font_;                       \
  }

  const lv_font_t* GetLVFontBySize(float font_size) const {
    if (current_font_ && std::fabs(font_size - current_font_size_) < 0.0001f)
      return current_font_;

#if LV_FONT_MONTSERRAT_12
    CHECK_MONTSERRAT_SIZE(12)
#endif
#if LV_FONT_MONTSERRAT_14
    CHECK_MONTSERRAT_SIZE(14)
#endif
#if LV_FONT_MONTSERRAT_16
    CHECK_MONTSERRAT_SIZE(16)
#endif
#if LV_FONT_MONTSERRAT_18
    CHECK_MONTSERRAT_SIZE(18)
#endif
#if LV_FONT_MONTSERRAT_20
    CHECK_MONTSERRAT_SIZE(20)
#endif
#if LV_FONT_MONTSERRAT_22
    CHECK_MONTSERRAT_SIZE(22)
#endif
#if LV_FONT_MONTSERRAT_24
    CHECK_MONTSERRAT_SIZE(24)
#endif
#if LV_FONT_MONTSERRAT_26
    CHECK_MONTSERRAT_SIZE(26)
#endif
#if LV_FONT_MONTSERRAT_28
    CHECK_MONTSERRAT_SIZE(28)
#endif
#if LV_FONT_MONTSERRAT_30
    CHECK_MONTSERRAT_SIZE(30)
#endif
#if LV_FONT_MONTSERRAT_32
    CHECK_MONTSERRAT_SIZE(32)
#endif
#if LV_FONT_MONTSERRAT_34
    CHECK_MONTSERRAT_SIZE(34)
#endif
#if LV_FONT_MONTSERRAT_36
    CHECK_MONTSERRAT_SIZE(36)
#endif
#if LV_FONT_MONTSERRAT_38
    CHECK_MONTSERRAT_SIZE(38)
#endif
#if LV_FONT_MONTSERRAT_40
    CHECK_MONTSERRAT_SIZE(40)
#endif
#if LV_FONT_MONTSERRAT_42
    CHECK_MONTSERRAT_SIZE(42)
#endif
#if LV_FONT_MONTSERRAT_44
    CHECK_MONTSERRAT_SIZE(44)
#endif
#if LV_FONT_MONTSERRAT_46
    CHECK_MONTSERRAT_SIZE(46)
#endif
#if LV_FONT_MONTSERRAT_48
    CHECK_MONTSERRAT_SIZE(48)
#endif
#if LV_FONT_MONTSERRAT_48
    return &lv_font_montserrat_48;
#else
    return LV_FONT_DEFAULT;
#endif
  }
#undef CHECK_MONTSERRAT_SIZE

 private:
  mutable float current_font_size_ = 0;
  mutable const lv_font_t* current_font_ = nullptr;
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_LVGL_LVGL_TYPEFACE_MONTSERRAT_H_
