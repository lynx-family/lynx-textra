// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_definition.h>
#include <textra/tt_color.h>

#include <string>

namespace ttoffice {
namespace tttext {
std::string TTColor::ToHexString() const {
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
