// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_I_TYPEFACE_HELPER_H_
#define PUBLIC_TEXTRA_I_TYPEFACE_HELPER_H_

#include <textra/font_info.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ttoffice {
namespace tttext {
using GlyphID = uint16_t;
using Unichar = uint32_t;

// An abstraction for platform fonts, allowing Minikin to be used with
// multiple actual implementations of fonts.
class ITypefaceHelper : public std::enable_shared_from_this<ITypefaceHelper> {
 public:
  explicit ITypefaceHelper(uint32_t unique_id) : unique_id_(unique_id) {}

  virtual ~ITypefaceHelper() = default;

  virtual float GetHorizontalAdvance(GlyphID glyph_id,
                                     float font_size) const = 0;
  virtual void GetHorizontalAdvances(GlyphID glyph_ids[], uint32_t count,
                                     float widths[], float font_size) const = 0;
  virtual void GetWidthBound(float* rect_ltwh, GlyphID glyph_id,
                             float font_size) const = 0;
  virtual void GetWidthBounds(float* rect_ltrb, GlyphID glyphs[],
                              uint32_t glyph_count, float font_size) = 0;
  // Override if font can provide access to raw data
  virtual const void* GetFontData() const = 0;

  // Override if font can provide access to raw data
  virtual size_t GetFontDataSize() const = 0;

  // Override if font can provide access to raw data.
  // Returns index within OpenType collection
  virtual int GetFontIndex() const = 0;

  virtual uint16_t UnicharToGlyph(Unichar codepoint,
                                  uint32_t variationSelector = 0) const = 0;

  virtual void UnicharsToGlyphs(const Unichar* unichars, uint32_t count,
                                GlyphID* glyphs) const = 0;

  const FontInfo& GetFontInfo(float font_size) {
    std::lock_guard<std::mutex> guard(font_info_cache_lock_);
    auto iter = font_info_cache_.find(font_size);
    if (iter != font_info_cache_.end()) {
      return iter->second;
    }
    auto& info = font_info_cache_[font_size];
    OnCreateFontInfo(&info, font_size);
    return info;
  }

  virtual uint32_t GetUnitsPerEm() const = 0;

  const FontStyle& FontStyle() const { return font_style_; }

  uint32_t GetUniqueId() const { return unique_id_; }

 protected:
  virtual void OnCreateFontInfo(FontInfo* info, float font_size) const = 0;

  uint32_t unique_id_;
  class FontStyle font_style_;
  std::unordered_map<float, FontInfo> font_info_cache_;
  std::string font_name_;
  std::mutex font_info_cache_lock_;
};
}  // namespace tttext
}  // namespace ttoffice
using TypefaceRef = std::shared_ptr<tttext::ITypefaceHelper>;
#endif  // PUBLIC_TEXTRA_I_TYPEFACE_HELPER_H_
