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
using Weight = uint16_t;
using Width = uint8_t;
using Slant = uint8_t;
class L_EXPORT FontStyle {
 public:
  static constexpr Weight kInvisible_Weight = 0;
  static constexpr Weight kThin_Weight = 100;
  static constexpr Weight kExtraLight_Weight = 200;
  static constexpr Weight kLight_Weight = 300;
  static constexpr Weight kNormal_Weight = 400;
  static constexpr Weight kMedium_Weight = 500;
  static constexpr Weight kSemiBold_Weight = 600;
  static constexpr Weight kBold_Weight = 700;
  static constexpr Weight kExtraBold_Weight = 800;
  static constexpr Weight kBlack_Weight = 900;
  static constexpr Weight kExtraBlack_Weight = 1000;

  static constexpr Width kUltraCondensed_Width = 1;
  static constexpr Width kExtraCondensed_Width = 2;
  static constexpr Width kCondensed_Width = 3;
  static constexpr Width kSemiCondensed_Width = 4;
  static constexpr Width kNormal_Width = 5;
  static constexpr Width kSemiExpanded_Width = 6;
  static constexpr Width kExpanded_Width = 7;
  static constexpr Width kExtraExpanded_Width = 8;
  static constexpr Width kUltraExpanded_Width = 9;

  static constexpr Slant kUpright_Slant = 0;
  static constexpr Slant kItalic_Slant = 1;
  static constexpr Slant kOblique_Slant = 2;

  constexpr FontStyle(Weight weight, Width width, uint8_t slant)
      : pack_value_(PackValue{
            .part = {.weight_ = weight, .width_ = width, .slant_ = slant}}) {}

  constexpr FontStyle()
      : FontStyle{kNormal_Weight, kNormal_Width, kUpright_Slant} {}

  bool operator==(const FontStyle& rhs) const {
    return pack_value_.value_ == rhs.pack_value_.value_;
  }
  bool operator!=(const FontStyle& rhs) const {
    return pack_value_.value_ != rhs.pack_value_.value_;
  }

  Weight GetWeight() const { return pack_value_.part.weight_; }
  Width GetWidth() const { return pack_value_.part.width_; }
  Slant GetSlant() const { return pack_value_.part.slant_; }
  void SetWeight(Weight weight) { pack_value_.part.weight_ = weight; }
  void SetWidth(Width width) { pack_value_.part.width_ = width; }
  void SetSlant(Slant slant) { pack_value_.part.slant_ = slant; }

  static constexpr FontStyle Normal() {
    return {kNormal_Weight, kNormal_Width, kUpright_Slant};
  }
  static constexpr FontStyle Bold() {
    return {kBold_Weight, kNormal_Width, kUpright_Slant};
  }
  static constexpr FontStyle Italic() {
    return {kNormal_Weight, kNormal_Width, kItalic_Slant};
  }
  static constexpr FontStyle BoldItalic() {
    return {kBold_Weight, kNormal_Width, kItalic_Slant};
  }

  int32_t Value() const { return pack_value_.value_; }

 private:
  union PackValue {
    int32_t value_;
    struct {
      uint16_t weight_;
      uint8_t width_;
      uint8_t slant_;
    } part;
  } pack_value_;
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

/**
 * @brief Holds em-height ascent/descent values for a given font size.
 */
class L_EXPORT FontEmHeight {
 public:
  FontEmHeight() : FontEmHeight(0, 0) {}
  FontEmHeight(float ascent, float descent)
      : ascent_(ascent), descent_(descent) {}
  ~FontEmHeight() = default;

  float GetAscent() const { return ascent_; }
  void SetAscent(float ascent) { ascent_ = ascent; }
  float GetDescent() const { return descent_; }
  void SetDescent(float descent) { descent_ = descent; }

 private:
  float ascent_ = 0;
  float descent_ = 0;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_FONT_INFO_H_
