// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/style.h>

#include <memory>
#include <utility>

#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style_attributes.h"

namespace ttoffice {
namespace tttext {
const Style& Style::DefaultStyle() {
  static const Style DEFAULT_STYLE;
  return DEFAULT_STYLE;
}
Style::Style() = default;
Style::Style(const Style& style) { *this = style; }
Style& Style::operator=(const Style& other) {
  if (&other == this) return *this;
  if (other.HasStyleAttribute(Style::FontDescriptorFlag))
    SetFontDescriptor(other.GetFontDescriptor());
  if (other.HasStyleAttribute(Style::TextSizeFlag))
    SetTextSize(other.GetTextSize());
  if (other.HasStyleAttribute(Style::TextScaleFlag))
    SetTextScale(other.GetTextScale());
  if (other.HasStyleAttribute(Style::ForegroundColorFlag))
    SetForegroundColor(other.GetForegroundColor());
  if (other.HasStyleAttribute(Style::BackgroundColorFlag))
    SetBackgroundColor(other.GetBackgroundColor());
  if (other.HasStyleAttribute(Style::DecorationColorFlag))
    SetDecorationColor(other.GetDecorationColor());
  if (other.HasStyleAttribute(Style::DecorationTypeFlag))
    SetDecorationType(other.GetDecorationType());
  if (other.HasStyleAttribute(Style::DecorationStyleFlag))
    SetDecorationStyle(other.GetDecorationStyle());
  if (other.HasStyleAttribute(Style::DecorationThicknessMultiplierFlag))
    SetDecorationThicknessMultiplier(other.GetDecorationThicknessMultiplier());
  if (other.HasStyleAttribute(Style::TextStrokeStyleFlag)) {
    SetTextStrokeValue(other.GetTextStrokeValue());
  }
  if (other.HasStyleAttribute(Style::BoldFlag)) SetBold(other.GetBold());
  if (other.HasStyleAttribute(Style::ItalicFlag)) SetItalic(other.GetItalic());
  if (other.HasStyleAttribute(Style::VerticalAlignmentFlag))
    SetVerticalAlignment(other.GetVerticalAlignment());
  if (other.HasStyleAttribute(Style::WordSpacingFlag))
    SetWordSpacing(other.GetWordSpacing());
  if (other.HasStyleAttribute(Style::LetterSpacingFlag))
    SetLetterSpacing(other.GetLetterSpacing());
  if (other.HasStyleAttribute(Style::TextShadowListFlag))
    SetTextShadowList(other.GetTextShadowList());
  if (other.HasStyleAttribute(Style::ForegroundPainterFlag))
    SetForegroundPainter(other.GetForegroundPainter());
  if (other.HasStyleAttribute(Style::BackgroundPainterFlag))
    SetBackgroundPainter(other.GetBackgroundPainter());
  if (other.HasStyleAttribute(Style::WordBreakFlag)) {
    SetWordBreak(other.GetWordBreak());
  }
  if (other.HasStyleAttribute(Style::BaselineOffsetFlag)) {
    SetBaselineOffset(other.GetBaselineOffset());
  }
  flag_ = other.flag_;
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
