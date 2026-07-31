// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_STYLE_STYLE_IMPL_H_
#define SRC_TEXTLAYOUT_STYLE_STYLE_IMPL_H_

#include <textra/font_info.h>
#include <textra/layout_definition.h>
#include <textra/painter.h>
#include <textra/style.h>
#include <textra/tt_color.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class ShapeStyle;

union DecorationStyle {
  uint64_t value;
  struct {
    uint32_t color_ : 32;
    uint16_t fixed_10x_thickness_ : 16;
    LineType line_type_ : 8;
    uint8_t padding_ : 8;
  } style_ = {TTColor::UNDEFINED, 10, LineType::kSolid, 0};
};

class StyleImpl {
 public:
  using TextShadowList = std::vector<TextShadow>;

  StyleImpl();
  StyleImpl(const StyleImpl& style);
  StyleImpl& operator=(const StyleImpl& other);
  ~StyleImpl();

  void Reset() { flag_ = 0; }
  void ClearShapeStyle() const;
  const FontDescriptor& GetFontDescriptor() const { return font_descriptor_; }
  void SetFontDescriptor(const FontDescriptor& val) {
    font_descriptor_ = val;
    flag_ |= Style::FontDescriptorFlag;
    ClearShapeStyle();
  }
  float GetTextSize() const { return text_size_; }
  void SetTextSize(const float& val) {
    text_size_ = val;
    flag_ |= Style::TextSizeFlag;
    ClearShapeStyle();
  }
  float GetTextScale() const { return text_scale_; }
  void SetTextScale(const float& val) {
    text_scale_ = val;
    flag_ |= Style::TextScaleFlag;
    ClearShapeStyle();
  }
  TTColor GetForegroundColor() const { return fg_color_; }
  void SetForegroundColor(const TTColor& val) {
    fg_color_ = val;
    flag_ |= Style::ForegroundColorFlag;
  }
  TTColor GetBackgroundColor() const { return bg_color_; }
  void SetBackgroundColor(const TTColor& val) {
    bg_color_ = val;
    flag_ |= Style::BackgroundColorFlag;
  }
  TTColor GetDecorationColor() const { return decoration_color_; }
  void SetDecorationColor(const TTColor& val) {
    decoration_color_ = val;
    flag_ |= Style::DecorationColorFlag;
  }
  DecorationType GetDecorationType() const { return decoration_type_; }
  void SetDecorationType(const DecorationType& val) {
    decoration_type_ = val;
    flag_ |= Style::DecorationTypeFlag;
  }
  LineType GetDecorationStyle() const { return decoration_style_; }
  void SetDecorationStyle(const LineType& val) {
    decoration_style_ = val;
    flag_ |= Style::DecorationStyleFlag;
  }
  float GetDecorationThicknessMultiplier() const {
    return decoration_thickness_multiplier_;
  }
  void SetDecorationThicknessMultiplier(const float& val) {
    decoration_thickness_multiplier_ = val;
    flag_ |= Style::DecorationThicknessMultiplierFlag;
  }
  float GetDecorationElementLength() const {
    return decoration_element_length_;
  }
  void SetDecorationElementLength(const float& val) {
    decoration_element_length_ = val;
    flag_ |= Style::DecorationElementLengthFlag;
  }
  float GetDecorationGapLength() const { return decoration_gap_length_; }
  void SetDecorationGapLength(const float& val) {
    decoration_gap_length_ = val;
    flag_ |= Style::DecorationGapLengthFlag;
  }
  float GetDecorationSideMargin() const { return decoration_side_margin_; }
  void SetDecorationSideMargin(const float& val) {
    decoration_side_margin_ = val;
    flag_ |= Style::DecorationSideMarginFlag;
  }
  void SetTextStrokeStyle(const TTColor& color, float thickness);
  TTColor GetTextStrokeColor() const { return text_stroke_.style_.color_; }
  float GetTextStrokeWidth() const {
    return text_stroke_.style_.fixed_10x_thickness_ / 10.f;
  }
  uint64_t GetTextStrokeValue() const { return text_stroke_.value; }
  void SetTextStrokeValue(uint64_t val) {
    text_stroke_.value = val;
    flag_ |= Style::TextStrokeStyleFlag;
  }
  bool GetBold() const { return bold_; }
  void SetBold(const bool& val) {
    bold_ = val;
    flag_ |= Style::BoldFlag;
  }
  bool GetItalic() const { return italic_; }
  void SetItalic(const bool& val) {
    italic_ = val;
    flag_ |= Style::ItalicFlag;
  }
  CharacterVerticalAlignment GetVerticalAlignment() const {
    return vertical_alignment_;
  }
  void SetVerticalAlignment(const CharacterVerticalAlignment& val) {
    vertical_alignment_ = val;
    flag_ |= Style::VerticalAlignmentFlag;
    ClearShapeStyle();
  }
  float GetWordSpacing() const { return word_spacing_; }
  void SetWordSpacing(const float& val) {
    word_spacing_ = val;
    flag_ |= Style::WordSpacingFlag;
  }
  float GetLetterSpacing() const { return letter_spacing_; }
  void SetLetterSpacing(const float& val) {
    letter_spacing_ = val;
    flag_ |= Style::LetterSpacingFlag;
    ClearShapeStyle();
  }
  Painter* GetForegroundPainter() const { return fg_painter_; }
  void SetForegroundPainter(Painter* val) {
    fg_painter_ = val;
    flag_ |= Style::ForegroundPainterFlag;
  }
  Painter* GetBackgroundPainter() const { return bg_painter_; }
  void SetBackgroundPainter(Painter* val) {
    bg_painter_ = val;
    flag_ |= Style::BackgroundPainterFlag;
  }
  WordBreakType GetWordBreak() const { return word_break_; }
  void SetWordBreak(const WordBreakType& val) {
    word_break_ = val;
    flag_ |= Style::WordBreakFlag;
  }
  float GetBaselineOffset() const { return baseline_offset_; }
  void SetBaselineOffset(const float& val) {
    baseline_offset_ = val;
    flag_ |= Style::BaselineOffsetFlag;
  }
  float GetTextSkew() const { return text_skew_; }
  void SetTextSkew(const float& val) {
    text_skew_ = val;
    flag_ |= Style::TextSkewFlag;
  }
  const TextShadowList& GetTextShadowList() const { return text_shadow_list_; }
  void SetTextShadowList(const TextShadowList& val) {
    if (val.empty()) return;
    text_shadow_list_ = val;
    flag_ |= Style::TextShadowListFlag;
  }
  float GetScaledTextSize() const { return GetTextSize() * GetTextScale(); }
  bool HasAttribute(AttributeType type) const { return flag_ & (1u << type); }
  bool HasStyleAttribute(AttrType style_flag) const {
    return flag_ & style_flag;
  }
  const ShapeStyle& GetShapeStyle() const;
  static const StyleImpl& DefaultStyle();

 public:
  FontDescriptor font_descriptor_;
  float text_size_;
  float text_scale_;
  TTColor fg_color_;
  TTColor bg_color_;
  TTColor decoration_color_;
  DecorationType decoration_type_;
  LineType decoration_style_;
  float decoration_thickness_multiplier_;
  float decoration_element_length_;
  float decoration_gap_length_;
  float decoration_side_margin_;
  DecorationStyle text_stroke_;
  bool bold_;
  bool italic_;
  CharacterVerticalAlignment vertical_alignment_;
  float word_spacing_;
  float letter_spacing_;
  Painter* fg_painter_;
  Painter* bg_painter_;
  WordBreakType word_break_;
  float baseline_offset_;
  float text_skew_;
  TextShadowList text_shadow_list_;
  AttrType flag_;
  mutable std::unique_ptr<ShapeStyle> shape_style_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_TEXTLAYOUT_STYLE_STYLE_IMPL_H_
