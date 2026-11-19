// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <textra/capi/textra_platform.h>

#include "src/capi/textra_internal.h"

namespace textra_capi {
thread_local Operation* operation = nullptr;

textra_status_t CopyString(const std::string& text, char* buffer,
                           uint32_t capacity, uint32_t* size) {
  if (!size || (!buffer && capacity)) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (text.size() > UINT32_MAX) return TEXTRA_STATUS_OUT_OF_RANGE;
  *size = static_cast<uint32_t>(text.size());
  if (!buffer && !capacity) return TEXTRA_STATUS_OK;
  if (capacity < text.size()) return TEXTRA_STATUS_BUFFER_TOO_SMALL;
  if (!text.empty()) std::memcpy(buffer, text.data(), text.size());
  return TEXTRA_STATUS_OK;
}
std::unique_ptr<textra_style> Clone(const textra_style* source) {
  auto result = std::make_unique<textra_style>();
  result->impl = source->impl;
  result->typeface = source->typeface;
  return result;
}
std::unique_ptr<textra_paragraph_style> Clone(
    const textra_paragraph_style* source) {
  auto result = std::make_unique<textra_paragraph_style>();
  result->impl = source->impl;
  result->typeface = source->typeface;
  result->ellipsis = source->ellipsis;
  result->break_allowed = source->break_allowed;
  return result;
}
textra_status_t Editable(textra_paragraph* p, bool general) {
  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  if (operation || p->locks) return TEXTRA_STATUS_BUSY;
  if (general && p->impl->GetType() != tt::ParagraphType::kGeneral)
    return TEXTRA_STATUS_UNSUPPORTED;
  return TEXTRA_STATUS_OK;
}
void Invalidate(textra_paragraph* p) {
  p->valid_layout = false;
  p->metrics_region = nullptr;
  p->single_engine = {};
  if (p->single_results) {
    --*p->single_results;
    p->single_results.reset();
  }
}
textra_status_t CheckFonts(const textra_paragraph* p,
                           const textra_text_layout* e) {
  const auto backend = e->backend;
  for (const auto& face : p->typefaces)
    if (face->backend != backend) return TEXTRA_STATUS_BACKEND_MISMATCH;
  if (p->style && p->style->typeface && p->style->typeface->backend != backend)
    return TEXTRA_STATUS_BACKEND_MISMATCH;
  return TEXTRA_STATUS_OK;
}
}  // namespace textra_capi

textra_text_context::~textra_text_context() { Unbind(); }
void textra_text_context::Unbind() {
  if (locked && paragraph) --paragraph->locks;
  locked = false;
  paragraph = {};
  engine = {};
}
textra_layout_region::~textra_layout_region() {
  impl.reset();
  for (const auto& dep : dependencies)
    if (dep->paragraph->metrics_region == this)
      dep->paragraph->valid_layout = false;
  dependencies.clear();
  if (adopted && callbacks.destroy) callbacks.destroy(userdata);
}
textra_layout_drawer::~textra_layout_drawer() {
  if (adopted && callbacks.destroy) callbacks.destroy(userdata);
}

namespace textra_capi {
void Overlay(tt::Style& destination, const tt::Style& source) {
  if (source.HasAttribute(tt::kTextSize))
    destination.SetTextSize(source.GetTextSize());
  if (source.HasAttribute(tt::kTextScale))
    destination.SetTextScale(source.GetTextScale());
  if (source.HasAttribute(tt::kForegroundColor))
    destination.SetForegroundColor(source.GetForegroundColor());
  if (source.HasAttribute(tt::kBackgroundColor))
    destination.SetBackgroundColor(source.GetBackgroundColor());
  if (source.HasAttribute(tt::kDecorationColor))
    destination.SetDecorationColor(source.GetDecorationColor());
  if (source.HasAttribute(tt::kDecorationType))
    destination.SetDecorationType(source.GetDecorationType());
  if (source.HasAttribute(tt::kDecorationStyle))
    destination.SetDecorationStyle(source.GetDecorationStyle());
  if (source.HasAttribute(tt::kDecorationThicknessMultiplier))
    destination.SetDecorationThicknessMultiplier(
        source.GetDecorationThicknessMultiplier());
  if (source.HasAttribute(tt::kDecorationElementLength))
    destination.SetDecorationElementLength(source.GetDecorationElementLength());
  if (source.HasAttribute(tt::kDecorationGapLength))
    destination.SetDecorationGapLength(source.GetDecorationGapLength());
  if (source.HasAttribute(tt::kDecorationSideMargin))
    destination.SetDecorationSideMargin(source.GetDecorationSideMargin());
  if (source.HasAttribute(tt::kBold)) destination.SetBold(source.GetBold());
  if (source.HasAttribute(tt::kItalic))
    destination.SetItalic(source.GetItalic());
  if (source.HasAttribute(tt::kVerticalAlignment))
    destination.SetVerticalAlignment(source.GetVerticalAlignment());
  if (source.HasAttribute(tt::kWordSpacing))
    destination.SetWordSpacing(source.GetWordSpacing());
  if (source.HasAttribute(tt::kLetterSpacing))
    destination.SetLetterSpacing(source.GetLetterSpacing());
  if (source.HasAttribute(tt::kWordBreak))
    destination.SetWordBreak(source.GetWordBreak());
  if (source.HasAttribute(tt::kBaselineOffset))
    destination.SetBaselineOffset(source.GetBaselineOffset());
  if (source.HasAttribute(tt::kTextSkew))
    destination.SetTextSkew(source.GetTextSkew());
  if (source.HasAttribute(tt::kFontDescriptor))
    destination.SetFontDescriptor(source.GetFontDescriptor());
  if (source.HasAttribute(tt::kForegroundGradient))
    destination.SetForegroundGradient(source.GetForegroundGradient());
  if (source.HasAttribute(tt::kTextStrokeStyle))
    destination.SetTextStrokeStyle(source.GetTextStrokeColor(),
                                   source.GetTextStrokeWidth());
  if (source.HasAttribute(tt::kTextShadowList))
    destination.SetTextShadowList(source.GetTextShadowList());
}
tt::Style EffectiveStyle(const textra_paragraph* p, uint32_t index) {
  tt::Style style;
  if (p->style) style = p->style->impl.GetDefaultStyle();
  for (const auto& span : p->spans)
    if (span.start <= index && index < span.end) {
      Overlay(style, span.style);
      break;
    }
  return style;
}
}  // namespace textra_capi

textra_paragraph::~textra_paragraph() {
  if (single_results) --*single_results;
}

#if !defined(TEXTRA_ENABLE_ANDROID_BACKEND)
textra_status_t TEXTRA_CALL textra_android_runtime_initialize(void*) {
  return TEXTRA_STATUS_UNSUPPORTED;
}
#endif
