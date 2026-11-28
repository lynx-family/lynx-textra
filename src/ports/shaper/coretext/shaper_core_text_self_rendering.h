// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_
#define SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/text_layout.h>

#include <string>
#include <vector>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class RunStyle;
constexpr uint32_t SELF_RENDERING_SHAPER_BUFF_SIZE = 1024;

// CoreText shaper for callers that render the returned glyphs themselves.
// This class deliberately forks ShaperCoreText so ordered web-font matching
// cannot change the native CoreText path. Keep its cache state independent as
// well: custom typefaces can be app-owned even though CoreText registration is
// process-wide.
class ShaperCoreTextSelfRendering : public TTShaper {
  friend class ShaperCoreTextSelfRenderingTestAccess;

 public:
  ShaperCoreTextSelfRendering() = delete;
  explicit ShaperCoreTextSelfRendering(
      FontmgrCollection& font_collections) noexcept;
  ~ShaperCoreTextSelfRendering() override;

 public:
  void ProcessBidirection(const char32_t* text, uint32_t length,
                          WriteDirection write_direction, uint32_t* visual_map,
                          uint32_t* logical_map, uint8_t* dir_vec) override;
  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;

 private:
  static CTFontDescriptorRef CopyFontDescriptorWithoutCascade(
      CTFontDescriptorRef font_descriptor);
  static CFArrayRef CreateOrderedCascadeDescriptors(
      const std::vector<CTFontDescriptorRef>& font_descriptors);
  static bool FontCoversStringRangeExactly(CTFontRef font, CFStringRef string,
                                           CFRange range);

  CFAttributedStringRef GenerateAttributeString(const char16_t* content,
                                                uint32_t length,
                                                uint32_t* u16char_map,
                                                CFDictionaryRef key) const;
  CFDictionaryRef GenerateAttributes(const ShapeKey& key) const;
  void ApplyOrderedFamilyFonts(
      CFMutableAttributedStringRef attributed_string, float text_size,
      const std::vector<TypefaceRef>& resolved_typefaces,
      const FontDescriptor& fd) const;
  CTFontRef CreateSafeFontWithOrderedFamilyList(
      CTFontRef existing_font,
      const std::vector<TypefaceRef>& resolved_typefaces,
      const FontDescriptor& fd) const;
  CTFontRef GetOrCreateSafeFont(const FontDescriptor& fd,
                                float text_size) const;

 private:
  mutable uint32_t tmp_buf_[SELF_RENDERING_SHAPER_BUFF_SIZE];
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_
