// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/style/style_impl.h"

#include <textra/style.h>

#include <cmath>
#include <limits>
#include <memory>

#include "src/textlayout/shape_cache.h"

namespace ttoffice {
namespace tttext {
StyleImpl::StyleImpl()
    : font_descriptor_{{}, FontStyle::Normal(), 0},
      text_size_(10.f * 96 / 72),
      text_scale_(1.0f),
      fg_color_(TTColor::BLACK),
      bg_color_(TTColor::UNDEFINED),
      decoration_color_(TTColor::UNDEFINED),
      decoration_type_(DecorationType::kNone),
      decoration_style_(LineType::kSolid),
      decoration_thickness_multiplier_(1.0f),
      decoration_element_length_(2.5f),
      decoration_gap_length_(1.5f),
      decoration_side_margin_(2.5f),
      bold_(false),
      italic_(false),
      vertical_alignment_(CharacterVerticalAlignment::kBaseLine),
      word_spacing_(0),
      letter_spacing_(0),
      fg_painter_(nullptr),
      bg_painter_(nullptr),
      word_break_(WordBreakType::kNormal),
      baseline_offset_(0.f),
      text_skew_(0.f),
      text_shadow_list_({}),
      flag_(0),
      shape_style_(nullptr) {
  text_stroke_.style_.color_ = TTColor::UNDEFINED;
  text_stroke_.style_.fixed_10x_thickness_ = 10;
  text_stroke_.style_.line_type_ = LineType::kSolid;
  text_stroke_.style_.padding_ = 0;
}
StyleImpl::StyleImpl(const StyleImpl& style) : StyleImpl() { *this = style; }
StyleImpl& StyleImpl::operator=(const StyleImpl& other) {
  if (&other == this) return *this;
  font_descriptor_ = other.font_descriptor_;
  text_size_ = other.text_size_;
  text_scale_ = other.text_scale_;
  fg_color_ = other.fg_color_;
  bg_color_ = other.bg_color_;
  decoration_color_ = other.decoration_color_;
  decoration_type_ = other.decoration_type_;
  decoration_style_ = other.decoration_style_;
  decoration_thickness_multiplier_ = other.decoration_thickness_multiplier_;
  decoration_element_length_ = other.decoration_element_length_;
  decoration_gap_length_ = other.decoration_gap_length_;
  decoration_side_margin_ = other.decoration_side_margin_;
  text_stroke_ = other.text_stroke_;
  bold_ = other.bold_;
  italic_ = other.italic_;
  vertical_alignment_ = other.vertical_alignment_;
  word_spacing_ = other.word_spacing_;
  letter_spacing_ = other.letter_spacing_;
  fg_painter_ = other.fg_painter_;
  bg_painter_ = other.bg_painter_;
  word_break_ = other.word_break_;
  baseline_offset_ = other.baseline_offset_;
  text_skew_ = other.text_skew_;
  text_shadow_list_ = other.text_shadow_list_;
  flag_ = other.flag_;
  shape_style_ = nullptr;
  return *this;
}
StyleImpl::~StyleImpl() = default;
void StyleImpl::ClearShapeStyle() const { shape_style_ = nullptr; }
void StyleImpl::SetTextStrokeStyle(const TTColor& color, float thickness) {
  text_stroke_.style_.color_ = color;
  if (std::isinf(thickness)) thickness = 0.0f;
  auto clamped_thickness =
      std::fmin(std::fmax(thickness * 10, 0.0f),
                static_cast<float>(std::numeric_limits<uint16_t>::max()));
  text_stroke_.style_.fixed_10x_thickness_ = std::round(clamped_thickness);
  flag_ |= Style::TextStrokeStyleFlag;
}
const ShapeStyle& StyleImpl::GetShapeStyle() const {
  if (shape_style_ == nullptr) {
    shape_style_ = std::make_unique<ShapeStyle>(
        font_descriptor_, GetScaledTextSize(), false, false);
  }
  return *shape_style_;
}
const StyleImpl& StyleImpl::DefaultStyle() {
  static const StyleImpl DEFAULT_STYLE;
  return DEFAULT_STYLE;
}

const Style& Style::DefaultStyle() {
  static const Style DEFAULT_STYLE;
  return DEFAULT_STYLE;
}
Style::Style() : impl_(std::make_unique<StyleImpl>()) {}
Style::Style(const Style& style) : Style(style.GetImpl()) {}
Style::Style(const StyleImpl& style)
    : impl_(std::make_unique<StyleImpl>(style)) {}
Style& Style::operator=(const Style& other) {
  if (&other == this) return *this;
  return *this = other.GetImpl();
}
Style& Style::operator=(const StyleImpl& other) {
  *impl_ = other;
  return *this;
}
Style::~Style() = default;
void Style::Reset() { impl_->Reset(); }
void Style::ClearShapeStyle() const { impl_->ClearShapeStyle(); }
const FontDescriptor& Style::GetFontDescriptor() const {
  return impl_->GetFontDescriptor();
}
void Style::SetFontDescriptor(const FontDescriptor& val) {
  impl_->SetFontDescriptor(val);
}
float Style::GetTextSize() const { return impl_->GetTextSize(); }
void Style::SetTextSize(const float& val) { impl_->SetTextSize(val); }
float Style::GetTextScale() const { return impl_->GetTextScale(); }
void Style::SetTextScale(const float& val) { impl_->SetTextScale(val); }
TTColor Style::GetForegroundColor() const {
  return impl_->GetForegroundColor();
}
void Style::SetForegroundColor(const TTColor& val) {
  impl_->SetForegroundColor(val);
}
TTColor Style::GetBackgroundColor() const {
  return impl_->GetBackgroundColor();
}
void Style::SetBackgroundColor(const TTColor& val) {
  impl_->SetBackgroundColor(val);
}
TTColor Style::GetDecorationColor() const {
  return impl_->GetDecorationColor();
}
void Style::SetDecorationColor(const TTColor& val) {
  impl_->SetDecorationColor(val);
}
DecorationType Style::GetDecorationType() const {
  return impl_->GetDecorationType();
}
void Style::SetDecorationType(const DecorationType& val) {
  impl_->SetDecorationType(val);
}
LineType Style::GetDecorationStyle() const {
  return impl_->GetDecorationStyle();
}
void Style::SetDecorationStyle(const LineType& val) {
  impl_->SetDecorationStyle(val);
}
float Style::GetDecorationThicknessMultiplier() const {
  return impl_->GetDecorationThicknessMultiplier();
}
void Style::SetDecorationThicknessMultiplier(const float& val) {
  impl_->SetDecorationThicknessMultiplier(val);
}
float Style::GetDecorationElementLength() const {
  return impl_->GetDecorationElementLength();
}
void Style::SetDecorationElementLength(const float& val) {
  impl_->SetDecorationElementLength(val);
}
float Style::GetDecorationGapLength() const {
  return impl_->GetDecorationGapLength();
}
void Style::SetDecorationGapLength(const float& val) {
  impl_->SetDecorationGapLength(val);
}
float Style::GetDecorationSideMargin() const {
  return impl_->GetDecorationSideMargin();
}
void Style::SetDecorationSideMargin(const float& val) {
  impl_->SetDecorationSideMargin(val);
}
void Style::SetTextStrokeStyle(const TTColor& color, float thickness) {
  impl_->SetTextStrokeStyle(color, thickness);
}
TTColor Style::GetTextStrokeColor() const {
  return impl_->GetTextStrokeColor();
}
float Style::GetTextStrokeWidth() const { return impl_->GetTextStrokeWidth(); }
uint64_t Style::GetTextStrokeValue() const {
  return impl_->GetTextStrokeValue();
}
void Style::SetTextStrokeValue(uint64_t val) { impl_->SetTextStrokeValue(val); }
bool Style::GetBold() const { return impl_->GetBold(); }
void Style::SetBold(const bool& val) { impl_->SetBold(val); }
bool Style::GetItalic() const { return impl_->GetItalic(); }
void Style::SetItalic(const bool& val) { impl_->SetItalic(val); }
CharacterVerticalAlignment Style::GetVerticalAlignment() const {
  return impl_->GetVerticalAlignment();
}
void Style::SetVerticalAlignment(const CharacterVerticalAlignment& val) {
  impl_->SetVerticalAlignment(val);
}
float Style::GetWordSpacing() const { return impl_->GetWordSpacing(); }
void Style::SetWordSpacing(const float& val) { impl_->SetWordSpacing(val); }
float Style::GetLetterSpacing() const { return impl_->GetLetterSpacing(); }
void Style::SetLetterSpacing(const float& val) { impl_->SetLetterSpacing(val); }
Painter* Style::GetForegroundPainter() const {
  return impl_->GetForegroundPainter();
}
void Style::SetForegroundPainter(Painter* val) {
  impl_->SetForegroundPainter(val);
}
Painter* Style::GetBackgroundPainter() const {
  return impl_->GetBackgroundPainter();
}
void Style::SetBackgroundPainter(Painter* val) {
  impl_->SetBackgroundPainter(val);
}
WordBreakType Style::GetWordBreak() const { return impl_->GetWordBreak(); }
void Style::SetWordBreak(const WordBreakType& val) { impl_->SetWordBreak(val); }
float Style::GetBaselineOffset() const { return impl_->GetBaselineOffset(); }
void Style::SetBaselineOffset(const float& val) {
  impl_->SetBaselineOffset(val);
}
float Style::GetTextSkew() const { return impl_->GetTextSkew(); }
void Style::SetTextSkew(const float& val) { impl_->SetTextSkew(val); }
const Style::TextShadowList& Style::GetTextShadowList() const {
  return impl_->GetTextShadowList();
}
void Style::SetTextShadowList(const TextShadowList& val) {
  impl_->SetTextShadowList(val);
}
float Style::GetScaledTextSize() const { return impl_->GetScaledTextSize(); }
bool Style::HasAttribute(const AttributeType type) const {
  return impl_->HasAttribute(type);
}
bool Style::HasStyleAttribute(const AttrType style_flag) const {
  return impl_->HasStyleAttribute(style_flag);
}
const ShapeStyle& Style::GetShapeStyle() const {
  return impl_->GetShapeStyle();
}
StyleImpl& Style::GetImpl() { return *impl_; }
const StyleImpl& Style::GetImpl() const { return *impl_; }
}  // namespace tttext
}  // namespace ttoffice
