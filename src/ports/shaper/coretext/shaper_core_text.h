// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_
#define SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/text_layout.h>

#include <vector>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class RunStyle;
class ShaperCoreText : public TTShaper {
 public:
  ShaperCoreText() = delete;
  explicit ShaperCoreText(FontmgrCollection& font_collections) noexcept;
  ~ShaperCoreText() override;

 public:
  void ProcessBidirection(const char32_t* text, uint32_t length,
                          WriteDirection write_direction, uint32_t* visual_map,
                          uint32_t* logical_map, uint8_t* dir_vec) override;
  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;

 private:
  CFAttributedStringRef GenerateAttributeString(const char16_t* content,
                                                uint32_t length,
                                                uint32_t* u16char_map,
                                                CFDictionaryRef key) const;
  CFDictionaryRef GenerateAttributes(const ShapeKey& key) const;

 private:
  mutable std::vector<TypefaceRef> ct_font_lst_;
  mutable CTFontDescriptorRef default_font_desc_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_
