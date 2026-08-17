// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_
#define SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_

#include <textra/layout_definition.h>

#include <cstdint>

namespace ttoffice {
namespace tttext {
class BidiWrapper {
 private:
  BidiWrapper();
  ~BidiWrapper();

 public:
  BidiWrapper(BidiWrapper&) = delete;
  void operator==(BidiWrapper&) = delete;

 public:
  static BidiWrapper& GetInstance();
  void SetPara(const char32_t* u32_content, const uint32_t& length,
               WriteDirection direction, uint8_t* bidi_levels,
               uint32_t* visual_map, uint32_t* logical_map);
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_BIDI_WRAPPER_H_
