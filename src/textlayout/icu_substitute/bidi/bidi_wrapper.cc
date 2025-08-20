// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/icu_substitute/bidi/bidi_wrapper.h"

#include "src/textlayout/utils/u_8_string.h"

BidiWrapper::BidiWrapper()
#if defined(USE_ICU)
{
#else
    : bidi_(std::make_unique<Bidi>()) {
#endif
}

BidiWrapper::~BidiWrapper() {}
BidiWrapper& BidiWrapper::GetInstance() {
  static BidiWrapper bidi_utils;
  return bidi_utils;
}
void BidiWrapper::SetPara(
    const std::string& str,
    ttoffice::textlayout::WriteDirection write_direction) {
  int16_t para_level = 0x7E;
  if (write_direction == ttoffice::textlayout::WriteDirection::kLTR ||
      write_direction == ttoffice::textlayout::WriteDirection::kTTB)
    para_level = 0;
  if (write_direction == ttoffice::textlayout::WriteDirection::kRTL ||
      write_direction == ttoffice::textlayout::WriteDirection::kBTT)
    para_level = 1;
#if defined(USE_ICU)
  auto& icu_wrapper_ = ttoffice::ICUWrapper::GetInstance();
  icu_wrapper_.BidiInit(str, para_level, bidi_levels_, visual_map_,
                        logical_map_);
#else
  auto u32_content = ttoffice::U8String::ToU32String(str);
  bidi_->setPara(u32_content, para_level, std::vector<int16_t>());
  bidi_levels = bidi_->getLevels();
  visual_map_ = bidi_->getVisualMap();
  logical_map_ = bidi_->getLogicalMap();
#endif
}
const std::vector<int16_t>& BidiWrapper::GetLevels() { return bidi_levels_; }

const std::vector<uint32_t>& BidiWrapper::GetVisualMap() { return visual_map_; }

const std::vector<uint32_t>& BidiWrapper::GetLogicalMap() {
  return logical_map_;
}
