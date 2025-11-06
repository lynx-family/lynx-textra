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
  if (other.HasAttribute(kFontDescriptor))
    SetFontDescriptor(other.GetFontDescriptor());
  if (other.HasAttribute(kTextSize)) SetTextSize(other.GetTextSize());
  if (other.HasAttribute(kTextScale)) SetTextScale(other.GetTextScale());
  if (other.HasAttribute(kForegroundColor))
    SetForegroundColor(other.GetForegroundColor());
  if (other.HasAttribute(kBackgroundColor))
    SetBackgroundColor(other.GetBackgroundColor());
  if (other.HasAttribute(kDecorationColor))
    SetDecorationColor(other.GetDecorationColor());
  if (other.HasAttribute(kDecorationType))
    SetDecorationType(other.GetDecorationType());
  if (other.HasAttribute(kDecorationStyle))
    SetDecorationStyle(other.GetDecorationStyle());
  if (other.HasAttribute(kDecorationThicknessMultiplier))
    SetDecorationThicknessMultiplier(other.GetDecorationThicknessMultiplier());
  if (other.HasAttribute(kTextStrokeStyle)) {
    text_stroke_ = other.text_stroke_;
  }
  if (other.HasAttribute(kBold)) SetBold(other.GetBold());
  if (other.HasAttribute(kItalic)) SetItalic(other.GetItalic());
  if (other.HasAttribute(kVerticalAlignment))
    SetVerticalAlignment(other.GetVerticalAlignment());
  if (other.HasAttribute(kWordSpacing)) SetWordSpacing(other.GetWordSpacing());
  if (other.HasAttribute(kLetterSpacing))
    SetLetterSpacing(other.GetLetterSpacing());
  if (other.HasAttribute(kTextShadowList))
    SetTextShadowList(other.GetTextShadowList());
  if (other.HasAttribute(kForegroundPainter))
    SetForegroundPainter(other.GetForegroundPainter());
  if (other.HasAttribute(kBackgroundPainter))
    SetBackgroundPainter(other.GetBackgroundPainter());
  if (other.HasAttribute(kWordBreak)) {
    SetWordBreak(other.GetWordBreak());
  }
  if (other.HasAttribute(kBaselineOffset)) {
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
