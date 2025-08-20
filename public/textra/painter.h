// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PAINTER_H_
#define PUBLIC_TEXTRA_PAINTER_H_

#include <textra/tt_color.h>

#include <string>
#include <vector>

namespace ttoffice {
namespace tttext {
class Style;
enum class FillStyle : uint8_t { kFill, kStroke, kStrokeAndFill };
enum class Cap : uint8_t {
  kButt_Cap,                                   //!< no stroke extension
  kRound_Cap,                                  //!< adds circle
  kSquare_Cap,                                 //!< adds square
  kLast_Cap = static_cast<int>(kSquare_Cap),   //!< largest Cap value
  kDefault_Cap = static_cast<int>(kButt_Cap),  //!< equivalent to kButt_Cap
};
enum class Join : uint8_t {
  kMiter_Join,  //!< extends to miter limit
  kRound_Join,  //!< adds circle
  kBevel_Join,  //!< connects outside edges
  kLast_Join = static_cast<int>(
      kBevel_Join),  //!< equivalent to the largest value for Join
  kDefault_Join = static_cast<int>(kMiter_Join),  //!< equivalent to kMiter_Join
};
struct TextShadow {
  TTColor color_{0};
  float offset_[2]{0, 0};
  double blur_radius_{0};
};
/**
 * @brief Encapsulates painting attributes for text and graphics rendering.
 *
 * The Painter class manages visual styling properties used during rendering,
 * such as fill, stroke cap/join, color, font styling (size, bold, italic),
 * text shadow effects, etc.
 *
 * The Painter class defines an interface with a default implementation. Painter
 * objects are typically created by ICanvasHelper implementations, allowing them
 * to provide backend-specific painters. For example, SkiaCanvasHelper can
 * create a Painter backed by SkPaint.
 */
class Painter {
 public:
  Painter()
      : fill_style_(FillStyle::kFill),
        stroke_width_(0),
        stroke_miter_(0),
        alpha_(0xFF),
        red_(0),
        green_(0),
        blue_(0),
        font_family_("pingfang"),
        text_size_(14),
        bold_(false),
        italic_(false),
        under_line_(false),
        cap_(Cap::kDefault_Cap),
        join_(Join::kDefault_Join),
        shadow_list_{} {}
  virtual ~Painter() = default;

 public:
  FillStyle GetFillStyle() const { return fill_style_; }
  void SetFillStyle(FillStyle style) { fill_style_ = style; }
  float GetStrokeWidth() const { return stroke_width_; }
  void SetStrokeWidth(float stroke_width) { stroke_width_ = stroke_width; }
  float GetStrokeMiter() const { return stroke_miter_; }
  void SetStrokeMiter(float stroke_miter) { stroke_miter_ = stroke_miter; }
  uint8_t GetAlpha() const { return alpha_; }
  void SetAlpha(uint8_t alpha) { alpha_ = alpha; }
  uint8_t GetRed() const { return red_; }
  void SetRed(uint8_t red) { red_ = red; }
  uint8_t GetGreen() const { return green_; }
  void SetGreen(uint8_t green) { green_ = green; }
  uint8_t GetBlue() const { return blue_; }
  void SetBlue(uint8_t blue) { blue_ = blue; }
  const std::string& GetFontFamily() const { return font_family_; }
  void SetFontFamily(const std::string& font_family) {
    font_family_ = font_family;
  }
  float GetTextSize() const { return text_size_; }
  void SetTextSize(float text_size) { text_size_ = text_size; }
  bool IsBold() const { return bold_; }
  void SetBold(bool bold) { bold_ = bold; }
  bool IsItalic() const { return italic_; }
  void SetItalic(bool italic) { italic_ = italic; }
  bool IsUnderLine() const { return under_line_; }
  void SetUnderLine(bool under_line) { under_line_ = under_line; }
  Cap GetCap() const { return cap_; }
  void SetCap(Cap cap) { cap_ = cap; }
  Join GetJoin() const { return join_; }
  void SetJoin(Join join) { join_ = join; }
  void SetColor(uint32_t color) {
    alpha_ = color >> 24u;
    red_ = (color >> 16u) & 0xFFu;
    green_ = (color >> 8u) & 0xFFu;
    blue_ = color & 0xFFu;
  }
  uint32_t GetColor() const {
    uint32_t a = alpha_ << 24u;
    uint32_t r = red_ << 16u;
    uint32_t g = green_ << 8u;
    uint32_t b = blue_;
    return a | r | g | b;
  }
  void SetShadowList(const std::vector<TextShadow>& list) {
    shadow_list_ = list;
  }
  const std::vector<TextShadow>& GetShadowList() const { return shadow_list_; }

 private:
  //  uint32_t font_id_;
  FillStyle fill_style_;
  float stroke_width_;
  float stroke_miter_;
  uint8_t alpha_;
  uint8_t red_;
  uint8_t green_;
  uint8_t blue_;
  std::string font_family_;
  float text_size_;
  bool bold_;
  bool italic_;
  bool under_line_;
  Cap cap_;
  Join join_;
  std::vector<TextShadow> shadow_list_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PAINTER_H_
