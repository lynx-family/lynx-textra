// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_
#define SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_

#include <textra/layout_definition.h>

#include <memory>
#include <string>
#include <vector>

#if defined(USE_ICU)
#include <textra/icu_wrapper.h>
#else
#include "src/textlayout/icu_substitute/bidi/algorithm/bidi.h"
#endif

class BidiWrapper {
 private:
  BidiWrapper();
  ~BidiWrapper();

 public:
  BidiWrapper(BidiWrapper&) = delete;
  void operator==(BidiWrapper&) = delete;

 public:
  static BidiWrapper& GetInstance();
  void SetPara(const std::string& str,
               ttoffice::textlayout::WriteDirection write_direction);
  const std::vector<int16_t>& GetLevels();
  const std::vector<uint32_t>& GetVisualMap();
  const std::vector<uint32_t>& GetLogicalMap();

 private:
  std::vector<int16_t> bidi_levels_;
  std::vector<uint32_t> visual_map_;
  std::vector<uint32_t> logical_map_;
#if !defined(USE_ICU)
  std::unique_ptr<Bidi> bidi_;
#endif
};
#endif  // SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_
