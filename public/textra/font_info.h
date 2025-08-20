// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_FONT_INFO_H_
#define PUBLIC_TEXTRA_FONT_INFO_H_

#include <textra/macro.h>

#include <string>
#include <vector>

namespace ttoffice {
namespace tttext {
/**
 * @brief Stores font variation properties (weight, width, slant).
 */
class L_EXPORT FontStyle {
 public:
  enum Weight : uint16_t {
    kInvisible_Weight = 0,
    kThin_Weight = 100,
    kExtraLight_Weight = 200,
    kLight_Weight = 300,
    kNormal_Weight = 400,
    kMedium_Weight = 500,
    kSemiBold_Weight = 600,
    kBold_Weight = 700,
    kExtraBold_Weight = 800,
    kBlack_Weight = 900,
    kExtraBlack_Weight = 1000,
  };

  enum Width : uint8_t {
    kUltraCondensed_Width = 1,
    kExtraCondensed_Width = 2,
    kCondensed_Width = 3,
    kSemiCondensed_Width = 4,
    kNormal_Width = 5,
    kSemiExpanded_Width = 6,
    kExpanded_Width = 7,
    kExtraExpanded_Width = 8,
    kUltraExpanded_Width = 9,
  };

  enum Slant : uint8_t {
    kUpright_Slant,
    kItalic_Slant,
    kOblique_Slant,
  };

  constexpr FontStyle(Weight weight, Width width, Slant slant)
      : value_(static_cast<uint16_t>(weight) +
               (static_cast<uint8_t>(width) << 16) +
               (static_cast<uint8_t>(slant) << 24)) {}

  constexpr FontStyle()
      : FontStyle{kNormal_Weight, kNormal_Width, kUpright_Slant} {}

  bool operator==(const FontStyle& rhs) const { return value_ == rhs.value_; }
  bool operator!=(const FontStyle& rhs) const { return value_ != rhs.value_; }

  Weight GetWeight() const { return static_cast<Weight>(value_ & 0xFFFF); }
  Width GetWidth() const { return static_cast<Width>((value_ >> 16) & 0xFF); }
  Slant GetSlant() const { return static_cast<Slant>((value_ >> 24) & 0xFF); }

  static constexpr FontStyle Normal() {
    return {Weight::kNormal_Weight, Width::kNormal_Width,
            Slant::kUpright_Slant};
  }
  static constexpr FontStyle Bold() {
    return {Weight::kBold_Weight, Width::kNormal_Width, Slant::kUpright_Slant};
  }
  static constexpr FontStyle Italic() {
    return {Weight::kNormal_Weight, Width::kNormal_Width, Slant::kItalic_Slant};
  }
  static constexpr FontStyle BoldItalic() {
    return {Weight::kBold_Weight, Width::kNormal_Width, Slant::kItalic_Slant};
  }

  int32_t Value() const { return value_; }

 private:
  int32_t value_;
};

/**
 * @brief Encapsulates the information needed to select and match fonts.
 *
 * Used throughout the text layout system for:
 * - Font selection and fallback in FontmgrCollection
 * - Text styling via the Style class
 * - Shaping operations in the text layout pipeline
 * - Caching font resolution results for performance
 *
 * The struct includes a custom hash function and equality operator to support
 * efficient lookup in font caches and collections.
 */
struct L_EXPORT FontDescriptor {
  std::vector<std::string> font_family_list_;
  FontStyle font_style_ = FontStyle::Normal();
  uint64_t platform_font_ = 0;
  struct Hasher {
    size_t operator()(const FontDescriptor& key) const {
      size_t seed = 0;
      for (auto& font : key.font_family_list_) {
        seed ^= std::hash<std::string>()(font) + 0x9e3779b9;
      }
      return seed ^ std::hash<int32_t>()(key.font_style_.Value()) ^
             std::hash<uint64_t>()(key.platform_font_);
    }
  };
  bool operator==(const FontDescriptor& rhs) const {
    return platform_font_ == rhs.platform_font_ &&
           font_style_ == rhs.font_style_ &&
           font_family_list_ == rhs.font_family_list_;
  }
};
/**
 * @brief Holds font metrics for a given font size.
 *
 * Font metrics define vertical spacing relative to the baseline.
 * Used in text layout to compute line height and align text across sizes.
 */
class L_EXPORT FontInfo {
 public:
  FontInfo() : FontInfo(0, 0, 0) {}
  FontInfo(float ascent, float descent, float font_size)
      : ascent_(ascent), descent_(descent), font_size_(font_size) {}
  ~FontInfo() = default;
  float GetTop() const { return top_; }
  void SetTop(float top) { top_ = top; }
  float GetAscent() const { return ascent_; }
  void SetAscent(float ascent) { ascent_ = ascent; }
  float GetDescent() const { return descent_; }
  void SetDescent(float descent) { descent_ = descent; }
  float GetBottom() const { return bottom_; }
  void SetBottom(float bottom) { bottom_ = bottom; }
  float GetLeading() const { return leading_; }
  void SetLeading(float leading) { leading_ = leading; }
  float GetFontSize() const { return font_size_; }
  void SetFontSize(float font_size) { font_size_ = font_size; }
  float GetHeight() const { return GetDescent() - GetAscent(); }

 private:
  /** Greatest extent above the baseline for any glyph.
    Typically less than zero.
  */
  float top_ = 0;

  /** Recommended distance above the baseline to reserve for a line of text.
      Typically less than zero.
  */
  float ascent_ = 0;

  /** Recommended distance below the baseline to reserve for a line of text.
      Typically greater than zero.
  */
  float descent_ = 0;

  /** Greatest extent below the baseline for any glyph.
      Typically greater than zero.
  */
  float bottom_ = 0;

  /** Recommended distance to add between lines of text.
      Typically greater than or equal to zero.
  */
  float leading_ = 0;

  float font_size_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_FONT_INFO_H_
