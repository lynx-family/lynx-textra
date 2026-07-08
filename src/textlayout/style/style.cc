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
  if (other.HasStyleAttribute(Style::DecorationElementLengthFlag))
    SetDecorationElementLength(other.GetDecorationElementLength());
  if (other.HasStyleAttribute(Style::DecorationGapLengthFlag))
    SetDecorationGapLength(other.GetDecorationGapLength());
  if (other.HasStyleAttribute(Style::DecorationSideMarginFlag))
    SetDecorationSideMargin(other.GetDecorationSideMargin());
  if (other.HasStyleAttribute(Style::DecorationOffsetFlag))
    SetAttribute(kDecorationOffset, other.GetAttribute(kDecorationOffset));
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
  if (other.HasStyleAttribute(Style::TextSkewFlag)) {
    SetTextSkew(other.GetTextSkew());
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
bool Style::SetAttribute(AttributeType attribute, const StyleValue& value) {
  switch (attribute) {
    case kFontDescriptor: {
      if (!value.IsPointer() || value.Pointer() == 0 ||
          value.length != sizeof(FontDescriptor)) {
        return false;
      }
      SetFontDescriptor(
          *reinterpret_cast<const FontDescriptor*>(value.Pointer()));
      return true;
    }
    case kTextSize:
      if (!value.IsNumber()) {
        return false;
      }
      SetTextSize(value.Float());
      return true;
    case kTextScale:
      if (!value.IsNumber()) {
        return false;
      }
      SetTextScale(value.Float());
      return true;
    case kVerticalAlignment:
      if (!value.IsNumber()) {
        return false;
      }
      SetVerticalAlignment(
          static_cast<CharacterVerticalAlignment>(value.Int32()));
      return true;
    case kWordSpacing:
      if (!value.IsNumber()) {
        return false;
      }
      SetWordSpacing(value.Float());
      return true;
    case kLetterSpacing:
      if (!value.IsNumber()) {
        return false;
      }
      SetLetterSpacing(value.Float());
      return true;
    case kForegroundColor:
      if (!value.IsNumber()) {
        return false;
      }
      SetForegroundColor(value.UInt32());
      return true;
    case kBackgroundColor:
      if (!value.IsNumber()) {
        return false;
      }
      SetBackgroundColor(value.UInt32());
      return true;
    case kDecorationColor:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationColor(value.UInt32());
      return true;
    case kDecorationType:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationType(static_cast<DecorationType>(value.Int32()));
      return true;
    case kDecorationStyle:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationStyle(static_cast<LineType>(value.Int32()));
      return true;
    case kDecorationThicknessMultiplier:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationThicknessMultiplier(value.Float());
      return true;
    case kTextStrokeStyle:
      if (!value.IsNumber()) {
        return false;
      }
      SetTextStrokeValue(value.UInt64());
      return true;
    case kBold:
      if (!value.IsBoolean()) {
        return false;
      }
      SetBold(value.Bool());
      return true;
    case kItalic:
      if (!value.IsBoolean()) {
        return false;
      }
      SetItalic(value.Bool());
      return true;
    case kTextShadowList:
      if (!value.IsPointer() || value.Pointer() == 0 ||
          value.length != sizeof(TextShadowList)) {
        return false;
      }
      SetTextShadowList(
          *reinterpret_cast<const TextShadowList*>(value.Pointer()));
      return true;
    case kForegroundPainter:
      if (!value.IsPointer()) {
        return false;
      }
      SetForegroundPainter(reinterpret_cast<Painter*>(value.Pointer()));
      return true;
    case kBackgroundPainter:
      if (!value.IsPointer()) {
        return false;
      }
      SetBackgroundPainter(reinterpret_cast<Painter*>(value.Pointer()));
      return true;
    case kWordBreak:
      if (!value.IsNumber()) {
        return false;
      }
      SetWordBreak(static_cast<WordBreakType>(value.Int32()));
      return true;
    case kBaselineOffset:
      if (!value.IsNumber()) {
        return false;
      }
      SetBaselineOffset(value.Float());
      return true;
    case kTextSkew:
      if (!value.IsNumber()) {
        return false;
      }
      SetTextSkew(value.Float());
      return true;
    case kDecorationElementLength:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationElementLength(value.Float());
      return true;
    case kDecorationGapLength:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationGapLength(value.Float());
      return true;
    case kDecorationSideMargin:
      if (!value.IsNumber()) {
        return false;
      }
      SetDecorationSideMargin(value.Float());
      return true;
    case kDecorationOffset:
      if (!value.IsNumber()) {
        return false;
      }
      decoration_offset_ = value.Float();
      flag_ |= DecorationOffsetFlag;
      return true;
    default:
      return false;
  }
}

StyleValue Style::GetAttribute(AttributeType attribute) const {
  StyleValue value;
  GetAttribute(attribute, &value);
  return value;
}

bool Style::GetAttribute(AttributeType attribute, StyleValue* value) const {
  if (value == nullptr) {
    return false;
  }

  switch (attribute) {
    case kFontDescriptor:
      *value = StyleValue(&font_descriptor_, sizeof(FontDescriptor));
      return true;
    case kTextSize:
      *value = StyleValue(GetTextSize());
      return true;
    case kTextScale:
      *value = StyleValue(GetTextScale());
      return true;
    case kVerticalAlignment:
      *value = StyleValue(static_cast<int32_t>(GetVerticalAlignment()));
      return true;
    case kWordSpacing:
      *value = StyleValue(GetWordSpacing());
      return true;
    case kLetterSpacing:
      *value = StyleValue(GetLetterSpacing());
      return true;
    case kForegroundColor:
      *value = StyleValue(GetForegroundColor().GetPlainColor());
      return true;
    case kBackgroundColor:
      *value = StyleValue(GetBackgroundColor().GetPlainColor());
      return true;
    case kDecorationColor:
      *value = StyleValue(GetDecorationColor().GetPlainColor());
      return true;
    case kDecorationType:
      *value = StyleValue(static_cast<int32_t>(GetDecorationType()));
      return true;
    case kDecorationStyle:
      *value = StyleValue(static_cast<int32_t>(GetDecorationStyle()));
      return true;
    case kDecorationThicknessMultiplier:
      *value = StyleValue(GetDecorationThicknessMultiplier());
      return true;
    case kTextStrokeStyle:
      *value = StyleValue(GetTextStrokeValue());
      return true;
    case kBold:
      *value = StyleValue(GetBold());
      return true;
    case kItalic:
      *value = StyleValue(GetItalic());
      return true;
    case kTextShadowList:
      *value = StyleValue(&text_shadow_list_, sizeof(TextShadowList));
      return true;
    case kForegroundPainter:
      *value = StyleValue(GetForegroundPainter());
      return true;
    case kBackgroundPainter:
      *value = StyleValue(GetBackgroundPainter());
      return true;
    case kWordBreak:
      *value = StyleValue(static_cast<int32_t>(GetWordBreak()));
      return true;
    case kBaselineOffset:
      *value = StyleValue(GetBaselineOffset());
      return true;
    case kTextSkew:
      *value = StyleValue(GetTextSkew());
      return true;
    case kDecorationElementLength:
      *value = StyleValue(GetDecorationElementLength());
      return true;
    case kDecorationGapLength:
      *value = StyleValue(GetDecorationGapLength());
      return true;
    case kDecorationSideMargin:
      *value = StyleValue(GetDecorationSideMargin());
      return true;
    case kDecorationOffset:
      *value = StyleValue(decoration_offset_);
      return true;
    default:
      *value = StyleValue();
      return false;
  }
}
}  // namespace tttext
}  // namespace ttoffice
