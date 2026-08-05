// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/icu_substitute/bidi/bidi_wrapper.h"

#include "src/textlayout/utils/u_8_string.h"

namespace ttoffice {
namespace tttext {
BidiWrapper::BidiWrapper() : bidi_(std::make_unique<Bidi>()) {}

BidiWrapper::~BidiWrapper() {}
BidiWrapper& BidiWrapper::GetInstance() {
  static BidiWrapper bidi_utils;
  return bidi_utils;
}
void BidiWrapper::SetPara(const char32_t* u32_content, const uint32_t& length,
                          WriteDirection direction, uint8_t* bidi_levels,
                          uint32_t* visual_map, uint32_t* logical_map) {
  int16_t para_level = 0x7E;
  if (direction == WriteDirection::kLTR ||
      direction == tttext::WriteDirection::kTTB)
    para_level = 0;
  if (direction == tttext::WriteDirection::kRTL ||
      direction == tttext::WriteDirection::kBTT)
    para_level = 1;
  bidi_->setPara(u32_content, para_level, std::vector<int16_t>());
  const auto visual_indices = bidi_->getVisualMap();
  const auto logical_indices = bidi_->getLogicalMap();
  for (int k = 0; k < static_cast<int>(length); k++) {
    bidi_levels[k] = bidi_->getLevelAt(k);
    visual_map[k] = visual_indices[k];
    logical_map[k] = logical_indices[k];
  }
}
}  // namespace tttext
}  // namespace ttoffice
