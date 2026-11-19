// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_internal.h"
#include "src/textlayout/style_attributes.h"
#include "src/textlayout/utils/u_8_string.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL
textra_paragraph_style_create(textra_paragraph_style_t** out_style) {
  if (!out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = nullptr;
  *out_style = new textra_paragraph_style();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_horizontal_alignment(
    textra_paragraph_style_t* style, textra_horizontal_alignment_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 6) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetHorizontalAlign(
      static_cast<tt::ParagraphHorizontalAlignment>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_horizontal_alignment(
    const textra_paragraph_style_t* style,
    textra_horizontal_alignment_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_horizontal_alignment_t>(
      style->impl.GetHorizontalAlign());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_vertical_alignment(
    textra_paragraph_style_t* style, textra_vertical_alignment_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 2) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetVerticalAlign(
      static_cast<tt::ParagraphVerticalAlignment>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_vertical_alignment(
    const textra_paragraph_style_t* style,
    textra_vertical_alignment_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_vertical_alignment_t>(style->impl.GetVerticalAlign());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_dominant_baseline(
    textra_paragraph_style_t* style, textra_dominant_baseline_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 5) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetDominantBaseline(static_cast<tt::DominantBaseline>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_dominant_baseline(
    const textra_paragraph_style_t* style,
    textra_dominant_baseline_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_dominant_baseline_t>(
      style->impl.GetDominantBaseline());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_write_direction(
    textra_paragraph_style_t* style, textra_write_direction_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 4) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetWriteDirection(static_cast<tt::WriteDirection>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_write_direction(
    const textra_paragraph_style_t* style,
    textra_write_direction_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_write_direction_t>(style->impl.GetWriteDirection());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_spacing_before(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetSpacingBeforeInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_spacing_before(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetSpacingBeforeInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_spacing_after(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetSpacingAfterInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_spacing_after(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetSpacingAfterInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_height_trim(
    textra_paragraph_style_t* style, textra_line_height_trim_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 3) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetLineHeightTrim(static_cast<tt::LineHeightTrim>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_height_trim(
    const textra_paragraph_style_t* style,
    textra_line_height_trim_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_line_height_trim_t>(style->impl.GetLineHeightTrim());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_inline_alignment_mode(
    textra_paragraph_style_t* style, textra_inline_alignment_mode_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 1) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetInlineVerticalAlignmentMode(
      static_cast<tt::InlineVerticalAlignmentMode>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_inline_alignment_mode(
    const textra_paragraph_style_t* style,
    textra_inline_alignment_mode_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_inline_alignment_mode_t>(
      style->impl.GetInlineVerticalAlignmentMode());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_space_before(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetLineSpaceBeforePx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_space_before(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetLineSpaceBeforePx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_space_after(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetLineSpaceAfterPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_space_after(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetLineSpaceAfterPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_max_lines(
    textra_paragraph_style_t* style, uint32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;

  style->impl.SetMaxLines(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_max_lines(
    const textra_paragraph_style_t* style, uint32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      style->impl.GetMaxLines() == UINT32_MAX ? 0u : style->impl.GetMaxLines();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_height_override(
    textra_paragraph_style_t* style, textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetLineHeightOverride(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_height_override(
    const textra_paragraph_style_t* style, textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->impl.LineHeightOverride());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_half_leading(
    textra_paragraph_style_t* style, textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetHalfLeading(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_half_leading(
    const textra_paragraph_style_t* style, textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->impl.HalfLeading());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_text_bounds_enabled(
    textra_paragraph_style_t* style, textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.EnableTextBounds(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_text_bounds_enabled(
    const textra_paragraph_style_t* style, textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->impl.EnableTextBounds());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_overflow_wrap(
    textra_paragraph_style_t* style, textra_overflow_wrap_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 2) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetOverflowWrap(static_cast<tt::OverflowWrap>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_overflow_wrap(
    const textra_paragraph_style_t* style, textra_overflow_wrap_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_overflow_wrap_t>(style->impl.GetOverflowWrap());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_set_break_around_punctuation_allowed(
    textra_paragraph_style_t* style, textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.AllowBreakAroundPunctuation(value);
  style->break_allowed = value != 0;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_get_break_around_punctuation_allowed(
    const textra_paragraph_style_t* style, textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_bool_t>(style->break_allowed);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_punctuation_compress(
    textra_paragraph_style_t* style, textra_punctuation_compress_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (value > 7) return TEXTRA_STATUS_UNSUPPORTED;
  style->impl.SetPunctuationCompressOptions(
      static_cast<tt::PunctuationCompressOption>(value));

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_punctuation_compress(
    const textra_paragraph_style_t* style,
    textra_punctuation_compress_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<textra_punctuation_compress_t>(
      style->impl.GetPunctuationCompressOptions());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_set_ellipsis_uses_default_foreground(
    textra_paragraph_style_t* style, textra_bool_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetEllipsisUsesDefaultForeground(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_get_ellipsis_uses_default_foreground(
    const textra_paragraph_style_t* style, textra_bool_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value =
      static_cast<textra_bool_t>(style->impl.EllipsisUsesDefaultForeground());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_start_indent(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetStartIndentInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_start_indent(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetStartIndentInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_start_indent_in_chars(
    textra_paragraph_style_t* style, int32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;

  style->impl.SetStartIndentInCharCnt(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_start_indent_in_chars(
    const textra_paragraph_style_t* style, int32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<int32_t>(style->impl.GetStartIndentInCharCnt());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_end_indent(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetEndIndentInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_end_indent(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetEndIndentInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_end_indent_in_chars(
    textra_paragraph_style_t* style, int32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;

  style->impl.SetEndIndentInCharCnt(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_end_indent_in_chars(
    const textra_paragraph_style_t* style, int32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<int32_t>(style->impl.GetEndIndentInCharCnt());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_first_line_indent(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetFirstLineIndentInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_first_line_indent(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetFirstLineIndentInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_set_first_line_indent_in_chars(
    textra_paragraph_style_t* style, int32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;

  style->impl.SetFirstLineIndentInCharCnt(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_get_first_line_indent_in_chars(
    const textra_paragraph_style_t* style, int32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<int32_t>(style->impl.GetFirstLineIndentInCharCnt());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_hanging_indent(
    textra_paragraph_style_t* style, float value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Finite(value)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetHangingIndentInPx(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_hanging_indent(
    const textra_paragraph_style_t* style, float* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<float>(style->impl.GetHangingIndentInPx());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_hanging_indent_in_chars(
    textra_paragraph_style_t* style, int32_t value) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;

  style->impl.SetHangingIndentInCharCnt(value);

  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_hanging_indent_in_chars(
    const textra_paragraph_style_t* style, int32_t* out_value) {
  if (!style || !out_value) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_value = static_cast<int32_t>(style->impl.GetHangingIndentInCharCnt());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_clone(const textra_paragraph_style_t* source,
                             textra_paragraph_style_t** out_paragraph_style) {
  if (!out_paragraph_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_paragraph_style = nullptr;
  if (!source) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_paragraph_style = Clone(source).release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_default_style(
    textra_paragraph_style_t* style, const textra_style_t* default_style) {
  if (!style || !default_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetDefaultStyle(default_style->impl);
  style->typeface = default_style->typeface;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_default_style(
    const textra_paragraph_style_t* style, textra_style_t** out_default_style) {
  if (!out_default_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_default_style = nullptr;
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto result = std::make_unique<textra_style>();
  result->impl = style->impl.GetDefaultStyle();
  result->typeface = style->typeface;
  *out_default_style = result.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_height(
    textra_paragraph_style_t* style, float height,
    textra_line_height_rule_t rule) {
  if (!style || !Nonnegative(height)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (rule > TEXTRA_LINE_HEIGHT_RULE_EXACT) return TEXTRA_STATUS_UNSUPPORTED;
  if (rule == TEXTRA_LINE_HEIGHT_RULE_AUTO) {
    if (height <= 0) return TEXTRA_STATUS_INVALID_ARGUMENT;
    auto spacing = style->impl.GetSpacing();
    spacing.line_rule_ = tt::RulerType::kAuto;
    spacing.line_percent_ = height;
    spacing.line_px_ = 0;
    style->impl.SetSpacing(spacing);
  } else {
    style->impl.SetLineHeightInPx(height, static_cast<tt::RulerType>(rule));
  }
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_line_height_factor(
    textra_paragraph_style_t* style, float factor) {
  if (!style || !Finite(factor) || factor <= 0)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto spacing = style->impl.GetSpacing();
  spacing.line_rule_ = tt::RulerType::kAuto;
  spacing.line_percent_ = factor;
  spacing.line_px_ = 0;
  style->impl.SetSpacing(spacing);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_line_height(
    const textra_paragraph_style_t* style, float* out_value,
    textra_line_height_rule_t* out_rule, textra_bool_t* out_is_factor) {
  if (!style || !out_value || !out_rule || !out_is_factor)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto rule = style->impl.GetLineHeightRule();
  *out_rule = static_cast<textra_line_height_rule_t>(rule);
  *out_is_factor = rule == tt::RulerType::kAuto;
  *out_value = *out_is_factor ? style->impl.GetLineHeightInPercent()
                              : style->impl.GetLineHeightInPx();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_ellipsis(
    textra_paragraph_style_t* style, const char* utf8, uint32_t byte_count) {
  if (!style || !::ttoffice::base::CheckValidUTF8String(utf8, byte_count))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto text = ::ttoffice::base::U8StringToU32(utf8 ? utf8 : "", byte_count);
  auto u16 = ::ttoffice::base::U32StringToU16(
      text.data(), static_cast<uint32_t>(text.size()));
  style->impl.SetEllipsis(u16);
  style->ellipsis = {};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_copy_ellipsis(
    const textra_paragraph_style_t* style, char* buffer, uint32_t capacity,
    uint32_t* out_size) {
  if (!style || !out_size) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (style->ellipsis) return TEXTRA_STATUS_NOT_FOUND;
  const auto& text = style->impl.GetEllipsis();
  return CopyString(::ttoffice::base::U32StringToU8(
                        text.data(), static_cast<uint32_t>(text.size())),
                    buffer, capacity, out_size);
}

textra_status_t TEXTRA_CALL textra_paragraph_style_set_ellipsis_delegate(
    textra_paragraph_style_t* style, textra_run_delegate_t* delegate) {
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.SetEllipsis(delegate ? delegate->impl : nullptr);
  style->ellipsis = Ref<textra_run_delegate>(delegate);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_ellipsis_delegate(
    const textra_paragraph_style_t* style,
    textra_run_delegate_t** out_delegate) {
  if (!out_delegate) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_delegate = nullptr;
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!style->ellipsis) return TEXTRA_STATUS_NOT_FOUND;
  Retain(style->ellipsis.get());
  *out_delegate = style->ellipsis.get();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_style_update_punctuation_config(
    textra_paragraph_style_t* style,
    const textra_punctuation_compress_config_t* config) {
  if (!style || !ValidStruct(config)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  tt::PunctuationCompressConfig value;
  value.unicode = TEXTRA_READ(config, codepoint, uint32_t{0});
  auto type = TEXTRA_READ(config, type, textra_punctuation_type_t{0});
  if (type > TEXTRA_PUNCTUATION_TYPE_CENTER) return TEXTRA_STATUS_UNSUPPORTED;
  if (value.unicode > 0x10ffff ||
      (value.unicode >= 0xd800 && value.unicode <= 0xdfff))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  value.type = static_cast<tt::PunctuationType>(type);
  value.default_compress_ratio = TEXTRA_READ(config, default_ratio, 0.0f);
  value.line_edge_compress_ratio = TEXTRA_READ(config, line_edge_ratio, 0.0f);
  value.adjacent_compress_ratio = TEXTRA_READ(config, adjacent_ratio, 0.0f);
  if (!Nonnegative(value.default_compress_ratio) ||
      !Nonnegative(value.line_edge_compress_ratio) ||
      !Nonnegative(value.adjacent_compress_ratio))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  style->impl.UpdatePunctuationCompressConfig(value);
  return TEXTRA_STATUS_OK;
}
