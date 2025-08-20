// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_definition.h>
#include <textra/tt_color.h>

#include <string>

namespace ttoffice {
namespace tttext {
#ifdef THEME_COLOR
TTColor::TTColor()
    : type_(ColorType::kUndefined), r_(0), g_(0), b_or_id_(0), alpha_(0) {}
TTColor::TTColor(const TTColor& color)
    : type_(color.type_),
      r_(color.r_),
      g_(color.g_),
      b_or_id_(color.b_or_id_),
      alpha_(color.alpha_) {}
TTColor::TTColor(uint32_t plain_color) : TTColor() { SetARGB(plain_color); }
TTColor::TTColor(uint8_t theme_color_id, uint8_t alpha) : TTColor() {
  SetThemeIdAndAlpha(theme_color_id, alpha);
}
uint32_t TTColor::GetPlainColor() const {
  return ((uint32_t)alpha_) << 24u | ((uint32_t)r_) << 16u |
         ((uint32_t)g_) << 8u | ((uint32_t)b_or_id_);
}
void TTColor::SetColor(uint32_t color) {
  type_ = ColorType::kARGB;
  alpha_ = color >> 24u;
  r_ = (color >> 16u) & 0xFF;
  g_ = (color >> 8u) & 0xFF;
  b_or_id_ = color & 0xFF;
}
#endif
std::string TTColor::ToHexString() const {
  if (IsThemeColor()) return "";
  auto color = GetPlainColor();
  char str_color[9];
  snprintf(str_color, sizeof(str_color), "%8x", color);
  str_color[8] = '\0';
  return str_color;
}
TTColor TTColor::ReadHexString(const char* argb_str) {
  uint32_t argb_color = 0;
  if (sscanf(argb_str, "%8x", &argb_color) == 0) {
    return {};
  }
  return TTColor(argb_color);
}
}  // namespace tttext
}  // namespace ttoffice
