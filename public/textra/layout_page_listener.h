// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_LAYOUT_PAGE_LISTENER_H_
#define PUBLIC_TEXTRA_LAYOUT_PAGE_LISTENER_H_

#include <textra/layout_definition.h>
#include <textra/tt_color.h>

#include <cstdint>
#include <utility>
#include <vector>
namespace ttoffice {
namespace tttext {
class RunDelegate;
enum class ThemeCategory : uint8_t;
class LayoutRegion;
class LayoutPageListener {
 public:
  LayoutPageListener() = default;
  virtual ~LayoutPageListener() = default;

 public:
  virtual void OnLineLayouted(LayoutRegion* page, uint32_t line_idx,
                              bool last_line_in_paragraph,
                              float available_height, LayoutResult* result) {}
  virtual std::vector<std::pair<float, float>> GetRangeList(float* top,
                                                            float range_height,
                                                            float start,
                                                            float end) {
    std::vector<std::pair<float, float>> lst;
    lst.emplace_back(std::make_pair(start, end));
    return lst;
  }

 public:
  static LayoutPageListener* GetInstance() {
    static LayoutPageListener instance;
    return &instance;
  }
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_LAYOUT_PAGE_LISTENER_H_
