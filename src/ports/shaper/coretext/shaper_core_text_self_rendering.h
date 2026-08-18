// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_
#define SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#include <textra/text_layout.h>

#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class RunStyle;
constexpr uint32_t SELF_RENDERING_SHAPER_BUFF_SIZE = 1024;

// CoreText shaper for callers that render the returned glyphs themselves.
// This class deliberately forks ShaperCoreText so ordered web-font matching
// cannot change the legacy kSystem path. Keep its cache state independent as
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
  static void SetSafeFontCacheMaxEntries(size_t max_entries);
  static size_t GetSafeFontCacheMaxEntries();
  // Releases every cached safe font. Callers of GetOrCreateSafeFont own the
  // reference they received, so dropping the cache's own reference here cannot
  // invalidate a shaping run that is already in flight.
  static void ClearSafeFontCache();

 private:
  static CTFontDescriptorRef CopyFontDescriptorWithoutCascade(
      CTFontDescriptorRef font_descriptor);
  static CFArrayRef CreateOrderedCascadeDescriptors(
      const std::vector<CTFontDescriptorRef>& font_descriptors);
  static bool FontCoversStringRangeExactly(CTFontRef font, CFStringRef string,
                                           CFRange range);

  struct CachedSafeFontKey {
    FontDescriptor font_descriptor_;
    float text_size_ = 0.0f;
    bool fake_bold_ = false;
    bool fake_italic_ = false;

    struct Hasher {
      size_t operator()(const CachedSafeFontKey& key) const {
        size_t seed = 0;
        const auto hash_combine = [&seed](size_t value) {
          seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        for (const auto& family : key.font_descriptor_.font_family_list_) {
          hash_combine(std::hash<std::string>()(family));
        }
        hash_combine(
            std::hash<int32_t>()(key.font_descriptor_.font_style_.Value()));
        hash_combine(
            std::hash<uint64_t>()(key.font_descriptor_.platform_font_));
        hash_combine(std::hash<float>()(key.text_size_));
        hash_combine(std::hash<bool>()(key.fake_bold_));
        hash_combine(std::hash<bool>()(key.fake_italic_));
        return seed;
      }
    };

    bool operator==(const CachedSafeFontKey& rhs) const {
      return font_descriptor_ == rhs.font_descriptor_ &&
             text_size_ == rhs.text_size_ && fake_bold_ == rhs.fake_bold_ &&
             fake_italic_ == rhs.fake_italic_;
    }
  };

  struct CachedSafeFontEntry {
    CTFontRef safe_font_ = nullptr;
    std::list<CachedSafeFontKey>::iterator lru_it_;
  };

  struct SafeFontCacheState {
    std::unordered_map<CachedSafeFontKey, CachedSafeFontEntry,
                       CachedSafeFontKey::Hasher>
        cache_;
    std::list<CachedSafeFontKey> lru_;
    std::mutex mutex_;
    size_t max_entries_ = 256;
  };

  CFAttributedStringRef GenerateAttributeString(const char16_t* content,
                                                uint32_t length,
                                                uint32_t* u16char_map,
                                                CFDictionaryRef key) const;
  CFDictionaryRef GenerateAttributes(const ShapeKey& key) const;
  void ApplyOrderedFamilyFonts(
      CFMutableAttributedStringRef attributed_string, float text_size,
      const std::vector<TypefaceRef>& resolved_typefaces) const;
  CTFontRef CreateSafeFontWithOrderedFamilyList(
      CTFontRef existing_font,
      const std::vector<TypefaceRef>& resolved_typefaces) const;
  CTFontRef GetOrCreateSafeFont(const FontDescriptor& fd, float text_size,
                                bool fake_bold, bool fake_italic) const;
  static SafeFontCacheState& GetSafeFontCacheState();
  static void TrimSafeFontCacheLocked(SafeFontCacheState& state);

 private:
  mutable std::vector<TypefaceRef> ct_font_lst_;
  mutable uint32_t tmp_buf_[SELF_RENDERING_SHAPER_BUFF_SIZE];
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_CORETEXT_SHAPER_CORE_TEXT_SELF_RENDERING_H_
