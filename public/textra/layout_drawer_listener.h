// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_LAYOUT_DRAWER_LISTENER_H_
#define PUBLIC_TEXTRA_LAYOUT_DRAWER_LISTENER_H_

#include <textra/layout_definition.h>
#include <textra/tt_color.h>
namespace ttoffice {
namespace tttext {
class LayoutDrawerListener {
 public:
  LayoutDrawerListener() = default;
  virtual ~LayoutDrawerListener() = default;

 public:
  virtual uint32_t FetchThemeColor(ThemeCategory style_type,
                                   const TTColor& color) {
    return color.IsThemeColor() ? 0xFF000000 : color.GetPlainColor();
  }

 public:
  static LayoutDrawerListener* GetInstance() {
    static LayoutDrawerListener instance;
    return &instance;
  }
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_LAYOUT_DRAWER_LISTENER_H_
