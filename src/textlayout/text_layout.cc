// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>
#include <textra/macro.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style/style_impl.h"
#include "src/textlayout/text_layout_impl.h"
#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/float_comparison.h"

namespace ttoffice {
namespace tttext {

TextLayout::TextLayout(FontmgrCollection* font_collection, ShaperType type)
    : TextLayout(TTShaper::CreateShaper(font_collection, type)) {}

TextLayout::TextLayout(std::unique_ptr<TTShaper> shaper)
    : shaper_(std::move(shaper)) {}

TextLayout::~TextLayout() = default;

bool TextLayout::Preload(ShaperType type) { return TTShaper::Preload(type); }

void TextLayout::ClearShapeCache() {
  ShapeCache::GetInstance().Clear();
  // Shaped results are not the only per-session font state: the self-rendering
  // CoreText shaper keeps a process-wide safe-font cache whose CTFontRef
  // entries pin the whole in-memory font file. Drop it here as well, otherwise
  // clearing the shape cache reclaims almost nothing.
  TTShaper::ClearFontCache();
}

LayoutResult TextLayout::LayoutEx(Paragraph* para, LayoutRegion* page,
                                  TTTextContext& context) const {
  TTASSERT(para);
  TTASSERT(page);
  shaper_->SetContext(context);
  return TextLayoutImpl::LayoutEx(para, page, context, shaper_.get());
}

bool TextLayout::LayoutInline(const char32_t* text, uint32_t length,
                              const Style& style, WriteDirection direction,
                              ICanvasHelper* canvas,
                              InlineLayoutMetrics* metrics,
                              TTTextContext& context) const {
  if (!text || !length || direction != WriteDirection::kLTR || !canvas ||
      !metrics ||
      style.GetVerticalAlignment() != CharacterVerticalAlignment::kBaseLine ||
      style.GetWordSpacing() != 0 || style.GetBackgroundPainter() ||
      style.GetBackgroundColor() != TTColor::UNDEFINED ||
      style.GetDecorationType() != DecorationType::kNone ||
      style.GetForegroundPainter()) {
    return false;
  }
  shaper_->SetContext(context);
  for (uint32_t index = 0; index < length; ++index) {
    if (text[index] < 0x20) return false;
  }

  StyleImpl layout_style(style.GetImpl());
  shaper_->ProcessShapeStyleTransform(layout_style);
  const auto shaped =
      shaper_->ShapeText(text, length, &layout_style.GetShapeStyle(), false);
  if (!shaped || shaped->GlyphCount() == 0 || shaped->CharCount() != length) {
    return false;
  }

  float max_ascent = 0;
  float max_descent = 0;
  for (uint32_t index = 0; index < shaped->GlyphCount(); ++index) {
    const auto& font = shaped->Font(index);
    if (!font) return false;
    const auto& info = font->GetFontInfo(layout_style.GetScaledTextSize());
    max_ascent = std::min(max_ascent, info.GetAscent());
    max_descent = std::max(max_descent, info.GetDescent());
  }

  float skew = layout_style.GetTextSkew();
  bool fake_italic = layout_style.GetItalic();
  const auto& first_font = shaped->Font(0);
  const auto& desired = layout_style.GetFontDescriptor().font_style_;
  if (desired != first_font->FontStyle()) {
    fake_italic =
        desired.GetSlant() == FontStyle::kItalic_Slant &&
        first_font->FontStyle().GetSlant() != FontStyle::kItalic_Slant;
  }
  if (fake_italic) skew += FAKE_ITALIC_SKEW;
  const float skew_extra = skew * std::abs(max_ascent);
  metrics->width =
      shaped->MeasureWidth(0, length, layout_style.GetLetterSpacing()) +
      std::abs(skew_extra);
  metrics->baseline = -max_ascent;
  // Match TextLine::GetLineBottom() - GetLineBaseLine() operation order. The
  // subtraction can intentionally round by one float ULP even when the raw
  // font descent is already available.
  metrics->descent = (metrics->baseline + max_descent) - metrics->baseline;
  if (!std::isfinite(metrics->width) || !std::isfinite(metrics->baseline) ||
      !std::isfinite(metrics->descent)) {
    return false;
  }

  const uint32_t glyph_count = shaped->GlyphCount();
  std::vector<uint16_t> glyphs(glyph_count);
  std::vector<float> pos_x(glyph_count);
  std::vector<float> pos_y(glyph_count);
  float x = FloatsLarger(skew_extra, 0.f) ? skew_extra : 0.f;
  for (uint32_t index = 0; index < glyph_count; ++index) {
    glyphs[index] = shaped->Glyphs(index);
    const float advance = shaped->Advances(index)[0];
    const float* position = shaped->Positions(index);
    pos_y[index] = position[1];
    if (advance < 0) {
      pos_x[index] = x - advance;
    } else {
      pos_x[index] = x;
      x += advance;
    }
    if (FloatsLarger(advance, 0)) x += layout_style.GetLetterSpacing();
  }

  auto painter = canvas->CreatePainter();
  if (!painter) return false;
  const auto& descriptor = layout_style.GetFontDescriptor();
  if (!descriptor.font_family_list_.empty())
    painter->SetFontFamily(descriptor.font_family_list_.front());
  painter->SetTextSize(layout_style.GetScaledTextSize());
  painter->SetFillColor(layout_style.GetForegroundColor());
  painter->SetBold(layout_style.GetBold());
  painter->SetItalic(layout_style.GetItalic());
  painter->SetStrokeColor(layout_style.GetTextStrokeColor());
  painter->SetStrokeWidth(layout_style.GetTextStrokeWidth());
  painter->SetShadowList(layout_style.GetTextShadowList());
  painter->SetTextSkew(layout_style.GetTextSkew());

  uint32_t start = 0;
  while (start < glyph_count) {
    uint32_t end = start + 1;
    while (end < glyph_count && shaped->Font(end) == shaped->Font(start)) ++end;
    const uint32_t next = end;
    const auto& font = shaped->Font(start);
    const bool original_bold = painter->IsBold();
    const bool original_italic = painter->IsItalic();
    if (desired != font->FontStyle()) {
      const auto wanted_weight = desired.GetWeight();
      painter->SetBold(
          wanted_weight >= FontStyle::kSemiBold_Weight &&
          static_cast<int32_t>(wanted_weight) -
                  static_cast<int32_t>(font->FontStyle().GetWeight()) >=
              200);
      painter->SetItalic(desired.GetSlant() == FontStyle::kItalic_Slant &&
                         font->FontStyle().GetSlant() !=
                             FontStyle::kItalic_Slant);
    }
    while (end > start && glyphs[end - 1] == 0) --end;
    if (end > start) {
      canvas->DrawGlyphs(font.get(), end - start, glyphs.data() + start,
                         nullptr, 0, 0, metrics->baseline, pos_x.data() + start,
                         pos_y.data() + start, painter.get());
    }
    painter->SetBold(original_bold);
    painter->SetItalic(original_italic);
    start = next;
  }
  return true;
}
}  // namespace tttext
}  // namespace ttoffice
