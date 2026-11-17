// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/c/textra.h>
#include <textra/fontmgr_collection.h>
#include <textra/gradient.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform_helper.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/capi/textra_capi_internal.h"

namespace {
using tttext::c_api_internal::Handle;
using tttext::c_api_internal::Raw;
using tttext::c_api_internal::Release;
using tttext::c_api_internal::Retain;
using tttext::c_api_internal::Share;
textra_status_t CopyString(const std::string& text, char* buffer,
                           size_t capacity, size_t* required) {
  *required = text.size();
  if (!buffer)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < text.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  if (!text.empty()) std::memcpy(buffer, text.data(), text.size());
  return TEXTRA_STATUS_OK;
}
bool ShaperAvailable(uint32_t type) {
  if (type == TEXTRA_SHAPER_TYPE_SYSTEM) {
#if defined(ENABLE_JAVASHAPER) || defined(ENABLE_CTSHAPER) || \
    defined(ENABLE_OHOS) || defined(ENABLE_LVGL)
    return true;
#else
    return false;
#endif
  }
#if defined(ENABLE_SKSHAPER) || defined(ENABLE_CTSHAPER)
  return true;
#else
  return false;
#endif
}
}  // namespace

namespace {
class CallbackRunDelegate final : public tttext::RunDelegate {
 public:
  CallbackRunDelegate(void* data, textra_destroy_callback_t destroy)
      : data(data), destroy(destroy) {}
  ~CallbackRunDelegate() override {
    if (destroy) destroy(data);
  }
  float GetAscent() const override {
    assert(ascent);
    return ascent(data);
  }
  float GetDescent() const override {
    assert(descent);
    return descent(data);
  }
  float GetAdvance() const override {
    assert(advance);
    return advance(data);
  }
  void Layout() override {
    if (layout) layout(data);
  }
  void Draw(tttext::ICanvasHelper* canvas, float x, float y) override {
    if (draw) draw(data, Handle<textra_canvas_helper_t>(canvas), x, y);
  }
  bool Equals(tttext::RunDelegate* other) override {
    if (!equals) return this == other;
    // The callback may only borrow this temporary handle.
    textra_run_delegate_t borrowed(
        std::shared_ptr<tttext::RunDelegate>(other, [](auto*) {}));
    return equals(data, &borrowed) != 0;
  }
  void* data;
  textra_destroy_callback_t destroy;
  textra_run_metric_callback_t ascent = nullptr, descent = nullptr,
                               advance = nullptr;
  textra_run_layout_callback_t layout = nullptr;
  textra_run_draw_callback_t draw = nullptr;
  textra_run_equals_callback_t equals = nullptr;
};
class CallbackPageListener final : public tttext::LayoutPageListener {
 public:
  CallbackPageListener(void* data, textra_destroy_callback_t destroy)
      : data(data), destroy(destroy) {}
  ~CallbackPageListener() override {
    if (destroy) destroy(data);
  }
  void OnLineLayouted(tttext::LayoutRegion* page, uint32_t index, bool last,
                      float height, tttext::LayoutResult* result) override {
    if (!line) return;
    uint32_t value = static_cast<uint32_t>(*result);
    line(data, Handle<textra_layout_region_t>(page), index, last, height,
         &value);
    *result = static_cast<tttext::LayoutResult>(value);
  }
  void* data;
  textra_destroy_callback_t destroy;
  textra_line_layouted_callback_t line = nullptr;
};
}  // namespace

textra_status_t TEXTRA_CALL textra_style_create(textra_style_t** out) noexcept {
  *out = Handle<textra_style_t>(new tttext::Style());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_style_destroy(textra_style_t* value) noexcept {
  delete Raw<tttext::Style>(value);
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_create(textra_paragraph_style_t** out) noexcept {
  *out = Handle<textra_paragraph_style_t>(new tttext::ParagraphStyle());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_paragraph_style_destroy(textra_paragraph_style_t* value) noexcept {
  delete Raw<tttext::ParagraphStyle>(value);
}

textra_status_t TEXTRA_CALL
textra_tttext_context_create(textra_tttext_context_t** out) noexcept {
  *out = Handle<textra_tttext_context_t>(new tttext::TTTextContext());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_tttext_context_destroy(textra_tttext_context_t* value) noexcept {
  delete Raw<tttext::TTTextContext>(value);
}

textra_status_t TEXTRA_CALL
textra_font_descriptor_create(textra_font_descriptor_t** out) noexcept {
  *out = Handle<textra_font_descriptor_t>(new tttext::FontDescriptor());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_font_descriptor_destroy(textra_font_descriptor_t* value) noexcept {
  delete Raw<tttext::FontDescriptor>(value);
}

textra_status_t TEXTRA_CALL
textra_gradient_create(textra_gradient_t** out) noexcept {
  *out = Handle<textra_gradient_t>(new tttext::Gradient());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_gradient_destroy(textra_gradient_t* value) noexcept {
  delete Raw<tttext::Gradient>(value);
}

textra_status_t TEXTRA_CALL textra_style_copy(const textra_style_t* source,
                                              textra_style_t** out) noexcept {
  *out = Handle<textra_style_t>(new tttext::Style(*Raw<tttext::Style>(source)));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_style_copy(const textra_paragraph_style_t* source,
                            textra_paragraph_style_t** out) noexcept {
  *out = Handle<textra_paragraph_style_t>(
      new tttext::ParagraphStyle(*Raw<tttext::ParagraphStyle>(source)));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_style_reset(textra_style_t* value) noexcept {
  Raw<tttext::Style>(value)->Reset();
}

void TEXTRA_CALL
textra_paragraph_save_style(textra_paragraph_t* value) noexcept {
  Raw<tttext::Paragraph>(value)->SaveStyle();
}

void TEXTRA_CALL
textra_paragraph_restore_style(textra_paragraph_t* value) noexcept {
  Raw<tttext::Paragraph>(value)->RestoreStyle();
}

void TEXTRA_CALL
textra_tttext_context_reset(textra_tttext_context_t* value) noexcept {
  Raw<tttext::TTTextContext>(value)->Reset();
}

float TEXTRA_CALL
textra_style_get_text_size(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetTextSize();
}

void TEXTRA_CALL textra_style_set_text_size(textra_style_t* value,
                                            float input) noexcept {
  Raw<tttext::Style>(value)->SetTextSize(input);
}

float TEXTRA_CALL
textra_style_get_text_scale(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetTextScale();
}

void TEXTRA_CALL textra_style_set_text_scale(textra_style_t* value,
                                             float input) noexcept {
  Raw<tttext::Style>(value)->SetTextScale(input);
}

float TEXTRA_CALL
textra_style_get_letter_spacing(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetLetterSpacing();
}

void TEXTRA_CALL textra_style_set_letter_spacing(textra_style_t* value,
                                                 float input) noexcept {
  Raw<tttext::Style>(value)->SetLetterSpacing(input);
}

float TEXTRA_CALL
textra_style_get_word_spacing(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetWordSpacing();
}

void TEXTRA_CALL textra_style_set_word_spacing(textra_style_t* value,
                                               float input) noexcept {
  Raw<tttext::Style>(value)->SetWordSpacing(input);
}

float TEXTRA_CALL
textra_style_get_baseline_offset(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetBaselineOffset();
}

void TEXTRA_CALL textra_style_set_baseline_offset(textra_style_t* value,
                                                  float input) noexcept {
  Raw<tttext::Style>(value)->SetBaselineOffset(input);
}

float TEXTRA_CALL
textra_style_get_text_skew(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetTextSkew();
}

void TEXTRA_CALL textra_style_set_text_skew(textra_style_t* value,
                                            float input) noexcept {
  Raw<tttext::Style>(value)->SetTextSkew(input);
}

float TEXTRA_CALL textra_style_get_decoration_thickness_multiplier(
    const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetDecorationThicknessMultiplier();
}

void TEXTRA_CALL textra_style_set_decoration_thickness_multiplier(
    textra_style_t* value, float input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationThicknessMultiplier(input);
}

float TEXTRA_CALL textra_style_get_decoration_element_length(
    const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetDecorationElementLength();
}

void TEXTRA_CALL textra_style_set_decoration_element_length(
    textra_style_t* value, float input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationElementLength(input);
}

float TEXTRA_CALL
textra_style_get_decoration_gap_length(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetDecorationGapLength();
}

void TEXTRA_CALL textra_style_set_decoration_gap_length(textra_style_t* value,
                                                        float input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationGapLength(input);
}

float TEXTRA_CALL
textra_style_get_decoration_side_margin(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetDecorationSideMargin();
}

void TEXTRA_CALL textra_style_set_decoration_side_margin(textra_style_t* value,
                                                         float input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationSideMargin(input);
}

uint32_t TEXTRA_CALL
textra_style_get_foreground_color(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetForegroundColor().GetPlainColor();
}

void TEXTRA_CALL textra_style_set_foreground_color(textra_style_t* value,
                                                   uint32_t input) noexcept {
  Raw<tttext::Style>(value)->SetForegroundColor(input);
}

uint32_t TEXTRA_CALL
textra_style_get_background_color(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetBackgroundColor().GetPlainColor();
}

void TEXTRA_CALL textra_style_set_background_color(textra_style_t* value,
                                                   uint32_t input) noexcept {
  Raw<tttext::Style>(value)->SetBackgroundColor(input);
}

uint32_t TEXTRA_CALL
textra_style_get_decoration_color(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetDecorationColor().GetPlainColor();
}

void TEXTRA_CALL textra_style_set_decoration_color(textra_style_t* value,
                                                   uint32_t input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationColor(input);
}

uint32_t TEXTRA_CALL
textra_style_get_bold(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetBold();
}

void TEXTRA_CALL textra_style_set_bold(textra_style_t* value,
                                       uint32_t input) noexcept {
  Raw<tttext::Style>(value)->SetBold(input != 0);
}

uint32_t TEXTRA_CALL
textra_style_get_italic(const textra_style_t* value) noexcept {
  return Raw<tttext::Style>(value)->GetItalic();
}

void TEXTRA_CALL textra_style_set_italic(textra_style_t* value,
                                         uint32_t input) noexcept {
  Raw<tttext::Style>(value)->SetItalic(input != 0);
}

textra_decoration_type_t TEXTRA_CALL
textra_style_get_decoration_type(const textra_style_t* value) noexcept {
  return static_cast<textra_decoration_type_t>(
      Raw<tttext::Style>(value)->GetDecorationType());
}

void TEXTRA_CALL textra_style_set_decoration_type(
    textra_style_t* value, textra_decoration_type_t input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationType(
      static_cast<tttext::DecorationType>(input));
}

textra_line_type_t TEXTRA_CALL
textra_style_get_decoration_style(const textra_style_t* value) noexcept {
  return static_cast<textra_line_type_t>(
      Raw<tttext::Style>(value)->GetDecorationStyle());
}

void TEXTRA_CALL textra_style_set_decoration_style(
    textra_style_t* value, textra_line_type_t input) noexcept {
  Raw<tttext::Style>(value)->SetDecorationStyle(
      static_cast<tttext::LineType>(input));
}

textra_character_vertical_alignment_t TEXTRA_CALL
textra_style_get_vertical_alignment(const textra_style_t* value) noexcept {
  return static_cast<textra_character_vertical_alignment_t>(
      Raw<tttext::Style>(value)->GetVerticalAlignment());
}

void TEXTRA_CALL textra_style_set_vertical_alignment(
    textra_style_t* value,
    textra_character_vertical_alignment_t input) noexcept {
  Raw<tttext::Style>(value)->SetVerticalAlignment(
      static_cast<tttext::CharacterVerticalAlignment>(input));
}

textra_word_break_type_t TEXTRA_CALL
textra_style_get_word_break(const textra_style_t* value) noexcept {
  return static_cast<textra_word_break_type_t>(
      Raw<tttext::Style>(value)->GetWordBreak());
}

void TEXTRA_CALL textra_style_set_word_break(
    textra_style_t* value, textra_word_break_type_t input) noexcept {
  Raw<tttext::Style>(value)->SetWordBreak(
      static_cast<tttext::WordBreakType>(input));
}

void TEXTRA_CALL textra_style_set_text_stroke_style(textra_style_t* style,
                                                    uint32_t color,
                                                    float thickness) noexcept {
  Raw<tttext::Style>(style)->SetTextStrokeStyle(color, thickness);
}

float TEXTRA_CALL
textra_style_get_text_stroke_width(const textra_style_t* style) noexcept {
  return Raw<tttext::Style>(style)->GetTextStrokeWidth();
}

uint32_t TEXTRA_CALL
textra_style_get_text_stroke_color(const textra_style_t* style) noexcept {
  return Raw<tttext::Style>(style)->GetTextStrokeColor().GetPlainColor();
}

float TEXTRA_CALL
textra_style_get_scaled_text_size(const textra_style_t* style) noexcept {
  return Raw<tttext::Style>(style)->GetScaledTextSize();
}

const textra_font_descriptor_t* TEXTRA_CALL
textra_style_get_font_descriptor(const textra_style_t* style) noexcept {
  return Handle<const textra_font_descriptor_t>(
      &Raw<tttext::Style>(style)->GetFontDescriptor());
}

void TEXTRA_CALL textra_style_set_font_descriptor(
    textra_style_t* style,
    const textra_font_descriptor_t* descriptor) noexcept {
  Raw<tttext::Style>(style)->SetFontDescriptor(
      *Raw<tttext::FontDescriptor>(descriptor));
}

uint32_t TEXTRA_CALL textra_style_has_attribute(
    const textra_style_t* style, textra_attribute_type_t type) noexcept {
  return Raw<tttext::Style>(style)->HasAttribute(
      static_cast<tttext::AttributeType>(type));
}

uint32_t TEXTRA_CALL textra_style_has_style_attribute(
    const textra_style_t* style, uint32_t flag) noexcept {
  return Raw<tttext::Style>(style)->HasStyleAttribute(flag);
}

textra_paragraph_horizontal_alignment_t TEXTRA_CALL
textra_paragraph_style_get_horizontal_align(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_paragraph_horizontal_alignment_t>(
      Raw<tttext::ParagraphStyle>(value)->GetHorizontalAlign());
}

void TEXTRA_CALL textra_paragraph_style_set_horizontal_align(
    textra_paragraph_style_t* value,
    textra_paragraph_horizontal_alignment_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetHorizontalAlign(
      static_cast<tttext::ParagraphHorizontalAlignment>(input));
}

textra_paragraph_vertical_alignment_t TEXTRA_CALL
textra_paragraph_style_get_vertical_align(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_paragraph_vertical_alignment_t>(
      Raw<tttext::ParagraphStyle>(value)->GetVerticalAlign());
}

void TEXTRA_CALL textra_paragraph_style_set_vertical_align(
    textra_paragraph_style_t* value,
    textra_paragraph_vertical_alignment_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetVerticalAlign(
      static_cast<tttext::ParagraphVerticalAlignment>(input));
}

textra_dominant_baseline_t TEXTRA_CALL
textra_paragraph_style_get_dominant_baseline(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_dominant_baseline_t>(
      Raw<tttext::ParagraphStyle>(value)->GetDominantBaseline());
}

void TEXTRA_CALL textra_paragraph_style_set_dominant_baseline(
    textra_paragraph_style_t* value,
    textra_dominant_baseline_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetDominantBaseline(
      static_cast<tttext::DominantBaseline>(input));
}

textra_write_direction_t TEXTRA_CALL textra_paragraph_style_get_write_direction(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_write_direction_t>(
      Raw<tttext::ParagraphStyle>(value)->GetWriteDirection());
}

void TEXTRA_CALL textra_paragraph_style_set_write_direction(
    textra_paragraph_style_t* value, textra_write_direction_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetWriteDirection(
      static_cast<tttext::WriteDirection>(input));
}

textra_line_height_trim_t TEXTRA_CALL
textra_paragraph_style_get_line_height_trim(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_line_height_trim_t>(
      Raw<tttext::ParagraphStyle>(value)->GetLineHeightTrim());
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_trim(
    textra_paragraph_style_t* value, textra_line_height_trim_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineHeightTrim(
      static_cast<tttext::LineHeightTrim>(input));
}

textra_inline_vertical_alignment_mode_t TEXTRA_CALL
textra_paragraph_style_get_inline_vertical_alignment_mode(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_inline_vertical_alignment_mode_t>(
      Raw<tttext::ParagraphStyle>(value)->GetInlineVerticalAlignmentMode());
}

void TEXTRA_CALL textra_paragraph_style_set_inline_vertical_alignment_mode(
    textra_paragraph_style_t* value,
    textra_inline_vertical_alignment_mode_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetInlineVerticalAlignmentMode(
      static_cast<tttext::InlineVerticalAlignmentMode>(input));
}

textra_overflow_wrap_t TEXTRA_CALL textra_paragraph_style_get_overflow_wrap(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_overflow_wrap_t>(
      Raw<tttext::ParagraphStyle>(value)->GetOverflowWrap());
}

void TEXTRA_CALL textra_paragraph_style_set_overflow_wrap(
    textra_paragraph_style_t* value, textra_overflow_wrap_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetOverflowWrap(
      static_cast<tttext::OverflowWrap>(input));
}

textra_punctuation_compress_option_t TEXTRA_CALL
textra_paragraph_style_get_punctuation_compress_options(
    const textra_paragraph_style_t* value) noexcept {
  return static_cast<textra_punctuation_compress_option_t>(
      Raw<tttext::ParagraphStyle>(value)->GetPunctuationCompressOptions());
}

void TEXTRA_CALL textra_paragraph_style_set_punctuation_compress_options(
    textra_paragraph_style_t* value,
    textra_punctuation_compress_option_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetPunctuationCompressOptions(
      static_cast<tttext::PunctuationCompressOption>(input));
}

float TEXTRA_CALL textra_paragraph_style_get_start_indent_in_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetStartIndentInPx();
}

void TEXTRA_CALL textra_paragraph_style_set_start_indent_in_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetStartIndentInPx(input);
}

float TEXTRA_CALL textra_paragraph_style_get_end_indent_in_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetEndIndentInPx();
}

void TEXTRA_CALL textra_paragraph_style_set_end_indent_in_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetEndIndentInPx(input);
}

float TEXTRA_CALL textra_paragraph_style_get_first_line_indent_in_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetFirstLineIndentInPx();
}

void TEXTRA_CALL textra_paragraph_style_set_first_line_indent_in_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetFirstLineIndentInPx(input);
}

float TEXTRA_CALL textra_paragraph_style_get_hanging_indent_in_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetHangingIndentInPx();
}

void TEXTRA_CALL textra_paragraph_style_set_hanging_indent_in_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetHangingIndentInPx(input);
}

float TEXTRA_CALL textra_paragraph_style_get_line_height_in_percent(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetLineHeightInPercent();
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_in_percent(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineHeightInPercent(input);
}

float TEXTRA_CALL textra_paragraph_style_get_line_space_before_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetLineSpaceBeforePx();
}

void TEXTRA_CALL textra_paragraph_style_set_line_space_before_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineSpaceBeforePx(input);
}

float TEXTRA_CALL textra_paragraph_style_get_line_space_after_px(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetLineSpaceAfterPx();
}

void TEXTRA_CALL textra_paragraph_style_set_line_space_after_px(
    textra_paragraph_style_t* value, float input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineSpaceAfterPx(input);
}

int32_t TEXTRA_CALL textra_paragraph_style_get_start_indent_in_char_cnt(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetStartIndentInCharCnt();
}

void TEXTRA_CALL textra_paragraph_style_set_start_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetStartIndentInCharCnt(input);
}

int32_t TEXTRA_CALL textra_paragraph_style_get_end_indent_in_char_cnt(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetEndIndentInCharCnt();
}

void TEXTRA_CALL textra_paragraph_style_set_end_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetEndIndentInCharCnt(input);
}

int32_t TEXTRA_CALL textra_paragraph_style_get_first_line_indent_in_char_cnt(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetFirstLineIndentInCharCnt();
}

void TEXTRA_CALL textra_paragraph_style_set_first_line_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetFirstLineIndentInCharCnt(input);
}

int32_t TEXTRA_CALL textra_paragraph_style_get_hanging_indent_in_char_cnt(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetHangingIndentInCharCnt();
}

void TEXTRA_CALL textra_paragraph_style_set_hanging_indent_in_char_cnt(
    textra_paragraph_style_t* value, int32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetHangingIndentInCharCnt(input);
}

uint32_t TEXTRA_CALL textra_paragraph_style_get_max_lines(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->GetMaxLines();
}

void TEXTRA_CALL textra_paragraph_style_set_max_lines(
    textra_paragraph_style_t* value, uint32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetMaxLines(input);
}

uint32_t TEXTRA_CALL textra_paragraph_style_get_line_height_override(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->LineHeightOverride();
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_override(
    textra_paragraph_style_t* value, uint32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineHeightOverride(input != 0);
}

uint32_t TEXTRA_CALL textra_paragraph_style_get_half_leading(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->HalfLeading();
}

void TEXTRA_CALL textra_paragraph_style_set_half_leading(
    textra_paragraph_style_t* value, uint32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetHalfLeading(input != 0);
}

uint32_t TEXTRA_CALL textra_paragraph_style_get_enable_text_bounds(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->EnableTextBounds();
}

void TEXTRA_CALL textra_paragraph_style_set_enable_text_bounds(
    textra_paragraph_style_t* value, uint32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->EnableTextBounds(input != 0);
}

uint32_t TEXTRA_CALL
textra_paragraph_style_get_ellipsis_uses_default_foreground(
    const textra_paragraph_style_t* value) noexcept {
  return Raw<tttext::ParagraphStyle>(value)->EllipsisUsesDefaultForeground();
}

void TEXTRA_CALL textra_paragraph_style_set_ellipsis_uses_default_foreground(
    textra_paragraph_style_t* value, uint32_t input) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetEllipsisUsesDefaultForeground(input !=
                                                                       0);
}

void TEXTRA_CALL textra_paragraph_style_set_default_style(
    textra_paragraph_style_t* style,
    const textra_style_t* default_style) noexcept {
  Raw<tttext::ParagraphStyle>(style)->SetDefaultStyle(
      *Raw<tttext::Style>(default_style));
}

const textra_style_t* TEXTRA_CALL textra_paragraph_style_get_default_style(
    const textra_paragraph_style_t* style) noexcept {
  return Handle<const textra_style_t>(
      &Raw<tttext::ParagraphStyle>(style)->GetDefaultStyle());
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_in_px(
    textra_paragraph_style_t* style, float height,
    textra_ruler_type_t ruler) noexcept {
  Raw<tttext::ParagraphStyle>(style)->SetLineHeightInPx(
      height, static_cast<tttext::RulerType>(ruler));
}

float TEXTRA_CALL textra_paragraph_style_get_line_height_in_px(
    const textra_paragraph_style_t* style) noexcept {
  return Raw<tttext::ParagraphStyle>(style)->GetLineHeightInPx();
}

textra_ruler_type_t TEXTRA_CALL textra_paragraph_style_get_line_height_rule(
    const textra_paragraph_style_t* style) noexcept {
  return static_cast<uint32_t>(
      Raw<tttext::ParagraphStyle>(style)->GetLineHeightRule());
}

void TEXTRA_CALL textra_paragraph_style_set_ellipsis_text(
    textra_paragraph_style_t* style, const char* nul_terminated_utf8) noexcept {
  Raw<tttext::ParagraphStyle>(style)->SetEllipsis(nul_terminated_utf8);
}

void TEXTRA_CALL textra_paragraph_style_set_ellipsis_delegate(
    textra_paragraph_style_t* style, textra_run_delegate_t* delegate) noexcept {
  Raw<tttext::ParagraphStyle>(style)->SetEllipsis(delegate ? delegate->value
                                                           : nullptr);
}

void TEXTRA_CALL textra_paragraph_style_allow_break_around_punctuation(
    textra_paragraph_style_t* style, uint32_t allow) noexcept {
  Raw<tttext::ParagraphStyle>(style)->AllowBreakAroundPunctuation(allow != 0);
}

textra_status_t TEXTRA_CALL textra_paragraph_create(
    textra_paragraph_type_t type, textra_paragraph_t** out) noexcept {
  *out = Handle<textra_paragraph_t>(
      tttext::Paragraph::Create(static_cast<tttext::ParagraphType>(type))
          .release());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_paragraph_destroy(textra_paragraph_t* paragraph) noexcept {
  delete Raw<tttext::Paragraph>(paragraph);
}

textra_paragraph_style_t* TEXTRA_CALL
textra_paragraph_get_paragraph_style(textra_paragraph_t* paragraph) noexcept {
  return Handle<textra_paragraph_style_t>(
      &Raw<tttext::Paragraph>(paragraph)->GetParagraphStyle());
}

void TEXTRA_CALL textra_paragraph_set_paragraph_style(
    textra_paragraph_t* paragraph,
    const textra_paragraph_style_t* style) noexcept {
  Raw<tttext::Paragraph>(paragraph)->SetParagraphStyle(
      Raw<tttext::ParagraphStyle>(style));
}

void TEXTRA_CALL textra_paragraph_add_text_run(textra_paragraph_t* paragraph,
                                               const textra_style_t* style,
                                               const char* utf8,
                                               uint32_t byte_count) noexcept {
  Raw<tttext::Paragraph>(paragraph)->AddTextRun(Raw<tttext::Style>(style), utf8,
                                                byte_count);
}

void TEXTRA_CALL textra_paragraph_add_shape_run(textra_paragraph_t* paragraph,
                                                const textra_style_t* style,
                                                textra_run_delegate_t* delegate,
                                                uint32_t is_float) noexcept {
  Raw<tttext::Paragraph>(paragraph)->AddShapeRun(
      Raw<tttext::Style>(style), delegate->value, is_float != 0);
}

void TEXTRA_CALL textra_paragraph_add_ghost_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate) noexcept {
  Raw<tttext::Paragraph>(paragraph)->AddGhostShapeRun(Raw<tttext::Style>(style),
                                                      delegate->value);
}

void TEXTRA_CALL textra_paragraph_apply_style_in_range(
    const textra_paragraph_t* paragraph, const textra_style_t* style,
    uint32_t start_char, uint32_t char_count) noexcept {
  Raw<tttext::Paragraph>(paragraph)->ApplyStyleInRange(
      *Raw<tttext::Style>(style), start_char, char_count);
}

void TEXTRA_CALL
textra_paragraph_query_style(textra_paragraph_t* paragraph, uint32_t char_index,
                             textra_style_t* out_style) noexcept {
  Raw<tttext::Paragraph>(paragraph)->QueryStyle(char_index,
                                                Raw<tttext::Style>(out_style));
}

textra_paragraph_type_t TEXTRA_CALL
textra_paragraph_get_type(const textra_paragraph_t* paragraph) noexcept {
  return static_cast<uint32_t>(Raw<tttext::Paragraph>(paragraph)->GetType());
}

textra_write_direction_t TEXTRA_CALL
textra_paragraph_get_resolved_write_direction(
    const textra_paragraph_t* paragraph) noexcept {
  return static_cast<uint32_t>(
      Raw<tttext::Paragraph>(paragraph)->GetResolvedWriteDirection());
}

uint32_t TEXTRA_CALL
textra_paragraph_get_char_count(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetCharCount();
}

uint32_t TEXTRA_CALL
textra_paragraph_get_run_count(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetRunCount();
}

float TEXTRA_CALL
textra_paragraph_get_width(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetWidth();
}

float TEXTRA_CALL
textra_paragraph_get_height(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetHeight();
}

float TEXTRA_CALL
textra_paragraph_get_baseline(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetBaseline();
}

float TEXTRA_CALL
textra_paragraph_get_ascent(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetAscent();
}

float TEXTRA_CALL
textra_paragraph_get_descent(const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetDescent();
}

float TEXTRA_CALL textra_paragraph_get_min_intrinsic_width(
    const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetMinIntrinsicWidth();
}

float TEXTRA_CALL textra_paragraph_get_max_intrinsic_width(
    const textra_paragraph_t* paragraph) noexcept {
  return Raw<tttext::Paragraph>(paragraph)->GetMaxIntrinsicWidth();
}

textra_status_t TEXTRA_CALL textra_paragraph_get_content_string(
    const textra_paragraph_t* paragraph, uint32_t start_char,
    uint32_t char_count, char* buffer, size_t capacity,
    size_t* required_bytes) noexcept {
  const auto* p = Raw<tttext::Paragraph>(paragraph);
  if (start_char > p->GetCharCount() ||
      char_count > p->GetCharCount() - start_char)
    return TEXTRA_STATUS_OUT_OF_RANGE;
  return CopyString(p->GetContentString(start_char, char_count), buffer,
                    capacity, required_bytes);
}

void TEXTRA_CALL textra_paragraph_get_word_boundary(
    const textra_paragraph_t* paragraph, uint32_t offset,
    textra_char_range_t* out) noexcept {
  auto range = Raw<tttext::Paragraph>(paragraph)->GetWordBoundary(offset);
  *out = {range.first, range.second};
}

void TEXTRA_CALL textra_paragraph_get_grapheme_boundary(
    const textra_paragraph_t* paragraph, uint32_t offset,
    textra_char_range_t* out) noexcept {
  auto range = Raw<tttext::Paragraph>(paragraph)->GetGraphemeBoundary(offset);
  *out = {range.first, range.second};
}

textra_status_t TEXTRA_CALL
textra_fontmgr_collection_create(textra_font_manager_t* default_manager,
                                 textra_fontmgr_collection_t** out) noexcept {
  *out = Handle<textra_fontmgr_collection_t>(new tttext::FontmgrCollection(
      default_manager ? default_manager->value : nullptr));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_fontmgr_collection_destroy(textra_fontmgr_collection_t* value) noexcept {
  delete Raw<tttext::FontmgrCollection>(value);
}

void TEXTRA_CALL textra_fontmgr_collection_set_default_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) noexcept {
  Raw<tttext::FontmgrCollection>(collection)
      ->SetDefaultFontManager(manager ? manager->value : nullptr);
}

textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_default_font_manager(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Share<textra_font_manager_t>(
      Raw<tttext::FontmgrCollection>(collection)->GetDefaultFontManager());
}

void TEXTRA_CALL textra_fontmgr_collection_set_asset_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) noexcept {
  Raw<tttext::FontmgrCollection>(collection)
      ->SetAssetFontManager(manager ? manager->value : nullptr);
}

textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_asset_font_manager(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Share<textra_font_manager_t>(
      Raw<tttext::FontmgrCollection>(collection)->GetAssetFontManager());
}

void TEXTRA_CALL textra_fontmgr_collection_set_dynamic_font_manager(
    textra_fontmgr_collection_t* collection,
    textra_font_manager_t* manager) noexcept {
  Raw<tttext::FontmgrCollection>(collection)
      ->SetDynamicFontManager(manager ? manager->value : nullptr);
}

textra_font_manager_t* TEXTRA_CALL
textra_fontmgr_collection_get_dynamic_font_manager(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Share<textra_font_manager_t>(
      Raw<tttext::FontmgrCollection>(collection)->GetDynamicFontManager());
}

void TEXTRA_CALL textra_fontmgr_collection_clear_caches(
    textra_fontmgr_collection_t* collection) noexcept {
  Raw<tttext::FontmgrCollection>(collection)->clearCaches();
}

void TEXTRA_CALL textra_fontmgr_collection_enable_font_fallback(
    textra_fontmgr_collection_t* collection) noexcept {
  Raw<tttext::FontmgrCollection>(collection)->enableFontFallback();
}

void TEXTRA_CALL textra_fontmgr_collection_disable_font_fallback(
    textra_fontmgr_collection_t* collection) noexcept {
  Raw<tttext::FontmgrCollection>(collection)->disableFontFallback();
}

uint32_t TEXTRA_CALL textra_fontmgr_collection_font_fallback_enabled(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Raw<tttext::FontmgrCollection>(collection)->fontFallbackEnabled();
}

void TEXTRA_CALL textra_fontmgr_collection_set_default_system_font_family(
    textra_fontmgr_collection_t* collection, const char* family) noexcept {
  Raw<tttext::FontmgrCollection>(collection)
      ->SetDefaultSystemFontFamily(family);
}

textra_status_t TEXTRA_CALL textra_text_layout_create(
    textra_fontmgr_collection_t* collection, textra_shaper_type_t type,
    textra_text_layout_t** out) noexcept {
  *out = nullptr;
  if (!ShaperAvailable(type)) return TEXTRA_STATUS_UNSUPPORTED;
  *out = Handle<textra_text_layout_t>(
      new tttext::TextLayout(Raw<tttext::FontmgrCollection>(collection),
                             static_cast<tttext::ShaperType>(type)));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_text_layout_destroy(textra_text_layout_t* layout) noexcept {
  delete Raw<tttext::TextLayout>(layout);
}

uint32_t TEXTRA_CALL
textra_text_layout_preload(textra_shaper_type_t type) noexcept {
  return tttext::TextLayout::Preload(static_cast<tttext::ShaperType>(type));
}

void TEXTRA_CALL textra_text_layout_clear_shape_cache(void) noexcept {
  tttext::TextLayout::ClearShapeCache();
}

textra_layout_result_t TEXTRA_CALL textra_text_layout_layout_ex(
    const textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_layout_region_t* region, textra_tttext_context_t* context) noexcept {
  return static_cast<uint32_t>(Raw<tttext::TextLayout>(layout)->LayoutEx(
      Raw<tttext::Paragraph>(paragraph), Raw<tttext::LayoutRegion>(region),
      *Raw<tttext::TTTextContext>(context)));
}

uint32_t TEXTRA_CALL textra_text_layout_layout_single_line(
    const textra_text_layout_t* layout, textra_paragraph_t* paragraph,
    textra_tttext_context_t* context) noexcept {
  return Raw<tttext::TextLayout>(layout)->LayoutSingleLine(
      Raw<tttext::Paragraph>(paragraph), *Raw<tttext::TTTextContext>(context));
}

void TEXTRA_CALL textra_tttext_context_enable_feature(
    textra_tttext_context_t* context, textra_feature_option_t feature,
    uint32_t enabled) noexcept {
  Raw<tttext::TTTextContext>(context)->EnableFeature(
      static_cast<tttext::FeatureOption>(feature), enabled != 0);
}

void TEXTRA_CALL textra_tttext_context_set_layout_position(
    const textra_tttext_context_t* context, uint32_t run_index,
    uint32_t char_index_in_run) noexcept {
  Raw<tttext::TTTextContext>(context)->SetLayoutPosition(run_index,
                                                         char_index_in_run);
}

void TEXTRA_CALL textra_tttext_context_get_layout_position(
    const textra_tttext_context_t* context,
    textra_layout_position_t* out) noexcept {
  auto pos = Raw<tttext::TTTextContext>(context)->GetLayoutPosition();
  *out = {pos.first, pos.second};
}

float TEXTRA_CALL textra_tttext_context_get_layout_bottom(
    const textra_tttext_context_t* context) noexcept {
  return Raw<tttext::TTTextContext>(context)->GetLayoutBottom();
}

void TEXTRA_CALL textra_tttext_context_set_layout_bottom(
    textra_tttext_context_t* context, float bottom) noexcept {
  Raw<tttext::TTTextContext>(context)->SetLayoutBottom(bottom);
}

textra_status_t TEXTRA_CALL textra_layout_region_create(
    float width, float height, textra_layout_mode_t width_mode,
    textra_layout_mode_t height_mode, textra_layout_page_listener_t* listener,
    textra_layout_region_t** out) noexcept {
  *out = Handle<textra_layout_region_t>(new tttext::LayoutRegion(
      width, height, static_cast<tttext::LayoutMode>(width_mode),
      static_cast<tttext::LayoutMode>(height_mode),
      Raw<tttext::LayoutPageListener>(listener)));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_layout_region_destroy(textra_layout_region_t* region) noexcept {
  delete Raw<tttext::LayoutRegion>(region);
}

float TEXTRA_CALL textra_layout_region_get_page_width(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->GetPageWidth();
}

float TEXTRA_CALL textra_layout_region_get_page_height(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->GetPageHeight();
}

float TEXTRA_CALL textra_layout_region_get_layouted_width(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->GetLayoutedWidth();
}

float TEXTRA_CALL textra_layout_region_get_layouted_height(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->GetLayoutedHeight();
}

uint32_t TEXTRA_CALL textra_layout_region_get_line_count(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->GetLineCount();
}

textra_layout_mode_t TEXTRA_CALL textra_layout_region_get_width_mode(
    const textra_layout_region_t* region) noexcept {
  return static_cast<uint32_t>(
      Raw<tttext::LayoutRegion>(region)->GetWidthMode());
}

textra_layout_mode_t TEXTRA_CALL textra_layout_region_get_height_mode(
    const textra_layout_region_t* region) noexcept {
  return static_cast<uint32_t>(
      Raw<tttext::LayoutRegion>(region)->GetHeightMode());
}

uint32_t TEXTRA_CALL textra_layout_region_did_exceed_max_lines(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->DidExceedMaxLines();
}

uint32_t TEXTRA_CALL textra_layout_region_is_full_filled(
    const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->IsFullFilled();
}

uint32_t TEXTRA_CALL
textra_layout_region_is_empty(const textra_layout_region_t* region) noexcept {
  return Raw<tttext::LayoutRegion>(region)->IsEmpty();
}

float TEXTRA_CALL textra_layout_region_get_line_pitch(
    const textra_layout_region_t* value) noexcept {
  return Raw<tttext::LayoutRegion>(value)->GetLinePitch();
}

void TEXTRA_CALL textra_layout_region_set_line_pitch(
    textra_layout_region_t* value, float input) noexcept {
  Raw<tttext::LayoutRegion>(value)->SetLinePitch(input);
}

textra_text_line_t* TEXTRA_CALL textra_layout_region_get_line(
    const textra_layout_region_t* region, uint32_t line_index) noexcept {
  return Handle<textra_text_line_t>(
      Raw<tttext::LayoutRegion>(region)->GetLine(line_index));
}

uint32_t TEXTRA_CALL
textra_text_line_get_start_char_pos(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetStartCharPos();
}

uint32_t TEXTRA_CALL
textra_text_line_get_end_char_pos(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetEndCharPos();
}

uint32_t TEXTRA_CALL
textra_text_line_get_char_count(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetCharCount();
}

float TEXTRA_CALL
textra_text_line_get_line_top(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineTop();
}

float TEXTRA_CALL
textra_text_line_get_line_bottom(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineBottom();
}

float TEXTRA_CALL
textra_text_line_get_line_height(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineHeight();
}

float TEXTRA_CALL
textra_text_line_get_line_base_line(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineBaseLine();
}

float TEXTRA_CALL
textra_text_line_get_line_left(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineLeft();
}

float TEXTRA_CALL
textra_text_line_get_line_right(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineRight();
}

float TEXTRA_CALL
textra_text_line_get_line_width(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetLineWidth();
}

float TEXTRA_CALL
textra_text_line_get_max_ascent(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetMaxAscent();
}

float TEXTRA_CALL
textra_text_line_get_max_descent(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetMaxDescent();
}

float TEXTRA_CALL
textra_text_line_get_content_top(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetContentTop();
}

float TEXTRA_CALL
textra_text_line_get_content_baseline(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetContentBaseline();
}

float TEXTRA_CALL
textra_text_line_get_content_bottom(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetContentBottom();
}

float TEXTRA_CALL
textra_text_line_get_content_height(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetContentHeight();
}

float TEXTRA_CALL
textra_text_line_get_start_indent(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetStartIndent();
}

float TEXTRA_CALL
textra_text_line_get_end_indent(const textra_text_line_t* line) noexcept {
  return Raw<tttext::TextLine>(line)->GetEndIndent();
}

textra_paragraph_t* TEXTRA_CALL
textra_text_line_get_paragraph(const textra_text_line_t* line) noexcept {
  return Handle<textra_paragraph_t>(
      Raw<tttext::TextLine>(line)->GetParagraph());
}

uint32_t TEXTRA_CALL textra_text_line_get_char_pos_by_coordinate_x(
    const textra_text_line_t* line, float x) noexcept {
  return Raw<tttext::TextLine>(line)->GetCharPosByCoordinateX(x);
}

void TEXTRA_CALL textra_text_line_get_bounding_rect_by_char_range(
    const textra_text_line_t* line, uint32_t start_char, uint32_t end_char,
    textra_rect_t* out) noexcept {
  float result[4];
  Raw<tttext::TextLine>(line)->GetBoundingRectByCharRange(result, start_char,
                                                          end_char);
  *out = {result[0], result[1], result[2], result[3]};
}

void TEXTRA_CALL textra_text_line_get_tight_bounding_rect_by_char_range(
    const textra_text_line_t* line, uint32_t start_char, uint32_t end_char,
    textra_tight_bounds_t* out) noexcept {
  float result[4];
  Raw<tttext::TextLine>(line)->GetTightBoundingRectByCharRange(
      result, start_char, end_char);
  *out = {result[0], result[1], result[2], result[3]};
}

void TEXTRA_CALL textra_text_line_modify_horizontal_alignment(
    textra_text_line_t* line,
    textra_paragraph_horizontal_alignment_t alignment) noexcept {
  Raw<tttext::TextLine>(line)->ModifyHorizontalAlignment(
      static_cast<tttext::ParagraphHorizontalAlignment>(alignment));
}

textra_status_t TEXTRA_CALL textra_layout_drawer_create(
    textra_canvas_helper_t* canvas, textra_layout_drawer_t** out) noexcept {
  *out = Handle<textra_layout_drawer_t>(
      new tttext::LayoutDrawer(Raw<tttext::ICanvasHelper>(canvas)));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL
textra_layout_drawer_destroy(textra_layout_drawer_t* value) noexcept {
  delete Raw<tttext::LayoutDrawer>(value);
}

void TEXTRA_CALL textra_layout_drawer_draw_layout_page(
    textra_layout_drawer_t* drawer, textra_layout_region_t* region) noexcept {
  Raw<tttext::LayoutDrawer>(drawer)->DrawLayoutPage(
      Raw<tttext::LayoutRegion>(region));
}

void TEXTRA_CALL textra_layout_drawer_draw_text_line(
    textra_layout_drawer_t* drawer, textra_text_line_t* line,
    uint32_t start_char_in_line, uint32_t end_char_in_line) noexcept {
  Raw<tttext::LayoutDrawer>(drawer)->DrawTextLine(
      Raw<tttext::TextLine>(line), start_char_in_line, end_char_in_line);
}

uint32_t TEXTRA_CALL textra_layout_drawer_draw_single_line(
    textra_layout_drawer_t* drawer, const textra_paragraph_t* paragraph,
    float x, float y) noexcept {
  return Raw<tttext::LayoutDrawer>(drawer)->DrawSingleLine(
      *Raw<tttext::Paragraph>(paragraph), x, y);
}

void TEXTRA_CALL
textra_canvas_helper_save(textra_canvas_helper_t* canvas) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->Save();
}

void TEXTRA_CALL
textra_canvas_helper_restore(textra_canvas_helper_t* canvas) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->Restore();
}

void TEXTRA_CALL
textra_canvas_helper_start_paint(textra_canvas_helper_t* canvas) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->StartPaint();
}

void TEXTRA_CALL
textra_canvas_helper_end_paint(textra_canvas_helper_t* canvas) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->EndPaint();
}

void TEXTRA_CALL textra_canvas_helper_translate(textra_canvas_helper_t* canvas,
                                                float x, float y) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->Translate(x, y);
}

void TEXTRA_CALL textra_canvas_helper_scale(textra_canvas_helper_t* canvas,
                                            float x, float y) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->Scale(x, y);
}

void TEXTRA_CALL textra_canvas_helper_rotate(textra_canvas_helper_t* canvas,
                                             float degrees) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->Rotate(degrees);
}

void TEXTRA_CALL textra_canvas_helper_clip_rect(textra_canvas_helper_t* canvas,
                                                float left, float top,
                                                float right, float bottom,
                                                uint32_t antialias) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->ClipRect(left, top, right, bottom,
                                               antialias != 0);
}

void TEXTRA_CALL
textra_canvas_helper_destroy(textra_canvas_helper_t* canvas) noexcept {
  delete Raw<tttext::ICanvasHelper>(canvas);
}

void TEXTRA_CALL
textra_font_manager_retain(textra_font_manager_t* value) noexcept {
  Retain(value);
}

void TEXTRA_CALL
textra_font_manager_release(textra_font_manager_t* value) noexcept {
  Release(value);
}

void TEXTRA_CALL
textra_typeface_helper_retain(textra_typeface_helper_t* value) noexcept {
  Retain(value);
}

void TEXTRA_CALL
textra_typeface_helper_release(textra_typeface_helper_t* value) noexcept {
  Release(value);
}

void TEXTRA_CALL
textra_run_delegate_retain(textra_run_delegate_t* value) noexcept {
  Retain(value);
}

void TEXTRA_CALL
textra_run_delegate_release(textra_run_delegate_t* value) noexcept {
  Release(value);
}

void TEXTRA_CALL
textra_resolved_gradient_retain(textra_resolved_gradient_t* value) noexcept {
  Retain(value);
}

void TEXTRA_CALL
textra_resolved_gradient_release(textra_resolved_gradient_t* value) noexcept {
  Release(value);
}

void TEXTRA_CALL textra_font_descriptor_set_font_style(
    textra_font_descriptor_t* descriptor, textra_font_style_t style) noexcept {
  if (style.width < 1 || style.width > 9 || style.slant > 2) {
    assert(false && "Invalid FontStyle");
    return;
  }
  Raw<tttext::FontDescriptor>(descriptor)->font_style_ =
      tttext::FontStyle(style.weight, style.width, style.slant);
}

void TEXTRA_CALL textra_font_descriptor_get_font_style(
    const textra_font_descriptor_t* descriptor,
    textra_font_style_t* out) noexcept {
  const auto& style = Raw<tttext::FontDescriptor>(descriptor)->font_style_;
  *out = {style.GetWeight(), style.GetWidth(), style.GetSlant()};
}

void TEXTRA_CALL textra_font_descriptor_set_platform_font(
    textra_font_descriptor_t* descriptor, uint64_t platform_font) noexcept {
  Raw<tttext::FontDescriptor>(descriptor)->platform_font_ = platform_font;
}

uint64_t TEXTRA_CALL textra_font_descriptor_get_platform_font(
    const textra_font_descriptor_t* descriptor) noexcept {
  return Raw<tttext::FontDescriptor>(descriptor)->platform_font_;
}

void TEXTRA_CALL textra_font_descriptor_set_font_families(
    textra_font_descriptor_t* descriptor, const char* const* families,
    const uint8_t* literals, size_t count) noexcept {
  auto* d = Raw<tttext::FontDescriptor>(descriptor);
  std::vector<std::string> names;
  names.reserve(count);
  for (size_t i = 0; i < count; ++i) names.emplace_back(families[i]);
  d->font_family_list_ = std::move(names);
  d->font_family_literals_.clear();
  if (literals && count)
    d->font_family_literals_.assign(literals, literals + count);
}

size_t TEXTRA_CALL textra_font_descriptor_get_font_family_count(
    const textra_font_descriptor_t* descriptor) noexcept {
  return Raw<tttext::FontDescriptor>(descriptor)->font_family_list_.size();
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_family(
    const textra_font_descriptor_t* descriptor, size_t index, char* buffer,
    size_t capacity, size_t* required_bytes) noexcept {
  const auto& families =
      Raw<tttext::FontDescriptor>(descriptor)->font_family_list_;
  if (index >= families.size()) return TEXTRA_STATUS_OUT_OF_RANGE;
  return CopyString(families[index], buffer, capacity, required_bytes);
}

textra_status_t TEXTRA_CALL textra_font_descriptor_get_font_family_literal(
    const textra_font_descriptor_t* descriptor, size_t index,
    uint32_t* has_literal, uint32_t* literal) noexcept {
  const auto* d = Raw<tttext::FontDescriptor>(descriptor);
  if (index >= d->font_family_list_.size()) return TEXTRA_STATUS_OUT_OF_RANGE;
  *has_literal = !d->font_family_literals_.empty();
  *literal = *has_literal ? d->font_family_literals_[index] != 0 : 0;
  return TEXTRA_STATUS_OK;
}

int32_t TEXTRA_CALL textra_font_manager_count_families(
    const textra_font_manager_t* manager) noexcept {
  return manager->value->countFamilies();
}

textra_typeface_helper_t* TEXTRA_CALL textra_font_manager_match_family_style(
    textra_font_manager_t* manager, const char* family,
    textra_font_style_t style) noexcept {
  return Share<textra_typeface_helper_t>(manager->value->matchFamilyStyle(
      family, tttext::FontStyle(style.weight, style.width, style.slant)));
}

textra_typeface_helper_t* TEXTRA_CALL textra_font_manager_make_from_file(
    textra_font_manager_t* manager, const char* path,
    int32_t ttc_index) noexcept {
  return Share<textra_typeface_helper_t>(
      manager->value->makeFromFile(path, ttc_index));
}

textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_create_typeface_from_platform_font(
    textra_font_manager_t* manager, const void* platform_font) noexcept {
  return Share<textra_typeface_helper_t>(
      manager->value->createTypefaceFromPlatformFont(platform_font));
}

void* TEXTRA_CALL textra_font_manager_get_platform_font_from_typeface(
    textra_font_manager_t* manager,
    textra_typeface_helper_t* typeface) noexcept {
  return manager->value->getPlatformFontFromTypeface(typeface->value);
}

textra_status_t TEXTRA_CALL
textra_run_delegate_create(void* user_data, textra_destroy_callback_t destroy,
                           textra_run_delegate_t** out) noexcept {
  *out = Share<textra_run_delegate_t>(
      std::make_shared<CallbackRunDelegate>(user_data, destroy));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_run_delegate_set_ascent_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->ascent = callback;
}

void TEXTRA_CALL textra_run_delegate_set_descent_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->descent = callback;
}

void TEXTRA_CALL textra_run_delegate_set_advance_callback(
    textra_run_delegate_t* delegate,
    textra_run_metric_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->advance = callback;
}

void TEXTRA_CALL textra_run_delegate_set_layout_callback(
    textra_run_delegate_t* delegate,
    textra_run_layout_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->layout = callback;
}

void TEXTRA_CALL textra_run_delegate_set_draw_callback(
    textra_run_delegate_t* delegate,
    textra_run_draw_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->draw = callback;
}

void* TEXTRA_CALL textra_run_delegate_get_user_data(
    const textra_run_delegate_t* delegate) noexcept {
  return static_cast<CallbackRunDelegate*>(delegate->value.get())->data;
}

void TEXTRA_CALL textra_run_delegate_set_equals_callback(
    textra_run_delegate_t* delegate,
    textra_run_equals_callback_t callback) noexcept {
  static_cast<CallbackRunDelegate*>(delegate->value.get())->equals = callback;
}

float TEXTRA_CALL
textra_run_delegate_get_ascent(const textra_run_delegate_t* delegate) noexcept {
  return delegate->value->GetAscent();
}

float TEXTRA_CALL textra_run_delegate_get_descent(
    const textra_run_delegate_t* delegate) noexcept {
  return delegate->value->GetDescent();
}

float TEXTRA_CALL textra_run_delegate_get_advance(
    const textra_run_delegate_t* delegate) noexcept {
  return delegate->value->GetAdvance();
}

float TEXTRA_CALL textra_run_delegate_get_x_offset(
    const textra_run_delegate_t* delegate) noexcept {
  return delegate->value->GetXOffset();
}

float TEXTRA_CALL textra_run_delegate_get_y_offset(
    const textra_run_delegate_t* delegate) noexcept {
  return delegate->value->GetYOffset();
}

textra_status_t TEXTRA_CALL textra_layout_page_listener_create(
    void* user_data, textra_destroy_callback_t destroy,
    textra_layout_page_listener_t** out) noexcept {
  *out = Handle<textra_layout_page_listener_t>(
      static_cast<tttext::LayoutPageListener*>(
          new CallbackPageListener(user_data, destroy)));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_layout_page_listener_destroy(
    textra_layout_page_listener_t* listener) noexcept {
  delete Raw<tttext::LayoutPageListener>(listener);
}

void TEXTRA_CALL textra_layout_page_listener_set_on_line_layouted_callback(
    textra_layout_page_listener_t* listener,
    textra_line_layouted_callback_t callback) noexcept {
  static_cast<CallbackPageListener*>(Raw<tttext::LayoutPageListener>(listener))
      ->line = callback;
}

textra_status_t TEXTRA_CALL textra_platform_helper_create_font_manager(
    textra_platform_type_t type, textra_font_manager_t** out) noexcept {
  *out = nullptr;
  auto value = tttext::PlatformHelper::CreateFontManager(
      static_cast<tttext::PlatformType>(type));
  if (!value) return TEXTRA_STATUS_UNSUPPORTED;
  *out = Share<textra_font_manager_t>(std::move(value));
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_platform_helper_create_canvas_helper(
    textra_platform_type_t type, void* handler,
    textra_canvas_helper_t** out) noexcept {
  *out = nullptr;
  auto value = tttext::PlatformHelper::CreateCanvasHelper(
      static_cast<tttext::PlatformType>(type), handler);
  if (!value) return TEXTRA_STATUS_UNSUPPORTED;
  *out = Handle<textra_canvas_helper_t>(value.release());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_style_set_text_shadow_list(
    textra_style_t* style, const textra_text_shadow_t* shadows,
    size_t count) noexcept {
  std::vector<tttext::TextShadow> values;
  values.reserve(count);
  for (size_t i = 0; i < count; ++i)
    values.push_back({tttext::TTColor(shadows[i].color),
                      {shadows[i].offset_x, shadows[i].offset_y},
                      shadows[i].blur_radius});
  Raw<tttext::Style>(style)->SetTextShadowList(values);
}

textra_status_t TEXTRA_CALL textra_style_get_text_shadow_list(
    const textra_style_t* style, textra_text_shadow_t* shadows, size_t capacity,
    size_t* required_count) noexcept {
  const auto& values = Raw<tttext::Style>(style)->GetTextShadowList();
  *required_count = values.size();
  if (!shadows)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < values.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < values.size(); ++i)
    shadows[i] = {values[i].color_.GetPlainColor(), values[i].offset_[0],
                  values[i].offset_[1], values[i].blur_radius_};
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_gradient_set_type(
    textra_gradient_t* gradient, textra_gradient_type_t type) noexcept {
  Raw<tttext::Gradient>(gradient)->type =
      static_cast<tttext::GradientType>(type);
}

textra_gradient_type_t TEXTRA_CALL
textra_gradient_get_type(const textra_gradient_t* gradient) noexcept {
  return static_cast<uint32_t>(Raw<tttext::Gradient>(gradient)->type);
}

void TEXTRA_CALL textra_gradient_set_linear(
    textra_gradient_t* gradient, float angle,
    textra_linear_gradient_direction_t direction) noexcept {
  Raw<tttext::Gradient>(gradient)->linear = {
      angle, static_cast<tttext::LinearGradientDirection>(direction)};
}

void TEXTRA_CALL textra_gradient_set_radial(
    textra_gradient_t* gradient, textra_gradient_length_t center_x,
    textra_gradient_length_t center_y, textra_radial_gradient_size_t size,
    uint32_t ellipse, textra_gradient_length_t radius_x,
    textra_gradient_length_t radius_y) noexcept {
  Raw<tttext::Gradient>(gradient)->radial = {
      {center_x.value, static_cast<tttext::GradientUnit>(center_x.unit)},
      {center_y.value, static_cast<tttext::GradientUnit>(center_y.unit)},
      static_cast<tttext::RadialGradientSize>(size),
      ellipse != 0,
      {radius_x.value, static_cast<tttext::GradientUnit>(radius_x.unit)},
      {radius_y.value, static_cast<tttext::GradientUnit>(radius_y.unit)}};
}

void TEXTRA_CALL textra_gradient_set_conic(textra_gradient_t* gradient,
                                           textra_gradient_length_t center_x,
                                           textra_gradient_length_t center_y,
                                           float start_angle) noexcept {
  Raw<tttext::Gradient>(gradient)->conic = {
      {center_x.value, static_cast<tttext::GradientUnit>(center_x.unit)},
      {center_y.value, static_cast<tttext::GradientUnit>(center_y.unit)},
      start_angle};
}

void TEXTRA_CALL textra_gradient_set_stops(textra_gradient_t* gradient,
                                           const textra_gradient_stop_t* stops,
                                           size_t count) noexcept {
  std::vector<tttext::GradientStop> values;
  values.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    values.push_back({tttext::TTColor(stops[i].color), stops[i].position,
                      static_cast<tttext::GradientUnit>(stops[i].unit)});
  }
  Raw<tttext::Gradient>(gradient)->stops = std::move(values);
}

textra_status_t TEXTRA_CALL
textra_gradient_resolve(const textra_gradient_t* gradient, textra_rect_t bounds,
                        textra_resolved_gradient_t** out) noexcept {
  *out = Share<textra_resolved_gradient_t>(
      std::make_shared<const tttext::ResolvedGradient>(
          Raw<tttext::Gradient>(gradient)->Resolve(
              {bounds.left, bounds.top, bounds.width, bounds.height})));
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_style_set_foreground_gradient(
    textra_style_t* style, textra_resolved_gradient_t* gradient) noexcept {
  Raw<tttext::Style>(style)->SetForegroundGradient(gradient ? gradient->value
                                                            : nullptr);
}

textra_resolved_gradient_t* TEXTRA_CALL
textra_style_get_foreground_gradient(const textra_style_t* style) noexcept {
  return Share<textra_resolved_gradient_t>(
      Raw<tttext::Style>(style)->GetForegroundGradient());
}

void TEXTRA_CALL textra_resolved_gradient_get_bounds(
    const textra_resolved_gradient_t* gradient, textra_rect_t* out) noexcept {
  const auto& b = gradient->value->bounds;
  *out = {b.left, b.top, b.width, b.height};
}

void TEXTRA_CALL textra_gradient_get_linear(
    const textra_gradient_t* gradient, float* angle,
    textra_linear_gradient_direction_t* direction) noexcept {
  const auto& v = Raw<tttext::Gradient>(gradient)->linear;
  *angle = v.angle;
  *direction = static_cast<uint32_t>(v.direction);
}

void TEXTRA_CALL textra_gradient_get_conic(const textra_gradient_t* gradient,
                                           textra_gradient_length_t* center_x,
                                           textra_gradient_length_t* center_y,
                                           float* start_angle) noexcept {
  const auto& v = Raw<tttext::Gradient>(gradient)->conic;
  *center_x = {v.center_x.value, static_cast<uint32_t>(v.center_x.unit)};
  *center_y = {v.center_y.value, static_cast<uint32_t>(v.center_y.unit)};
  *start_angle = v.start_angle;
}

void TEXTRA_CALL textra_gradient_get_radial(
    const textra_gradient_t* gradient, textra_gradient_length_t* center_x,
    textra_gradient_length_t* center_y, textra_radial_gradient_size_t* size,
    uint32_t* ellipse, textra_gradient_length_t* radius_x,
    textra_gradient_length_t* radius_y) noexcept {
  const auto& v = Raw<tttext::Gradient>(gradient)->radial;
  *center_x = {v.center_x.value, static_cast<uint32_t>(v.center_x.unit)};
  *center_y = {v.center_y.value, static_cast<uint32_t>(v.center_y.unit)};
  *size = static_cast<uint32_t>(v.size);
  *ellipse = v.ellipse;
  *radius_x = {v.radius_x.value, static_cast<uint32_t>(v.radius_x.unit)};
  *radius_y = {v.radius_y.value, static_cast<uint32_t>(v.radius_y.unit)};
}

textra_status_t TEXTRA_CALL textra_gradient_get_stops(
    const textra_gradient_t* gradient, textra_gradient_stop_t* stops,
    size_t capacity, size_t* required_count) noexcept {
  const auto& v = Raw<tttext::Gradient>(gradient)->stops;
  *required_count = v.size();
  if (!stops)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < v.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < v.size(); ++i)
    stops[i] = {v[i].color.GetPlainColor(), v[i].position,
                static_cast<uint32_t>(v[i].unit)};
  return TEXTRA_STATUS_OK;
}

textra_gradient_type_t TEXTRA_CALL textra_resolved_gradient_get_type(
    const textra_resolved_gradient_t* gradient) noexcept {
  return static_cast<uint32_t>(gradient->value->type);
}

void TEXTRA_CALL
textra_resolved_gradient_get_start(const textra_resolved_gradient_t* gradient,
                                   textra_gradient_point_t* out) noexcept {
  *out = {gradient->value->start.x, gradient->value->start.y};
}

void TEXTRA_CALL
textra_resolved_gradient_get_end(const textra_resolved_gradient_t* gradient,
                                 textra_gradient_point_t* out) noexcept {
  *out = {gradient->value->end.x, gradient->value->end.y};
}

void TEXTRA_CALL
textra_resolved_gradient_get_center(const textra_resolved_gradient_t* gradient,
                                    textra_gradient_point_t* out) noexcept {
  *out = {gradient->value->center.x, gradient->value->center.y};
}

float TEXTRA_CALL textra_resolved_gradient_get_radius_x(
    const textra_resolved_gradient_t* gradient) noexcept {
  return gradient->value->radius_x;
}

float TEXTRA_CALL textra_resolved_gradient_get_radius_y(
    const textra_resolved_gradient_t* gradient) noexcept {
  return gradient->value->radius_y;
}

float TEXTRA_CALL textra_resolved_gradient_get_start_angle(
    const textra_resolved_gradient_t* gradient) noexcept {
  return gradient->value->start_angle;
}

textra_status_t TEXTRA_CALL textra_resolved_gradient_get_stops(
    const textra_resolved_gradient_t* gradient, uint32_t* colors,
    float* positions, size_t capacity, size_t* required_count) noexcept {
  const auto& v = *gradient->value;
  *required_count = v.colors.size();
  if (!colors && !positions && capacity == 0) return TEXTRA_STATUS_OK;
  if (!colors || !positions) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < v.colors.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < v.colors.size(); ++i) {
    colors[i] = v.colors[i];
    positions[i] = v.positions[i];
  }
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_canvas_helper_create_painter(
    textra_canvas_helper_t* canvas, textra_painter_t** out) noexcept {
  auto p = Raw<tttext::ICanvasHelper>(canvas)->CreatePainter();
  *out = Handle<textra_painter_t>(p.release());
  return *out ? TEXTRA_STATUS_OK : TEXTRA_STATUS_BACKEND_ERROR;
}

void TEXTRA_CALL textra_canvas_helper_skew(textra_canvas_helper_t* value,
                                           float sx, float sy) noexcept {
  Raw<tttext::ICanvasHelper>(value)->Skew(sx, sy);
}

void TEXTRA_CALL
textra_canvas_helper_clear(textra_canvas_helper_t* value) noexcept {
  Raw<tttext::ICanvasHelper>(value)->Clear();
}

void TEXTRA_CALL textra_canvas_helper_clear_rect(textra_canvas_helper_t* value,
                                                 float left, float top,
                                                 float right,
                                                 float bottom) noexcept {
  Raw<tttext::ICanvasHelper>(value)->ClearRect(left, top, right, bottom);
}

void TEXTRA_CALL textra_canvas_helper_fill_rect(textra_canvas_helper_t* value,
                                                float left, float top,
                                                float right, float bottom,
                                                uint32_t color) noexcept {
  Raw<tttext::ICanvasHelper>(value)->FillRect(left, top, right, bottom, color);
}

void TEXTRA_CALL textra_canvas_helper_draw_color(textra_canvas_helper_t* value,
                                                 uint32_t color) noexcept {
  Raw<tttext::ICanvasHelper>(value)->DrawColor(color);
}

void TEXTRA_CALL textra_canvas_helper_draw_line(
    textra_canvas_helper_t* canvas, float x1, float y1, float x2, float y2,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawLine(x1, y1, x2, y2,
                                               Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_rect(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawRect(left, top, right, bottom,
                                               Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_oval(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawOval(left, top, right, bottom,
                                               Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_circle(
    textra_canvas_helper_t* canvas, float x, float y, float radius,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawCircle(x, y, radius,
                                                 Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_arc(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, float start_angle, float sweep_angle, uint32_t use_center,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawArc(
      left, top, right, bottom, start_angle, sweep_angle, use_center != 0,
      Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_arc_to(
    textra_canvas_helper_t* canvas, float start_x, float start_y, float mid_x,
    float mid_y, float end_x, float end_y, float radius,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawArcTo(start_x, start_y, mid_x, mid_y,
                                                end_x, end_y, radius,
                                                Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_round_rect(
    textra_canvas_helper_t* canvas, float left, float top, float right,
    float bottom, float radius, textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawRoundRect(
      left, top, right, bottom, radius, Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_text(
    textra_canvas_helper_t* canvas, const textra_typeface_helper_t* font,
    const char* text, uint32_t text_bytes, float x, float y,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawText(
      font->value.get(), text, text_bytes, x, y, Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_canvas_helper_draw_glyphs(
    textra_canvas_helper_t* canvas, const textra_typeface_helper_t* font,
    uint32_t glyph_count, const uint16_t* glyphs, const char* text,
    uint32_t text_bytes, float origin_x, float origin_y, float* x, float* y,
    textra_painter_t* painter) noexcept {
  Raw<tttext::ICanvasHelper>(canvas)->DrawGlyphs(
      font->value.get(), glyph_count, glyphs, text, text_bytes, origin_x,
      origin_y, x, y, Raw<tttext::Painter>(painter));
}

textra_status_t TEXTRA_CALL
textra_painter_create(textra_painter_t** out) noexcept {
  *out = Handle<textra_painter_t>(new tttext::Painter());
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_painter_destroy(textra_painter_t* value) noexcept {
  delete Raw<tttext::Painter>(value);
}

uint32_t TEXTRA_CALL
textra_painter_get_fill_color(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetFillColor().GetPlainColor();
}

void TEXTRA_CALL textra_painter_set_fill_color(textra_painter_t* value,
                                               uint32_t input) noexcept {
  Raw<tttext::Painter>(value)->SetFillColor(input);
}

uint32_t TEXTRA_CALL
textra_painter_get_stroke_color(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetStrokeColor().GetPlainColor();
}

void TEXTRA_CALL textra_painter_set_stroke_color(textra_painter_t* value,
                                                 uint32_t input) noexcept {
  Raw<tttext::Painter>(value)->SetStrokeColor(input);
}

float TEXTRA_CALL
textra_painter_get_stroke_width(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetStrokeWidth();
}

void TEXTRA_CALL textra_painter_set_stroke_width(textra_painter_t* value,
                                                 float input) noexcept {
  Raw<tttext::Painter>(value)->SetStrokeWidth(input);
}

float TEXTRA_CALL
textra_painter_get_stroke_miter(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetStrokeMiter();
}

void TEXTRA_CALL textra_painter_set_stroke_miter(textra_painter_t* value,
                                                 float input) noexcept {
  Raw<tttext::Painter>(value)->SetStrokeMiter(input);
}

float TEXTRA_CALL
textra_painter_get_text_size(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetTextSize();
}

void TEXTRA_CALL textra_painter_set_text_size(textra_painter_t* value,
                                              float input) noexcept {
  Raw<tttext::Painter>(value)->SetTextSize(input);
}

float TEXTRA_CALL
textra_painter_get_text_skew(const textra_painter_t* value) noexcept {
  return Raw<tttext::Painter>(value)->GetTextSkew();
}

void TEXTRA_CALL textra_painter_set_text_skew(textra_painter_t* value,
                                              float input) noexcept {
  Raw<tttext::Painter>(value)->SetTextSkew(input);
}

uint32_t TEXTRA_CALL
textra_painter_is_bold(const textra_painter_t* painter) noexcept {
  return Raw<tttext::Painter>(painter)->IsBold();
}

void TEXTRA_CALL textra_painter_set_bold(textra_painter_t* painter,
                                         uint32_t value) noexcept {
  Raw<tttext::Painter>(painter)->SetBold(value != 0);
}

uint32_t TEXTRA_CALL
textra_painter_is_italic(const textra_painter_t* painter) noexcept {
  return Raw<tttext::Painter>(painter)->IsItalic();
}

void TEXTRA_CALL textra_painter_set_italic(textra_painter_t* painter,
                                           uint32_t value) noexcept {
  Raw<tttext::Painter>(painter)->SetItalic(value != 0);
}

uint32_t TEXTRA_CALL
textra_painter_is_under_line(const textra_painter_t* painter) noexcept {
  return Raw<tttext::Painter>(painter)->IsUnderLine();
}

void TEXTRA_CALL textra_painter_set_under_line(textra_painter_t* painter,
                                               uint32_t value) noexcept {
  Raw<tttext::Painter>(painter)->SetUnderLine(value != 0);
}

textra_cap_t TEXTRA_CALL
textra_painter_get_cap(const textra_painter_t* value) noexcept {
  return static_cast<textra_cap_t>(Raw<tttext::Painter>(value)->GetCap());
}

void TEXTRA_CALL textra_painter_set_cap(textra_painter_t* value,
                                        textra_cap_t input) noexcept {
  Raw<tttext::Painter>(value)->SetCap(static_cast<tttext::Cap>(input));
}

textra_join_t TEXTRA_CALL
textra_painter_get_join(const textra_painter_t* value) noexcept {
  return static_cast<textra_join_t>(Raw<tttext::Painter>(value)->GetJoin());
}

void TEXTRA_CALL textra_painter_set_join(textra_painter_t* value,
                                         textra_join_t input) noexcept {
  Raw<tttext::Painter>(value)->SetJoin(static_cast<tttext::Join>(input));
}

void TEXTRA_CALL textra_painter_set_font_family(textra_painter_t* painter,
                                                const char* family) noexcept {
  Raw<tttext::Painter>(painter)->SetFontFamily(family);
}

textra_status_t TEXTRA_CALL textra_painter_get_font_family(
    const textra_painter_t* painter, char* buffer, size_t capacity,
    size_t* required_bytes) noexcept {
  return CopyString(Raw<tttext::Painter>(painter)->GetFontFamily(), buffer,
                    capacity, required_bytes);
}

void TEXTRA_CALL textra_painter_set_fill_gradient(
    textra_painter_t* painter, textra_resolved_gradient_t* gradient) noexcept {
  Raw<tttext::Painter>(painter)->SetFillGradient(gradient ? gradient->value
                                                          : nullptr);
}

textra_resolved_gradient_t* TEXTRA_CALL
textra_painter_get_fill_gradient(const textra_painter_t* painter) noexcept {
  return Share<textra_resolved_gradient_t>(
      Raw<tttext::Painter>(painter)->GetFillGradient());
}

void TEXTRA_CALL textra_painter_set_shadow_list(
    textra_painter_t* painter, const textra_text_shadow_t* shadows,
    size_t count) noexcept {
  std::vector<tttext::TextShadow> values;
  values.reserve(count);
  for (size_t i = 0; i < count; ++i)
    values.push_back({tttext::TTColor(shadows[i].color),
                      {shadows[i].offset_x, shadows[i].offset_y},
                      shadows[i].blur_radius});
  Raw<tttext::Painter>(painter)->SetShadowList(values);
}

textra_status_t TEXTRA_CALL textra_painter_get_shadow_list(
    const textra_painter_t* painter, textra_text_shadow_t* shadows,
    size_t capacity, size_t* required_count) noexcept {
  const auto& v = Raw<tttext::Painter>(painter)->GetShadowList();
  *required_count = v.size();
  if (!shadows)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < v.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < v.size(); ++i)
    shadows[i] = {v[i].color_.GetPlainColor(), v[i].offset_[0], v[i].offset_[1],
                  v[i].blur_radius_};
  return TEXTRA_STATUS_OK;
}

const textra_style_t* TEXTRA_CALL textra_style_default_style(void) noexcept {
  return Handle<const textra_style_t>(&tttext::Style::DefaultStyle());
}

void TEXTRA_CALL textra_style_assign(textra_style_t* value,
                                     const textra_style_t* source) noexcept {
  *Raw<tttext::Style>(value) = *Raw<tttext::Style>(source);
}

void TEXTRA_CALL
textra_paragraph_style_assign(textra_paragraph_style_t* value,
                              const textra_paragraph_style_t* source) noexcept {
  *Raw<tttext::ParagraphStyle>(value) = *Raw<tttext::ParagraphStyle>(source);
}

void TEXTRA_CALL
textra_font_descriptor_assign(textra_font_descriptor_t* value,
                              const textra_font_descriptor_t* source) noexcept {
  *Raw<tttext::FontDescriptor>(value) = *Raw<tttext::FontDescriptor>(source);
}

textra_painter_t* TEXTRA_CALL
textra_style_get_foreground_painter(const textra_style_t* style) noexcept {
  return Handle<textra_painter_t>(
      Raw<tttext::Style>(style)->GetForegroundPainter());
}

void TEXTRA_CALL textra_style_set_foreground_painter(
    textra_style_t* style, textra_painter_t* painter) noexcept {
  Raw<tttext::Style>(style)->SetForegroundPainter(
      Raw<tttext::Painter>(painter));
}

textra_painter_t* TEXTRA_CALL
textra_style_get_background_painter(const textra_style_t* style) noexcept {
  return Handle<textra_painter_t>(
      Raw<tttext::Style>(style)->GetBackgroundPainter());
}

void TEXTRA_CALL textra_style_set_background_painter(
    textra_style_t* style, textra_painter_t* painter) noexcept {
  Raw<tttext::Style>(style)->SetBackgroundPainter(
      Raw<tttext::Painter>(painter));
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_in_px_exact(
    textra_paragraph_style_t* value, float px) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineHeightInPxExact(px);
}

void TEXTRA_CALL textra_paragraph_style_set_line_height_in_px_at_least(
    textra_paragraph_style_t* value, float px) noexcept {
  Raw<tttext::ParagraphStyle>(value)->SetLineHeightInPxAtLeast(px);
}

textra_status_t TEXTRA_CALL textra_paragraph_style_get_ellipsis(
    const textra_paragraph_style_t* style, uint32_t* codepoints,
    size_t capacity, size_t* required_count) noexcept {
  const auto& v = Raw<tttext::ParagraphStyle>(style)->GetEllipsis();
  *required_count = v.size();
  if (!codepoints)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < v.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < v.size(); ++i) codepoints[i] = v[i];
  return TEXTRA_STATUS_OK;
}

void TEXTRA_CALL textra_paragraph_style_set_ellipsis_utf16(
    textra_paragraph_style_t* style, const uint16_t* text,
    size_t count) noexcept {
  std::u16string v;
  v.reserve(count);
  for (size_t i = 0; i < count; ++i)
    v.push_back(static_cast<char16_t>(text[i]));
  Raw<tttext::ParagraphStyle>(style)->SetEllipsis(v);
}

textra_run_delegate_t* TEXTRA_CALL textra_paragraph_style_get_ellipsis_delegate(
    const textra_paragraph_style_t* style) noexcept {
  return Share<textra_run_delegate_t>(
      Raw<tttext::ParagraphStyle>(style)->GetEllipsisDelegate());
}

void TEXTRA_CALL textra_paragraph_style_update_punctuation_compress_config(
    textra_paragraph_style_t* style, uint32_t unicode,
    textra_punctuation_type_t type, float default_ratio, float line_edge_ratio,
    float adjacent_ratio) noexcept {
  Raw<tttext::ParagraphStyle>(style)->UpdatePunctuationCompressConfig(
      {static_cast<char32_t>(unicode),
       static_cast<tttext::PunctuationType>(type), default_ratio,
       line_edge_ratio, adjacent_ratio});
}

void TEXTRA_CALL textra_layout_region_update_layouted_size(
    textra_layout_region_t* region, textra_text_line_t* line,
    const textra_tttext_context_t* context) noexcept {
  Raw<tttext::LayoutRegion>(region)->UpdateLayoutedSize(
      Raw<tttext::TextLine>(line), *Raw<tttext::TTTextContext>(context));
}

uint32_t TEXTRA_CALL
textra_text_line_is_empty(const textra_text_line_t* value) noexcept {
  return Raw<tttext::TextLine>(value)->IsEmpty();
}

uint32_t TEXTRA_CALL textra_text_line_is_last_line_of_paragraph(
    const textra_text_line_t* value) noexcept {
  return Raw<tttext::TextLine>(value)->IsLastLineOfParagraph();
}

void TEXTRA_CALL textra_text_line_strip_by_ellipsis(
    textra_text_line_t* line, const uint32_t* nul_terminated_utf32) noexcept {
  std::u32string v;
  if (nul_terminated_utf32) {
    for (auto* p = nul_terminated_utf32; *p; ++p)
      v.push_back(static_cast<char32_t>(*p));
  }
  Raw<tttext::TextLine>(line)->StripByEllipsis(nul_terminated_utf32 ? v.c_str()
                                                                    : nullptr);
}

void TEXTRA_CALL textra_text_line_get_char_bounding_rect(
    const textra_text_line_t* line, uint32_t char_pos,
    textra_rect_t* out) noexcept {
  float r[4];
  Raw<tttext::TextLine>(line)->GetCharBoundingRect(r, char_pos);
  *out = {r[0], r[1], r[2], r[3]};
}

void TEXTRA_CALL textra_text_line_get_bounding_rect_for_line(
    const textra_text_line_t* line, textra_rect_t* out) noexcept {
  float r[4];
  Raw<tttext::TextLine>(line)->GetBoundingRectForLine(r);
  *out = {r[0], r[1], r[2], r[3]};
}

void TEXTRA_CALL
textra_run_delegate_layout(textra_run_delegate_t* delegate) noexcept {
  delegate->value->Layout();
}

void TEXTRA_CALL textra_run_delegate_draw(textra_run_delegate_t* delegate,
                                          textra_canvas_helper_t* canvas,
                                          float x, float y) noexcept {
  delegate->value->Draw(Raw<tttext::ICanvasHelper>(canvas), x, y);
}

textra_typeface_helper_t* TEXTRA_CALL textra_font_manager_legacy_make_typeface(
    textra_font_manager_t* manager, const char* family,
    textra_font_style_t style) noexcept {
  return Share<textra_typeface_helper_t>(manager->value->legacyMakeTypeface(
      family, tttext::FontStyle(style.weight, style.width, style.slant)));
}

textra_typeface_helper_t* TEXTRA_CALL
textra_font_manager_match_family_style_character(textra_font_manager_t* manager,
                                                 const char* family,
                                                 textra_font_style_t style,
                                                 const char** bcp47,
                                                 int32_t bcp47_count,
                                                 uint32_t character) noexcept {
  return Share<textra_typeface_helper_t>(
      manager->value->matchFamilyStyleCharacter(
          family, tttext::FontStyle(style.weight, style.width, style.slant),
          bcp47, bcp47_count, character));
}

size_t TEXTRA_CALL textra_fontmgr_collection_get_font_managers_count(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Raw<tttext::FontmgrCollection>(collection)->getFontManagersCount();
}

textra_status_t TEXTRA_CALL textra_fontmgr_collection_find_typefaces(
    const textra_fontmgr_collection_t* collection,
    const textra_font_descriptor_t* descriptor,
    textra_typeface_helper_t** typefaces, size_t capacity,
    size_t* required_count) noexcept {
  auto v = Raw<tttext::FontmgrCollection>(collection)
               ->findTypefaces(*Raw<tttext::FontDescriptor>(descriptor));
  *required_count = v.size();
  if (!typefaces)
    return capacity == 0 ? TEXTRA_STATUS_OK : TEXTRA_STATUS_INVALID_ARGUMENT;
  if (capacity < v.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  for (size_t i = 0; i < v.size(); ++i)
    typefaces[i] = Share<textra_typeface_helper_t>(std::move(v[i]));
  return TEXTRA_STATUS_OK;
}

textra_typeface_helper_t* TEXTRA_CALL
textra_fontmgr_collection_default_fallback(
    const textra_fontmgr_collection_t* collection) noexcept {
  return Share<textra_typeface_helper_t>(
      Raw<tttext::FontmgrCollection>(collection)->defaultFallback());
}

textra_typeface_helper_t* TEXTRA_CALL
textra_fontmgr_collection_default_fallback_character(
    const textra_fontmgr_collection_t* collection, uint32_t character,
    textra_font_style_t style, const char* locale) noexcept {
  return Share<textra_typeface_helper_t>(
      Raw<tttext::FontmgrCollection>(collection)
          ->defaultFallback(
              character,
              tttext::FontStyle(style.weight, style.width, style.slant),
              locale));
}

textra_status_t TEXTRA_CALL
textra_font_descriptor_copy(const textra_font_descriptor_t* source,
                            textra_font_descriptor_t** out) noexcept {
  *out = Handle<textra_font_descriptor_t>(
      new tttext::FontDescriptor(*Raw<tttext::FontDescriptor>(source)));
  return TEXTRA_STATUS_OK;
}

uint32_t TEXTRA_CALL
textra_font_descriptor_equals(const textra_font_descriptor_t* value,
                              const textra_font_descriptor_t* other) noexcept {
  return *Raw<tttext::FontDescriptor>(value) ==
         *Raw<tttext::FontDescriptor>(other);
}

float TEXTRA_CALL textra_typeface_helper_get_horizontal_advance(
    const textra_typeface_helper_t* typeface, uint16_t glyph,
    float size) noexcept {
  return typeface->value->GetHorizontalAdvance(glyph, size);
}

void TEXTRA_CALL textra_typeface_helper_get_horizontal_advances(
    const textra_typeface_helper_t* typeface, uint16_t* glyphs, uint32_t count,
    float* widths, float size) noexcept {
  typeface->value->GetHorizontalAdvances(glyphs, count, widths, size);
}

uint16_t TEXTRA_CALL textra_typeface_helper_unichar_to_glyph(
    const textra_typeface_helper_t* typeface, uint32_t codepoint,
    uint32_t variation_selector) noexcept {
  return typeface->value->UnicharToGlyph(codepoint, variation_selector);
}

void TEXTRA_CALL textra_typeface_helper_unichars_to_glyphs(
    const textra_typeface_helper_t* typeface, const uint32_t* codepoints,
    uint32_t count, uint16_t* glyphs) noexcept {
  typeface->value->UnicharsToGlyphs(codepoints, count, glyphs);
}

uint32_t TEXTRA_CALL textra_typeface_helper_get_unique_id(
    const textra_typeface_helper_t* typeface) noexcept {
  return typeface->value->GetUniqueId();
}

void TEXTRA_CALL textra_typeface_helper_get_width_bound(
    const textra_typeface_helper_t* typeface, uint16_t glyph, float size,
    textra_rect_t* out) noexcept {
  float r[4];
  typeface->value->GetWidthBound(r, glyph, size);
  *out = {r[0], r[1], r[2], r[3]};
}

void TEXTRA_CALL textra_typeface_helper_get_width_bounds(
    textra_typeface_helper_t* typeface, uint16_t* glyphs, uint32_t count,
    float size, textra_bounds_t* out) noexcept {
  float r[4];
  typeface->value->GetWidthBounds(r, glyphs, count, size);
  *out = {r[0], r[1], r[2], r[3]};
}

const textra_font_info_t* TEXTRA_CALL textra_typeface_helper_get_font_info(
    textra_typeface_helper_t* typeface, float size) noexcept {
  return Handle<const textra_font_info_t>(&typeface->value->GetFontInfo(size));
}

const textra_font_em_height_t* TEXTRA_CALL
textra_typeface_helper_get_font_em_height(textra_typeface_helper_t* typeface,
                                          float size) noexcept {
  return Handle<const textra_font_em_height_t>(
      &typeface->value->GetFontEmHeight(size));
}

void TEXTRA_CALL
textra_typeface_helper_get_font_style(const textra_typeface_helper_t* typeface,
                                      textra_font_style_t* out) noexcept {
  const auto& s = typeface->value->FontStyle();
  *out = {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

float TEXTRA_CALL
textra_font_info_get_top(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetTop();
}

float TEXTRA_CALL
textra_font_info_get_ascent(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetAscent();
}

float TEXTRA_CALL
textra_font_info_get_descent(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetDescent();
}

float TEXTRA_CALL
textra_font_info_get_bottom(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetBottom();
}

float TEXTRA_CALL
textra_font_info_get_leading(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetLeading();
}

float TEXTRA_CALL
textra_font_info_get_font_size(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetFontSize();
}

float TEXTRA_CALL
textra_font_info_get_height(const textra_font_info_t* value) noexcept {
  return Raw<tttext::FontInfo>(value)->GetHeight();
}

float TEXTRA_CALL textra_font_em_height_get_ascent(
    const textra_font_em_height_t* value) noexcept {
  return Raw<tttext::FontEmHeight>(value)->GetAscent();
}

float TEXTRA_CALL textra_font_em_height_get_descent(
    const textra_font_em_height_t* value) noexcept {
  return Raw<tttext::FontEmHeight>(value)->GetDescent();
}

textra_font_style_t TEXTRA_CALL textra_font_style_normal(void) noexcept {
  auto s = tttext::FontStyle::Normal();
  return {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

textra_font_style_t TEXTRA_CALL textra_font_style_bold(void) noexcept {
  auto s = tttext::FontStyle::Bold();
  return {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

textra_font_style_t TEXTRA_CALL textra_font_style_italic(void) noexcept {
  auto s = tttext::FontStyle::Italic();
  return {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

textra_font_style_t TEXTRA_CALL textra_font_style_bold_italic(void) noexcept {
  auto s = tttext::FontStyle::BoldItalic();
  return {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

