// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/capi/textra_internal.h"
#include "src/textlayout/style/style_impl.h"
#include "src/textlayout/utils/u_8_string.h"

using namespace textra_capi;

textra_status_t TEXTRA_CALL
textra_paragraph_create(const textra_paragraph_options_t* options,
                        textra_paragraph_t** out_paragraph) {
  if (!out_paragraph) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_paragraph = nullptr;
  if (!ValidStruct(options)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto type = TEXTRA_READ(options, type, textra_paragraph_type_t{0});
  if (type > TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE)
    return TEXTRA_STATUS_UNSUPPORTED;
  auto result = std::make_unique<textra_paragraph>();
  result->impl = tt::Paragraph::Create(type == TEXTRA_PARAGRAPH_TYPE_GENERAL
                                           ? tt::ParagraphType::kGeneral
                                           : tt::ParagraphType::kSingleLine);
  if (type == TEXTRA_PARAGRAPH_TYPE_GENERAL) {
    auto style = std::make_unique<textra_paragraph_style>();
    result->style = Ref<textra_paragraph_style>::Adopt(style.release());
  }
  *out_paragraph = result.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_type(
    const textra_paragraph_t* paragraph, textra_paragraph_type_t* out_type) {
  if (!paragraph || !out_type) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_type = paragraph->impl->GetType() == tt::ParagraphType::kGeneral
                  ? TEXTRA_PARAGRAPH_TYPE_GENERAL
                  : TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_set_paragraph_style(
    textra_paragraph_t* paragraph, const textra_paragraph_style_t* style) {
  auto status = Editable(paragraph, true);
  if (status) return status;
  if (!style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto copy = Clone(style);
  paragraph->impl->SetParagraphStyle(&copy->impl);
  paragraph->style = Ref<textra_paragraph_style>::Adopt(copy.release());
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_paragraph_style(
    const textra_paragraph_t* paragraph, textra_paragraph_style_t** out_style) {
  if (!out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = nullptr;
  if (!paragraph) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!paragraph->style) return TEXTRA_STATUS_UNSUPPORTED;
  *out_style = Clone(paragraph->style.get()).release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_add_text_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    const char* utf8, uint32_t byte_count) {
  auto status = Editable(paragraph);
  if (status) return status;
  uint32_t extra = 0;
  if (!::ttoffice::base::CheckValidUTF8String(utf8, byte_count, true, &extra))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!byte_count) return TEXTRA_STATUS_OK;
  if (paragraph->impl->GetType() == tt::ParagraphType::kSingleLine &&
      paragraph->has_text)
    return TEXTRA_STATUS_UNSUPPORTED;
  if (extra > UINT32_MAX - paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  if (style && style->typeface) paragraph->typefaces.push_back(style->typeface);
  paragraph->spans.push_back(
      {paragraph->impl->GetCharCount(),
       paragraph->impl->GetCharCount() + static_cast<uint32_t>(extra),
       style ? style->impl : tt::Style()});
  // The legacy general-paragraph implementation validates a NUL-terminated
  // string internally. Keep that read within a C API-owned allocation.
  const std::string text(utf8, byte_count);
  paragraph->impl->AddTextRun(style ? &style->impl : nullptr, text.c_str(),
                              byte_count);
  paragraph->has_text = true;
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_add_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate, textra_bool_t is_float) {
  auto status = Editable(paragraph, true);
  if (status) return status;
  if (!delegate) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!Bool(is_float)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  status = delegate->impl->Measure();
  if (status) return status;
  paragraph->delegates.push_back(delegate->impl);
  if (style && style->typeface) paragraph->typefaces.push_back(style->typeface);
  paragraph->spans.push_back({paragraph->impl->GetCharCount(),
                              paragraph->impl->GetCharCount() + 1,
                              style ? style->impl : tt::Style()});
  paragraph->impl->AddShapeRun(style ? &style->impl : nullptr, delegate->impl,
                               is_float != 0);
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_add_ghost_shape_run(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    textra_run_delegate_t* delegate) {
  auto status = Editable(paragraph, true);
  if (status) return status;
  if (!delegate) return TEXTRA_STATUS_INVALID_ARGUMENT;

  status = delegate->impl->Measure();
  if (status) return status;
  paragraph->delegates.push_back(delegate->impl);
  if (style && style->typeface) paragraph->typefaces.push_back(style->typeface);
  paragraph->impl->AddGhostShapeRun(style ? &style->impl : nullptr,
                                    delegate->impl);
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_apply_style(
    textra_paragraph_t* paragraph, const textra_style_t* style,
    const textra_text_range_t* range) {
  auto status = Editable(paragraph, true);
  if (status) return status;
  if (!style || !range) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (range->start > range->end || range->end > paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  // The C++ range editor only applies StyleManager (paint) attributes.
  // Reject shape-affecting attributes rather than reporting an ignored edit.
  for (uint32_t id = tt::kLayoutAttrStart; id < tt::kLayoutAttrEnd; ++id)
    if (style->impl.HasAttribute(static_cast<tt::AttributeType>(id)))
      return TEXTRA_STATUS_UNSUPPORTED;
  if (range->start == range->end) return TEXTRA_STATUS_OK;
  std::vector<StyleSpan> spans;
  for (const auto& span : paragraph->spans) {
    if (span.end <= range->start || span.start >= range->end) {
      spans.push_back(span);
      continue;
    }
    if (span.start < range->start)
      spans.push_back({span.start, range->start, span.style});
    auto copy = span.style;
    Overlay(copy, style->impl);
    spans.push_back({std::max(span.start, range->start),
                     std::min(span.end, range->end), copy});
    if (span.end > range->end)
      spans.push_back({range->end, span.end, span.style});
  }
  paragraph->impl->ApplyStyleInRange(style->impl, range->start,
                                     range->end - range->start);
  paragraph->spans = std::move(spans);
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_save_style(textra_paragraph_t* paragraph) {
  auto status = Editable(paragraph, true);
  if (status) return status;

  paragraph->saved_spans.push_back(paragraph->spans);
  paragraph->impl->SaveStyle();
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_restore_style(textra_paragraph_t* paragraph) {
  auto status = Editable(paragraph, true);
  if (status) return status;
  if (paragraph->saved_spans.empty()) return TEXTRA_STATUS_INVALID_STATE;
  auto restored = paragraph->saved_spans.back();
  uint32_t saved_end = 0;
  for (const auto& span : restored) saved_end = std::max(saved_end, span.end);
  // C++ RestoreStyle restores paint ranges, not text content or shaping
  // attributes of runs appended since SaveStyle. Preserve that suffix.
  for (const auto& span : paragraph->spans)
    if (span.end > saved_end) {
      auto layout_only = span.style;
      layout_only.GetImpl().flag_ &=
          (tt::AttrType{1} << tt::kLayoutAttrEnd) - 1;
      restored.push_back(
          {std::max(saved_end, span.start), span.end, layout_only});
    }
  paragraph->impl->RestoreStyle();
  paragraph->spans = std::move(restored);
  paragraph->saved_spans.pop_back();
  Invalidate(paragraph);
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_char_count(
    const textra_paragraph_t* paragraph, uint32_t* out_count) {
  if (!paragraph || !out_count) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_count = paragraph->impl->GetCharCount();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_run_count(
    const textra_paragraph_t* paragraph, uint32_t* out_count) {
  if (!paragraph || !out_count) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_count = paragraph->impl->GetRunCount();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_copy_text(
    const textra_paragraph_t* paragraph, const textra_text_range_t* range,
    char* buffer, uint32_t capacity, uint32_t* out_size) {
  if (!paragraph || !range || !out_size) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (range->start > range->end || range->end > paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  return CopyString(paragraph->impl->GetContentString(
                        range->start, range->end - range->start),
                    buffer, capacity, out_size);
}

textra_status_t TEXTRA_CALL
textra_paragraph_query_style(const textra_paragraph_t* paragraph,
                             uint32_t char_index, textra_style_t** out_style) {
  if (!out_style) return TEXTRA_STATUS_INVALID_ARGUMENT;
  *out_style = nullptr;
  if (!paragraph) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (char_index >= paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  auto result = std::make_unique<textra_style>();
  result->impl = EffectiveStyle(paragraph, char_index);
  auto native = result->impl.GetFontDescriptor().platform_font_;
  if (paragraph->style && paragraph->style->typeface &&
      reinterpret_cast<uintptr_t>(paragraph->style->typeface->native) == native)
    result->typeface = paragraph->style->typeface;
  for (const auto& face : paragraph->typefaces)
    if (reinterpret_cast<uintptr_t>(face->native) == native)
      result->typeface = face;
  *out_style = result.release();
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL
textra_paragraph_get_metrics(const textra_paragraph_t* paragraph,
                             textra_paragraph_metrics_t* out_metrics) {
  if (!paragraph || !ValidStruct(out_metrics))
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!paragraph->valid_layout) return TEXTRA_STATUS_INVALID_STATE;
  TEXTRA_WRITE(out_metrics, width, paragraph->impl->GetWidth());
  TEXTRA_WRITE(out_metrics, height, paragraph->impl->GetHeight());
  TEXTRA_WRITE(out_metrics, baseline, paragraph->impl->GetBaseline());
  TEXTRA_WRITE(out_metrics, ascent, paragraph->impl->GetAscent());
  TEXTRA_WRITE(out_metrics, descent, paragraph->impl->GetDescent());
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_intrinsic_widths(
    const textra_paragraph_t* paragraph, float* out_min, float* out_max) {
  if (!paragraph || !out_min || !out_max) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!paragraph->valid_layout) return TEXTRA_STATUS_INVALID_STATE;
  auto min = paragraph->impl->GetMinIntrinsicWidth();
  auto max = paragraph->impl->GetMaxIntrinsicWidth();
  *out_min = min;
  *out_max = max;
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_word_boundary(
    const textra_paragraph_t* paragraph, uint32_t char_index,
    textra_text_range_t* out_range) {
  if (!paragraph || !out_range) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (char_index >= paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;
  if (!paragraph->valid_layout) return TEXTRA_STATUS_INVALID_STATE;
  auto bounds = paragraph->impl->GetWordBoundary(char_index);
  *out_range = {bounds.first, bounds.second};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_grapheme_boundary(
    const textra_paragraph_t* paragraph, uint32_t char_index,
    textra_text_range_t* out_range) {
  if (!paragraph || !out_range) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (char_index >= paragraph->impl->GetCharCount())
    return TEXTRA_STATUS_OUT_OF_RANGE;

  auto bounds = paragraph->impl->GetGraphemeBoundary(char_index);
  *out_range = {bounds.first, bounds.second};
  return TEXTRA_STATUS_OK;
}

textra_status_t TEXTRA_CALL textra_paragraph_get_resolved_direction(
    const textra_paragraph_t* paragraph,
    textra_write_direction_t* out_direction) {
  if (!paragraph || !out_direction) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (!paragraph->valid_layout) return TEXTRA_STATUS_INVALID_STATE;
  *out_direction = static_cast<textra_write_direction_t>(
      paragraph->impl->GetResolvedWriteDirection());
  return TEXTRA_STATUS_OK;
}
