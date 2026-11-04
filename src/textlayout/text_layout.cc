// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/fontmgr_collection.h>
#include <textra/macro.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include "src/textlayout/internal/boundary_analyst.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/single_line_paragraph_impl.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/text_layout_impl.h"
#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/grapheme_utils.h"
#include "src/textlayout/utils/log_util.h"
#include "src/textlayout/utils/u_8_string.h"

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
  if (para->GetType() == ParagraphType::kSingleLine) {
    LogUtil::E("LayoutEx does not support kSingleLine; use LayoutSingleLine");
    return LayoutResult::kUnsupported;
  }
  shaper_->SetContext(context);
  const auto start_line = page->GetLineCount();
  auto status = TextLayoutImpl::LayoutEx(para, page, context, shaper_.get());
  para->layout_width_ = para->layout_height_ = para->layout_baseline_ = 0;
  if (page->GetLineCount() > start_line) {
    const auto* first = page->GetLine(start_line);
    para->layout_baseline_ = first->GetLineBaseLine() - first->GetLineTop();
    para->layout_height_ =
        page->GetLine(page->GetLineCount() - 1)->GetLineBottom() -
        first->GetLineTop();
    for (auto index = start_line; index < page->GetLineCount(); ++index)
      para->layout_width_ =
          std::max(para->layout_width_, page->GetLine(index)->GetLineWidth());
  }
  return status;
}

bool TextLayout::LayoutSingleLine(Paragraph* paragraph,
                                  TTTextContext& context) const {
  if (!paragraph || paragraph->GetType() != ParagraphType::kSingleLine)
    return false;
  auto* result = static_cast<SingleLineParagraphImpl*>(paragraph);
  if (result->HasUnsupportedOperations()) return false;
  result->Invalidate();
  const auto* text = result->text.data();
  const auto length = static_cast<uint32_t>(result->text.size());
  const auto& style = result->input_style;
  if (!text || !length ||
      style.GetVerticalAlignment() != CharacterVerticalAlignment::kBaseLine ||
      style.GetWordSpacing() != 0 || style.GetBackgroundPainter() ||
      style.GetBackgroundColor() != TTColor::UNDEFINED ||
      style.GetDecorationType() != DecorationType::kNone ||
      style.GetForegroundPainter() || !std::isfinite(style.GetTextSize()) ||
      !std::isfinite(style.GetTextScale()) ||
      !std::isfinite(style.GetLetterSpacing()) ||
      !std::isfinite(style.GetBaselineOffset()) ||
      !std::isfinite(style.GetTextSkew())) {
    return false;
  }
  for (uint32_t index = 0; index < length; ++index) {
    if (text[index] < 0x20 || text[index] == 0x2028 || text[index] == 0x2029 ||
        text[index] > 0x10FFFF ||
        (text[index] >= 0xD800 && text[index] <= 0xDFFF)) {
      return false;
    }
  }
  shaper_->SetContext(context);
  result->style = result->input_style;
  auto& layout_style = result->style;
  shaper_->ProcessShapeStyleTransform(layout_style);
  if (!std::isfinite(layout_style.GetScaledTextSize()) ||
      layout_style.GetTextSize() <= 0 || layout_style.GetTextScale() <= 0) {
    return false;
  }
  auto shaped =
      shaper_->ShapeText(text, length, &layout_style.GetShapeStyle(), false);
  if (!shaped || !shaped->GlyphCount() || shaped->CharCount() != length) {
    return false;
  }
  float max_ascent = 0;
  float max_descent = 0;
  float first_ascent = 0;
  const ITypefaceHelper* previous = nullptr;
  for (uint32_t i = 0; i < shaped->GlyphCount(); ++i) {
    const auto& font = shaped->Font(i);
    if (!font) return false;
    if (font.get() == previous) continue;
    previous = font.get();
    const auto& info = font->GetFontInfo(layout_style.GetTextSize());
    max_ascent = std::min(max_ascent, info.GetAscent());
    max_descent = std::max(max_descent, info.GetDescent());
    if (i == 0) first_ascent = info.GetAscent();
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
  if (!FloatsEqual(skew, 0) && layout_style.GetTextScale() != 1.f) {
    first_ascent =
        first_font->GetFontInfo(layout_style.GetScaledTextSize()).GetAscent();
  }
  result->skew_extra = skew * std::abs(first_ascent);
  result->width =
      shaped->MeasureWidth(0, length, layout_style.GetLetterSpacing()) +
      std::abs(result->skew_extra);
  // Match the ordinary line box; baseline offset moves the ink and expands
  // the box on the corresponding side without losing its layout baseline.
  const auto offset = layout_style.GetBaselineOffset();
  result->baseline = -max_ascent - std::min(0.f, offset);
  const auto bottom = result->baseline + max_descent + std::max(0.f, offset);
  result->descent = bottom - result->baseline;
  if (!std::isfinite(result->width) || !std::isfinite(result->baseline) ||
      !std::isfinite(result->descent) || !std::isfinite(result->skew_extra)) {
    result->Invalidate();
    return false;
  }
  result->shaped = std::move(shaped);
  return true;
}

SingleLineParagraphImpl::SingleLineParagraphImpl()
    : input_style(StyleImpl::DefaultStyle()) {}
SingleLineParagraphImpl::~SingleLineParagraphImpl() = default;
uint32_t SingleLineParagraphImpl::GetCharCount() const {
  return static_cast<uint32_t>(text.size());
}
uint32_t SingleLineParagraphImpl::GetRunCount() const {
  return text.empty() ? 0u : 1u;
}
std::string SingleLineParagraphImpl::GetContentString(uint32_t start,
                                                      uint32_t count) const {
  if (start >= text.size()) return {};
  count = std::min(count, static_cast<uint32_t>(text.size()) - start);
  return base::U32StringToU8(text.data() + start, count);
}
void SingleLineParagraphImpl::Invalidate() const {
  shaped.reset();
  width = baseline = descent = 0;
  boundaries_.reset();
}
void SingleLineParagraphImpl::RejectOperation(const char* operation) const {
  unsupported_ = true;
  Invalidate();
  LogUtil::E("%s is unsupported for kSingleLine; create a kGeneral paragraph",
             operation);
}
ParagraphStyle& SingleLineParagraphImpl::GetParagraphStyle() {
  RejectOperation("GetParagraphStyle");
  if (!rejected_paragraph_style_)
    rejected_paragraph_style_ = std::make_unique<ParagraphStyle>();
  return *rejected_paragraph_style_;
}
void SingleLineParagraphImpl::SetParagraphStyle(const ParagraphStyle*) {
  RejectOperation("SetParagraphStyle");
}
void SingleLineParagraphImpl::AddTextRun(const Style* value,
                                         const char* content, uint32_t length) {
  if (!content || !length || unsupported_) return;
  if (!text.empty()) {
    RejectOperation("AddTextRun after the first run");
    return;
  }
  input_style = value ? value->GetImpl() : StyleImpl::DefaultStyle();
  text = base::U8StringToU32(content, length);
  Invalidate();
}
void SingleLineParagraphImpl::AddShapeRun(const Style*,
                                          std::shared_ptr<RunDelegate>, bool) {
  RejectOperation("AddShapeRun");
}
void SingleLineParagraphImpl::AddGhostShapeRun(const Style*,
                                               std::shared_ptr<RunDelegate>) {
  RejectOperation("AddGhostShapeRun");
}
void SingleLineParagraphImpl::ApplyStyleInRange(const Style&, uint32_t,
                                                uint32_t) const {
  RejectOperation("ApplyStyleInRange");
}
void SingleLineParagraphImpl::SaveStyle() { RejectOperation("SaveStyle"); }
void SingleLineParagraphImpl::RestoreStyle() {
  RejectOperation("RestoreStyle");
}
const BoundaryAnalyst& SingleLineParagraphImpl::GetBoundaries() const {
  if (!boundaries_) {
    StyleManager styles;
    styles.SetParagraphStyle(input_style);
    boundaries_ = std::make_unique<BoundaryAnalyst>(
        text.data(), GetCharCount(), kLineBreakStrategyDefault, styles);
  }
  return *boundaries_;
}
float SingleLineParagraphImpl::GetMaxIntrinsicWidth() const { return width; }
float SingleLineParagraphImpl::GetMinIntrinsicWidth() const {
  if (!shaped) return 0;
  const auto& boundaries = GetBoundaries();
  float minimum = 0;
  uint32_t start = 0;
  const auto count = GetCharCount();
  for (uint32_t pos = 0; pos < count; ++pos) {
    const auto boundary = boundaries.GetBoundaryType(pos);
    if (pos + 1 != count && boundary < BoundaryType::kLineBreakable &&
        !(boundary >= BoundaryType::kGraphme &&
          input_style.GetWordBreak() == WordBreakType::kBreakAll))
      continue;
    auto end = pos + 1;
    while (end > start && base::IsSpaceChar(text[end - 1])) --end;
    if (end > start) {
      minimum = std::max(
          minimum,
          shaped->MeasureWidth(start, end - start, style.GetLetterSpacing()) +
              std::abs(skew_extra));
    }
    start = pos + 1;
  }
  return minimum;
}
std::pair<uint32_t, uint32_t> SingleLineParagraphImpl::GetWordBoundary(
    uint32_t pos) const {
  if (unsupported_ || pos >= GetCharCount()) return {0, 0};
  const auto& boundaries = GetBoundaries();
  return {boundaries.FindPrevBoundary(pos, BoundaryType::kWord),
          boundaries.FindNextBoundary(pos, BoundaryType::kWord)};
}
std::pair<uint32_t, uint32_t> SingleLineParagraphImpl::GetGraphemeBoundary(
    uint32_t pos) const {
  if (unsupported_ || pos >= GetCharCount()) return {0, 0};
  const auto range = FindGraphemeCluster(
      pos, GetCharCount(), [this](uint32_t index) { return text[index]; });
  auto start = range.start;
  auto end = range.end;
  if (shaped) {
    // Match the normal paragraph's refinement for graphemes whose backend
    // produced several advancing glyphs, using the saved shaping result.
    for (auto glyph = shaped->CharToGlyph(start);
         glyph < shaped->CharToGlyph(end); ++glyph) {
      if (!FloatsLarger(shaped->Advances(glyph)[0], 0)) continue;
      const auto char_pos = shaped->GlyphToChar(glyph);
      if (char_pos <= start || char_pos >= end) continue;
      if (char_pos > pos) {
        end = char_pos;
        break;
      }
      start = char_pos;
    }
  }
  return {start, end};
}
void SingleLineParagraphImpl::QueryStyle(uint32_t pos, Style* output) {
  if (!output) return;
  output->Reset();
  if (!unsupported_ && pos < GetCharCount())
    *output = shaped ? style : input_style;
}
WriteDirection SingleLineParagraphImpl::GetResolvedWriteDirection() const {
  return WriteDirection::kLTR;
}

}  // namespace tttext
}  // namespace ttoffice
