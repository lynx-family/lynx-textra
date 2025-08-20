// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_TT_SHAPER_H_
#define SRC_TEXTLAYOUT_TT_SHAPER_H_

#include <textra/font_info.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_typeface_helper.h>
#include <textra/layout_definition.h>
#include <textra/macro.h>
#include <textra/text_layout.h>

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
class ShapeResult;
class ShapeKey;
class ShapeCache;
class ShapeStyle;
class TTShaper {
 public:
  TTShaper() noexcept = delete;
  explicit TTShaper(FontmgrCollection font_collection) noexcept;
  virtual ~TTShaper();
  TTShaper(const TTShaper& other) = delete;
  void operator=(const TTShaper& other) = delete;

  static std::unique_ptr<TTShaper> CreateShaper(
      FontmgrCollection* font_collection, ShaperType type = kSystem);

  const FontmgrCollection& GetFontCollection() const noexcept {
    return font_collection_;
  };
  virtual void ProcessBidirection(const char32_t* text, uint32_t length,
                                  WriteDirection write_direction,
                                  uint32_t* visual_map, uint32_t* logical_map,
                                  uint8_t* dir_vec);
  virtual void OnShapeText(const ShapeKey& key, ShapeResult* result) const = 0;
  std::shared_ptr<ShapeResult> ShapeText(const char32_t* text, uint32_t length,
                                         const ShapeStyle* shape_style,
                                         bool rtl) const;

 protected:
  FontmgrCollection font_collection_;
};

class ShapeStyle {
  friend std::hash<ShapeStyle>;

 public:
  ShapeStyle() = default;

  ShapeStyle(const FontDescriptor& font_descriptor, float font_size,
             bool fake_bold, bool fake_italic)
      : font_descriptor_(font_descriptor),
        font_size_(font_size),
        fake_bold_(fake_bold),
        fake_italic_(fake_italic),
        hash_(UpdateHash()) {}

 public:
  const FontDescriptor& GetFontDescriptor() const { return font_descriptor_; }
  float GetFontSize() const { return font_size_; }
  bool FakeBold() const { return fake_bold_; }
  bool FakeItalic() const { return fake_italic_; }
  void SetFontDescriptor(const FontDescriptor& font_descriptor) {
    font_descriptor_ = font_descriptor;
    hash_ = UpdateHash();
  }

 private:
  std::size_t UpdateHash() const {
    std::size_t hash = 0;
    for (const auto& font_family : font_descriptor_.font_family_list_) {
      hash ^= std::hash<std::string>()(font_family);
    }
    hash ^= std::hash<uint64_t>()(font_descriptor_.platform_font_);
    hash ^= std::hash<int32_t>()(font_descriptor_.font_style_.Value());
    hash ^= std::hash<float>()(font_size_);
    hash ^= std::hash<bool>()(fake_bold_) << 0u;
    hash ^= std::hash<bool>()(fake_italic_) << 1u;
    return hash;
  }

 public:
  bool operator==(const ShapeStyle& o) const {
    TTASSERT(hash_ == UpdateHash());
    TTASSERT(o.hash_ == o.UpdateHash());
    return hash_ == o.hash_;
  }
  std::size_t operator()(const ShapeStyle& k) const {
    TTASSERT(hash_ == UpdateHash());
    return hash_;
  }

 private:
  FontDescriptor font_descriptor_{};
  float font_size_ = 0;
  bool fake_bold_ = false;
  bool fake_italic_ = false;
  std::size_t hash_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice

namespace std {
template <>
struct hash<tttext::ShapeStyle> {
  size_t operator()(const tttext::ShapeStyle& record) const noexcept {
    return record.hash_;
  }
};
}  // namespace std

namespace ttoffice {
namespace tttext {
class ShapeKey {
  friend std::hash<const ShapeKey>;
  friend std::hash<ShapeKey>;

 public:
  ShapeKey() : text_(), style_(), rtl_(false), hash_(0) {}
  ShapeKey(const char32_t* text, uint32_t length, FontDescriptor&& p_font_list,
           float font_size, bool fake_bold, bool fake_italic, bool rtl)
      : text_(text, length),
        style_(std::move(p_font_list), font_size, fake_bold, fake_italic),
        rtl_(rtl),
        hash_(UpdateHash()) {}
  ShapeKey(const char32_t* text, uint32_t length, const ShapeStyle* shape_style,
           bool rtl)
      : text_(text, length),
        style_(*shape_style),
        rtl_(rtl),
        hash_(UpdateHash()) {}

 private:
  std::size_t UpdateHash() const {
    return std::hash<std::u32string>()(text_) ^
           std::hash<ShapeStyle>()(style_) ^ std::hash<bool>()(rtl_);
  }

 public:
  bool operator==(const ShapeKey& o) const {
    return hash_ == o.hash_ && style_ == o.style_ && rtl_ == o.rtl_ &&
           text_ == o.text_;
  }
  std::size_t operator()(const ShapeKey& k) const {
    TTASSERT(k.hash_ == k.UpdateHash());
    return k.hash_;
  }

 public:
  std::u32string text_;
  ShapeStyle style_;
  bool rtl_;
  std::size_t hash_;
};

class PlatformShapingResultReader {
 public:
  virtual ~PlatformShapingResultReader() = default;
  virtual uint32_t GlyphCount() const = 0;
  virtual uint32_t TextCount() const = 0;
  virtual GlyphID ReadGlyphID(uint32_t idx) const = 0;
  virtual float ReadAdvanceX(uint32_t idx) const = 0;
  virtual float ReadAdvanceY(uint32_t idx) const { return 0; }
  virtual float ReadPositionX(uint32_t idx) const { return 0; }
  virtual float ReadPositionY(uint32_t idx) const { return 0; }
  virtual uint32_t ReadIndices(uint32_t idx) const = 0;
  virtual TypefaceRef ReadFontId(uint32_t idx) const = 0;
};

class ShapeResult {
  friend TTShaper;

 public:
  explicit ShapeResult(const uint32_t& char_count, bool is_rtl)
      : is_rtl_(is_rtl) {
    advances_.reserve(char_count);
    c2glyph_indices_.reserve(char_count);
  }
  ShapeResult(const ShapeResult& result) = default;

 public:
  uint32_t CharCount() const {
    return static_cast<uint32_t>(c2glyph_indices_.size());
  }
  uint32_t GlyphCount() const { return static_cast<uint32_t>(glyphs_.size()); }
  const GlyphID& Glyphs(const uint32_t& idx) const { return glyphs_[idx]; }
  const float* Advances(const uint32_t& idx) const {
    return advances_[idx].data();
  }
  const float* Positions(const uint32_t& idx) const {
    return position_[idx].data();
  }
  const TypefaceRef& Font(const uint32_t& glyph_idx) const {
    return font_[glyph_idx];
  }
  const TypefaceRef& FontByCharId(const uint32_t& char_idx) const {
    auto glyph_pos = CharToGlyph(char_idx);
    if (glyph_pos == GlyphCount() && glyph_pos != 0) glyph_pos--;
    return Font(glyph_pos);
  }
  uint32_t CharToGlyph(const uint32_t& char_idx) const {
    return char_idx < c2glyph_indices_.size()
               ? c2glyph_indices_[char_idx]
               : static_cast<uint32_t>(glyphs_.size());
  }
  uint32_t GlyphToChar(const uint32_t& glyph_idx) const {
    return glyph_idx < indices_.size()
               ? indices_[glyph_idx]
               : static_cast<uint32_t>(c2glyph_indices_.size());
  }
  bool IsRTL() const { return is_rtl_; }
  void AppendPlatformShapingResult(const PlatformShapingResultReader& reader);
  void ReserveGlyphCount(const uint32_t& glyph_count) {
    glyphs_.resize(glyph_count);
    font_.resize(glyph_count);
    indices_.resize(glyph_count);
    advances_.resize(glyph_count);
    position_.resize(glyph_count);
  }
  float MeasureWidth(uint32_t start_char, uint32_t char_count,
                     float letter_spacing) const;

 private:
  bool is_rtl_ = false;
  std::vector<GlyphID> glyphs_;
  std::vector<TypefaceRef> font_;               // size equal to glyph count
  std::vector<std::array<float, 2>> advances_;  // size equal to char count
  std::vector<std::array<float, 2>> position_;  // size equal to char count
  std::vector<uint32_t> c2glyph_indices_;       // size equal to char count
  std::vector<uint32_t> indices_;               // glyph id to char id map
};

}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_TT_SHAPER_H_
