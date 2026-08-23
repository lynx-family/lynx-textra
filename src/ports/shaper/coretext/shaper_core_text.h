// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_
#define SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/text_layout.h>

#include <cstddef>
#include <vector>

#include "src/ports/shaper/coretext/safe_font_cache.h"
#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class RunStyle;
constexpr uint32_t SHAPER_BUFF_SIZE = 1024;
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
  static void SetSafeFontCacheMaxEntries(size_t max_entries);
  static size_t GetSafeFontCacheMaxEntries();
  static void ClearSafeFontCache();

 private:
  struct CachedSafeFontKey {
    FontDescriptor font_descriptor_;
    float text_size_ = 0.0f;
    bool fake_bold_ = false;
    bool fake_italic_ = false;

    struct Hasher {
      size_t operator()(const CachedSafeFontKey& key) const {
        size_t seed = FontDescriptor::Hasher()(key.font_descriptor_);
        seed ^= std::hash<float>()(key.text_size_) + 0x9e3779b9 + (seed << 6) +
                (seed >> 2);
        seed ^= std::hash<bool>()(key.fake_bold_) + 0x9e3779b9 + (seed << 6) +
                (seed >> 2);
        seed ^= std::hash<bool>()(key.fake_italic_) + 0x9e3779b9 + (seed << 6) +
                (seed >> 2);
        return seed;
      }
    };

    bool operator==(const CachedSafeFontKey& rhs) const {
      return font_descriptor_ == rhs.font_descriptor_ &&
             text_size_ == rhs.text_size_ && fake_bold_ == rhs.fake_bold_ &&
             fake_italic_ == rhs.fake_italic_;
    }
  };

  using SafeFontCacheType =
      SafeFontCache<CachedSafeFontKey, CachedSafeFontKey::Hasher>;

  CFAttributedStringRef GenerateAttributeString(const char16_t* content,
                                                uint32_t length,
                                                uint32_t* u16char_map,
                                                CFDictionaryRef key) const;
  CFDictionaryRef GenerateAttributes(const ShapeKey& key) const;
  CTFontRef CreateSafeFontUnified(CTFontRef existing_font) const;
  CTFontRef GetOrCreateSafeFont(const FontDescriptor& fd, float text_size,
                                bool fake_bold, bool fake_italic) const;
  static SafeFontCacheType& GetGlobalSafeFontCache();

 private:
  mutable std::vector<TypefaceRef> ct_font_lst_;
  mutable uint32_t tmp_buf_[SHAPER_BUFF_SIZE];
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_H_
