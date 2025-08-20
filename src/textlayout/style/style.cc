// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/style.h>
#include <textra/text_line.h>

#include <memory>
#include <utility>

#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style_attributes.h"

namespace ttoffice {
namespace tttext {
const Style& Style::DefaultStyle() {
  static const Style DEFAULT_STYLE{
      {{}, FontStyle::Normal(), 0},           // FontDescriptorList
      10.f * 96 / 72,                         // TextSize
      1.f,                                    // TextScale
      TTColor{TTColor::BLACK()},              // FgColor
      TTColor::UNDEFINED(),                   // BgColor
      TTColor{},                              // DecorationColor
      DecorationType::kNone,                  // DecorationType
      LineType::kSolid,                       // DecorationStyle
      1.0f,                                   // DecorationThicknessMultiplier
      false,                                  // Bold
      false,                                  // Italic
      CharacterVerticalAlignment::kBaseLine,  // VerticalAlignment
      0.f,                                    // WordSpacing
      0.f,                                    // LetterSpacing
      {},                                     // TextShadowList
      nullptr,                                // Foreground Painter
      nullptr,                                // Background Painter
      WordBreakType::kNormal,                 // WordBreakType
      0,                                      // Baseline Offset
      Style::FullFlag()};
  return DEFAULT_STYLE;
}
Style::Style()
    : text_size_(0),
      text_scale_(1),
      fg_color_(),
      bg_color_(),
      decoration_color_(TTColor::UNDEFINED()),
      decoration_type_(DecorationType::kNone),
      decoration_style_(LineType::kSolid),
      decoration_thickness_multiplier_(1.f),
      bold_(false),
      italic_(false),
      vertical_alignment_(CharacterVerticalAlignment::kBaseLine),
      word_spacing_(0.f),
      letter_spacing_(0.f),
      fg_painter_(0),
      bg_painter_(0),
      word_break_(WordBreakType::kNormal),
      baseline_offset_(0),
      flag_(0) {}
Style::Style(FontDescriptor font_descriptor, float text_size, float text_scale,
             const TTColor& fg_color, const TTColor& bg_color,
             const TTColor& decoration_color, DecorationType decoration_type,
             LineType decoration_style, float decoration_thickness_multiplier,
             bool bold, bool italic,
             CharacterVerticalAlignment vertical_alignment, float word_spacing,
             float letter_spacing, const TextShadowList& text_shadow_list,
             Painter* foreground_painter, Painter* background_painter,
             WordBreakType word_break, float baseline_offset, uint32_t flag)
    : font_descriptor_({std::move(font_descriptor)}),
      text_size_(text_size),
      text_scale_(text_scale),
      fg_color_(fg_color),
      bg_color_(bg_color),
      decoration_color_(decoration_color),
      decoration_type_(decoration_type),
      decoration_style_(decoration_style),
      decoration_thickness_multiplier_(decoration_thickness_multiplier),
      bold_(bold),
      italic_(italic),
      vertical_alignment_(vertical_alignment),
      word_spacing_(word_spacing),
      letter_spacing_(letter_spacing),
      fg_painter_(foreground_painter),
      bg_painter_(background_painter),
      word_break_(word_break),
      baseline_offset_(0),
      text_shadow_list_(text_shadow_list),
      flag_(flag) {}
Style::Style(const Style& style) : Style() { *this = style; }
Style& Style::operator=(const Style& other) {
  if (&other == this) return *this;
  if (other.HasFontDescriptor()) SetFontDescriptor(other.GetFontDescriptor());
  if (other.HasTextSize()) SetTextSize(other.GetTextSize());
  if (other.HasTextScale()) SetTextScale(other.GetTextScale());
  if (other.HasForegroundColor())
    SetForegroundColor(other.GetForegroundColor());
  if (other.HasBackgroundColor())
    SetBackgroundColor(other.GetBackgroundColor());
  if (other.HasDecorationColor())
    SetDecorationColor(other.GetDecorationColor());
  if (other.HasDecorationType()) SetDecorationType(other.GetDecorationType());
  if (other.HasDecorationStyle())
    SetDecorationStyle(other.GetDecorationStyle());
  if (other.HasDecorationThicknessMultiplier())
    SetDecorationThicknessMultiplier(other.GetDecorationThicknessMultiplier());
  if (other.HasBold()) SetBold(other.GetBold());
  if (other.HasItalic()) SetItalic(other.GetItalic());
  if (other.HasVerticalAlignment())
    SetVerticalAlignment(other.GetVerticalAlignment());
  if (other.HasWordSpacing()) SetWordSpacing(other.GetWordSpacing());
  if (other.HasLetterSpacing()) SetLetterSpacing(other.GetLetterSpacing());
  if (other.HasTextShadowList()) SetTextShadowList(other.GetTextShadowList());
  if (other.HasForegroundPainter())
    SetForegroundPainter(other.GetForegroundPainter());
  if (other.HasBackgroundPainter())
    SetBackgroundPainter(other.GetBackgroundPainter());
  if (other.HasWordBreak()) {
    SetWordBreak(other.GetWordBreak());
  }
  if (other.HasBaselineOffset()) {
    SetBaselineOffset(other.GetBaselineOffset());
  }
  return *this;
}
Style::~Style() = default;
void Style::ClearShapeStyle() const { shape_style_ = nullptr; }
const ShapeStyle& Style::GetShapeStyle() const {
  if (shape_style_ == nullptr) {
    shape_style_ = std::make_unique<ShapeStyle>(
        font_descriptor_, GetScaledTextSize(), false, false);
  }
  return *shape_style_;
}
}  // namespace tttext
}  // namespace ttoffice
