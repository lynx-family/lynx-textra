// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_internal.h"
#include "src/textlayout/style/style_impl.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL textra_style_create(textra_style_t** out_style) {
  if (!out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = nullptr;
  *out_style = new textra_style();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_text_size(textra_style_t* style,
                                                       float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value) && value > 0)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetTextSize(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_get_text_size(const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetTextSize());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_text_scale(textra_style_t* style,
                                                        float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value) && value > 0)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetTextScale(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_get_text_scale(const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetTextScale());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_foreground_color(textra_style_t* style, uint32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetForegroundColor(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_foreground_color(
    const textra_style_t* style, uint32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<uint32_t>(style->impl.GetForegroundColor());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_background_color(textra_style_t* style, uint32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetBackgroundColor(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_background_color(
    const textra_style_t* style, uint32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<uint32_t>(style->impl.GetBackgroundColor());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_decoration_color(textra_style_t* style, uint32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDecorationColor(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_color(
    const textra_style_t* style, uint32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<uint32_t>(style->impl.GetDecorationColor());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_decoration_type(
    textra_style_t* style, textra_decoration_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(value <= 7)) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetDecorationType(static_cast<tt::DecorationType>(value));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_type(
    const textra_style_t* style, textra_decoration_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_decoration_t>(style->impl.GetDecorationType());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_decoration_style(
    textra_style_t* style, textra_line_style_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(value <= TEXTRA_LINE_STYLE_WAVY)) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetDecorationStyle(static_cast<tt::LineType>(value));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_style(
    const textra_style_t* style, textra_line_style_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_line_style_t>(style->impl.GetDecorationStyle());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_decoration_thickness_multiplier(
    textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Nonnegative(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDecorationThicknessMultiplier(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_thickness_multiplier(
    const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<float>(style->impl.GetDecorationThicknessMultiplier());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_decoration_element_length(textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Nonnegative(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDecorationElementLength(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_element_length(
    const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetDecorationElementLength());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_decoration_gap_length(textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Nonnegative(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDecorationGapLength(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_gap_length(
    const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetDecorationGapLength());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_decoration_side_margin(textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Nonnegative(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDecorationSideMargin(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_decoration_side_margin(
    const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetDecorationSideMargin());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_bold(textra_style_t* style,
                                                  textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Bool(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetBold(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_bold(const textra_style_t* style,
                                                  textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->impl.GetBold());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_italic(textra_style_t* style,
                                                    textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Bool(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetItalic(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_italic(const textra_style_t* style,
                                                    textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->impl.GetItalic());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_vertical_alignment(
    textra_style_t* style, textra_character_alignment_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(value <= TEXTRA_CHARACTER_ALIGNMENT_BOTTOM))
    return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetVerticalAlignment(
      static_cast<tt::CharacterVerticalAlignment>(value));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_vertical_alignment(
    const textra_style_t* style, textra_character_alignment_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_character_alignment_t>(
      style->impl.GetVerticalAlignment());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_word_spacing(textra_style_t* style,
                                                          float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetWordSpacing(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_get_word_spacing(const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetWordSpacing());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_letter_spacing(textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetLetterSpacing(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_get_letter_spacing(const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetLetterSpacing());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_word_break(textra_style_t* style, textra_word_break_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(value <= TEXTRA_WORD_BREAK_KEEP_ALL)) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetWordBreak(static_cast<tt::WordBreakType>(value));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_word_break(
    const textra_style_t* style, textra_word_break_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_word_break_t>(style->impl.GetWordBreak());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_set_baseline_offset(textra_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetBaselineOffset(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_baseline_offset(
    const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetBaselineOffset());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_text_skew(textra_style_t* style,
                                                       float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!(Finite(value))) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetTextSkew(value);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_style_get_text_skew(const textra_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetTextSkew());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_has_attribute(
    const textra_style_t* style, textra_style_attribute_t attribute,
    textra_bool_t* out_is_set) {
  if (!style || !out_is_set) return TEXTRA_STATUS_INVALID_ARGUMENT;
  tt::AttributeType internal;
  switch (attribute) {
    case TEXTRA_STYLE_ATTRIBUTE_FONT_DESCRIPTOR:
      internal = tt::kFontDescriptor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE:
      internal = tt::kTextSize;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SCALE:
      internal = tt::kTextScale;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR:
      internal = tt::kForegroundColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BACKGROUND_COLOR:
      internal = tt::kBackgroundColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_COLOR:
      internal = tt::kDecorationColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_TYPE:
      internal = tt::kDecorationType;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_STYLE:
      internal = tt::kDecorationStyle;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_THICKNESS_MULTIPLIER:
      internal = tt::kDecorationThicknessMultiplier;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_ELEMENT_LENGTH:
      internal = tt::kDecorationElementLength;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_GAP_LENGTH:
      internal = tt::kDecorationGapLength;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_SIDE_MARGIN:
      internal = tt::kDecorationSideMargin;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BOLD:
      internal = tt::kBold;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_ITALIC:
      internal = tt::kItalic;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_VERTICAL_ALIGNMENT:
      internal = tt::kVerticalAlignment;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_SPACING:
      internal = tt::kWordSpacing;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_LETTER_SPACING:
      internal = tt::kLetterSpacing;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_BREAK:
      internal = tt::kWordBreak;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BASELINE_OFFSET:
      internal = tt::kBaselineOffset;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SKEW:
      internal = tt::kTextSkew;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_STROKE:
      internal = tt::kTextStrokeStyle;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_GRADIENT:
      internal = tt::kForegroundGradient;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SHADOWS:
      internal = tt::kTextShadowList;
      break;
    default:
      return TEXTRA_STATUS_UNSUPPORTED;
  }
  *out_is_set = style->impl.HasAttribute(internal);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_unset_attribute(
    textra_style_t* style, textra_style_attribute_t attribute) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  tt::AttributeType internal;
  switch (attribute) {
    case TEXTRA_STYLE_ATTRIBUTE_FONT_DESCRIPTOR:
      internal = tt::kFontDescriptor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE:
      internal = tt::kTextSize;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SCALE:
      internal = tt::kTextScale;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR:
      internal = tt::kForegroundColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BACKGROUND_COLOR:
      internal = tt::kBackgroundColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_COLOR:
      internal = tt::kDecorationColor;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_TYPE:
      internal = tt::kDecorationType;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_STYLE:
      internal = tt::kDecorationStyle;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_THICKNESS_MULTIPLIER:
      internal = tt::kDecorationThicknessMultiplier;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_ELEMENT_LENGTH:
      internal = tt::kDecorationElementLength;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_GAP_LENGTH:
      internal = tt::kDecorationGapLength;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_SIDE_MARGIN:
      internal = tt::kDecorationSideMargin;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BOLD:
      internal = tt::kBold;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_ITALIC:
      internal = tt::kItalic;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_VERTICAL_ALIGNMENT:
      internal = tt::kVerticalAlignment;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_SPACING:
      internal = tt::kWordSpacing;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_LETTER_SPACING:
      internal = tt::kLetterSpacing;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_BREAK:
      internal = tt::kWordBreak;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BASELINE_OFFSET:
      internal = tt::kBaselineOffset;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SKEW:
      internal = tt::kTextSkew;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_STROKE:
      internal = tt::kTextStrokeStyle;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_GRADIENT:
      internal = tt::kForegroundGradient;
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SHADOWS:
      internal = tt::kTextShadowList;
      break;
    default:
      return TEXTRA_STATUS_UNSUPPORTED;
  }
  tt::Style defaults;
  switch (attribute) {
    case TEXTRA_STYLE_ATTRIBUTE_FONT_DESCRIPTOR:
      style->impl.SetFontDescriptor(defaults.GetFontDescriptor());
      style->typeface = {};
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE:
      style->impl.SetTextSize(defaults.GetTextSize());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SCALE:
      style->impl.SetTextScale(defaults.GetTextScale());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR:
      style->impl.SetForegroundColor(defaults.GetForegroundColor());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BACKGROUND_COLOR:
      style->impl.SetBackgroundColor(defaults.GetBackgroundColor());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_COLOR:
      style->impl.SetDecorationColor(defaults.GetDecorationColor());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_TYPE:
      style->impl.SetDecorationType(defaults.GetDecorationType());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_STYLE:
      style->impl.SetDecorationStyle(defaults.GetDecorationStyle());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_THICKNESS_MULTIPLIER:
      style->impl.SetDecorationThicknessMultiplier(
          defaults.GetDecorationThicknessMultiplier());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_ELEMENT_LENGTH:
      style->impl.SetDecorationElementLength(
          defaults.GetDecorationElementLength());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_GAP_LENGTH:
      style->impl.SetDecorationGapLength(defaults.GetDecorationGapLength());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_DECORATION_SIDE_MARGIN:
      style->impl.SetDecorationSideMargin(defaults.GetDecorationSideMargin());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BOLD:
      style->impl.SetBold(defaults.GetBold());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_ITALIC:
      style->impl.SetItalic(defaults.GetItalic());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_VERTICAL_ALIGNMENT:
      style->impl.SetVerticalAlignment(defaults.GetVerticalAlignment());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_SPACING:
      style->impl.SetWordSpacing(defaults.GetWordSpacing());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_LETTER_SPACING:
      style->impl.SetLetterSpacing(defaults.GetLetterSpacing());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_WORD_BREAK:
      style->impl.SetWordBreak(defaults.GetWordBreak());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_BASELINE_OFFSET:
      style->impl.SetBaselineOffset(defaults.GetBaselineOffset());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SKEW:
      style->impl.SetTextSkew(defaults.GetTextSkew());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_STROKE:
      style->impl.SetTextStrokeStyle(defaults.GetTextStrokeColor(),
                                     defaults.GetTextStrokeWidth());
      break;
    case TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_GRADIENT:
      style->impl.SetForegroundGradient(nullptr);
      break;
    case TEXTRA_STYLE_ATTRIBUTE_TEXT_SHADOWS:
      style->impl.SetTextShadowList({});
      break;
  }
  style->impl.GetImpl().flag_ &= ~(tt::AttrType{1} << internal);
  style->impl.ClearShapeStyle();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_reset(textra_style_t* style) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl = tt::Style();
  style->typeface = {};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_clone(const textra_style_t* source,
                                               textra_style_t** out_style) {
  if (!out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = nullptr;
  if (!source) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = Clone(source).release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_font_descriptor(
    textra_style_t* style, const textra_font_descriptor_t* descriptor) {
  if (!style || !descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetFontDescriptor(descriptor->impl);
  style->typeface = descriptor->typeface;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_font_descriptor(
    const textra_style_t* style, textra_font_descriptor_t** out_descriptor) {
  if (!out_descriptor) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_descriptor = nullptr;
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto value = std::make_unique<textra_font_descriptor>();
  value->impl = style->impl.GetFontDescriptor();
  value->typeface = style->typeface;
  *out_descriptor = value.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_text_stroke(textra_style_t* style,
                                                         uint32_t argb,
                                                         float thickness) {
  if (!style || !Nonnegative(thickness)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetTextStrokeStyle(argb, thickness);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_text_stroke(
    const textra_style_t* style, uint32_t* out_argb, float* out_thickness) {
  if (!style || !out_argb || !out_thickness)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_argb = style->impl.GetTextStrokeColor();
  *out_thickness = style->impl.GetTextStrokeWidth();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_foreground_gradient(
    textra_style_t* style, textra_resolved_gradient_t* gradient) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetForegroundGradient(gradient ? gradient->impl : nullptr);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_get_foreground_gradient(
    const textra_style_t* style, textra_resolved_gradient_t** out_gradient) {
  if (!out_gradient) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_gradient = nullptr;
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto impl = style->impl.GetForegroundGradient();
  if (!impl) return TEXTRA_STATUS_NOT_FOUND;
  auto value = std::make_unique<textra_resolved_gradient>();
  value->impl = std::move(impl);
  *out_gradient = value.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_set_text_shadows(
    textra_style_t* style, const textra_text_shadow_t* shadows,
    uint32_t count) {
  if (!style || (!shadows && count)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  std::vector<tt::TextShadow> result;
  result.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    const auto& v = shadows[i];
    if (!Finite(v.offset_x) || !Finite(v.offset_y) ||
        !std::isfinite(v.blur_radius) || v.blur_radius < 0)
      return TEXTRA_STATUS_INVALID_ARGUMENT;
    result.push_back(
        {tt::TTColor(v.argb), {v.offset_x, v.offset_y}, v.blur_radius});
  }
  style->impl.SetTextShadowList(result);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_style_copy_text_shadows(
    const textra_style_t* style, textra_text_shadow_t* buffer,
    uint32_t capacity, uint32_t* out_count) {
  if (!style || !out_count || (!buffer && capacity))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& values = style->impl.GetTextShadowList();
  *out_count = static_cast<uint32_t>(values.size());
  if (!buffer && !capacity) return TEXTRA_STATUS_OK;
  if (capacity < values.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < values.size(); ++i)
    buffer[i] = {values[i].color_, values[i].offset_[0], values[i].offset_[1],
                 values[i].blur_radius_};
  return TEXTRA_STATUS_OK;
}
