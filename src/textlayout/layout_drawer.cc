// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_drawer.h>

#include <algorithm>
#include <array>

// #include "block_region.h"
#include <textra/i_canvas_helper.h>
#include <textra/layout_region.h>

#include "src/textlayout/internal/line_range.h"
#include "src/textlayout/internal/run_range.h"
#include "src/textlayout/layout_measurer.h"
#include "src/textlayout/run/ghost_run.h"
#include "src/textlayout/run/object_run.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/text_line_impl.h"
#include "src/textlayout/utils/tt_rectf.h"

#if defined(TTTEXT_DEBUG)
// #define DRAW_LINE_BOX
#endif
namespace {
constexpr float kUnderlineDefaultDashLength = 2.5f;
constexpr float kUnderlineDefaultGapLength = 1.5f;
constexpr float kUnderlineDefaultStrokeWidth = 1.f;
constexpr float kUnderlineDefaultSideMargin = 2.5f;
}  // namespace
namespace ttoffice {
namespace tttext {
LayoutDrawer::LayoutDrawer(ICanvasHelper* canvas)
    : canvas_(canvas), listener_(LayoutDrawerListener::GetInstance()) {}
LayoutDrawer::~LayoutDrawer() = default;
void LayoutDrawer::DrawLayoutPage(LayoutRegion* layout_page) {
  if (!canvas_) return;
  for (auto& line : layout_page->line_lst_) {
    DrawTextLine(line.get(), 0, line->GetCharCount());
  }
}
void LayoutDrawer::SetListener(LayoutDrawerListener* listener) {
  listener_ = listener;
}

void LayoutDrawer::DrawTextLine(TextLine* i_line, uint32_t char_start_in_line,
                                uint32_t char_end_in_line) {
  auto* line = TTDYNAMIC_CAST<TextLineImpl*>(i_line);

  auto line_start_in_para = line->GetStartCharPos();
  auto char_start_in_para = char_start_in_line + line_start_in_para;
  auto char_end_in_para = char_end_in_line + line_start_in_para;
  DrawLineBackground(line, char_start_in_para, char_end_in_para);
  bool not_ghost_content_drawed = false;
  for (auto& drawer_piece : line->drawer_list_) {
    TTASSERT(!drawer_piece->GetRun()->IsControlRun());
    if (drawer_piece->GetRun()->IsGhostRun()) {
      if (not_ghost_content_drawed || char_start_in_line == 0) {
        DrawDrawerPiece(line, drawer_piece.get(), 0,
                        drawer_piece->GetCharCount());
      }
    } else {
      auto piece_start = drawer_piece->GetStartCharPosInParagraph();
      auto piece_end = drawer_piece->GetEndCharPosInParagraph();
      auto draw_start = std::max(piece_start, char_start_in_para);
      auto draw_end = std::min(piece_end, char_end_in_para);
      if (draw_start >= draw_end) break;
      DrawDrawerPiece(line, drawer_piece.get(), draw_start - piece_start,
                      draw_end - piece_start);
      not_ghost_content_drawed = true;
    }
  }
  DrawLineDecoration(line, char_start_in_para, char_end_in_para);
#if DRAW_AUXILIARY_LINE
  line->DumpLineInfo();
  RunPr run_style;
  Painter painter(&run_style, 0);
  painter.SetStyle(Style::kStrokeAndFill);
  painter.SetColor(0xFF00FF00);
  canvas_->drawLine(static_cast<float>(range.GetXMin()),
                    static_cast<float>(line->GetLineBottom()),
                    static_cast<float>(range.GetXMax()),
                    static_cast<float>(line->GetLineBottom()), &painter);
#endif
}
/**
 * TODO
 * It should implement a special process if there are multiple line ranges in a
 * line.
 * @param line
 */
void LayoutDrawer::DrawLineBackground(TextLine* i_line,
                                      uint32_t char_start_in_para,
                                      uint32_t char_end_in_para) {
  auto* line = TTDYNAMIC_CAST<TextLineImpl*>(i_line);
  auto& paragraph = *line->GetParagraph();
  auto& style_manager = paragraph.style_manager_;
  auto start_char = char_start_in_para;
  StyleRange range;
  std::array<float, 4> rect{};
  while (range.GetRange().GetEnd() <= char_end_in_para) {
    style_manager->GetStyleRange(
        &range, start_char,
        Style::BackgroundColorFlag() | Style::BackgroundPainterFlag());
    if (range.GetStyle().GetBackgroundColor().GetAlpha() != 0 ||
        range.GetStyle().GetBackgroundPainter() != nullptr) {
      auto end_char = std::min(range.GetRange().GetEnd(), char_end_in_para);
      line->GetBoundingRectByCharRange(rect.data(), start_char, end_char);
      auto& style = range.GetStyle();
      auto default_painter = canvas_->CreatePainter();
      auto painter = style.GetBackgroundPainter();
      if (painter == nullptr) {
        painter = default_painter.get();
        painter->SetFillStyle(FillStyle::kFill);
        painter->SetColor(style.GetBackgroundColor().GetPlainColor());
      }
      canvas_->DrawRect(rect[0], line->GetLineTop(), rect[0] + rect[2],
                        line->GetLineBottom(), painter);
    }
    start_char = range.GetRange().GetEnd();
  }
}
void LayoutDrawer::DrawLineDecoration(TextLine* i_line,
                                      uint32_t char_start_in_para,
                                      uint32_t char_end_in_para) const {
  auto* line = TTDYNAMIC_CAST<TextLineImpl*>(i_line);
  auto& paragraph = *line->GetParagraph();
  auto& style_manager = paragraph.style_manager_;
  auto start_char = char_start_in_para;
  StyleRange range;
  std::array<float, 4> rect{};
  float line_y = 0;
  while (range.GetRange().GetEnd() <= char_end_in_para) {
    style_manager->GetStyleRange(&range, start_char, Style::DecorationFlag());
    auto end_char = std::min(range.GetRange().GetEnd(), char_end_in_para);
    auto& decorate_style = range.GetStyle();
    auto& default_text_style = line->GetParagraph()->GetDefaultStyle();
    if (decorate_style.GetDecorationType() != DecorationType::kNone) {
      line->GetBoundingRectByCharRange(rect.data(), start_char, end_char);
      switch (decorate_style.GetDecorationType()) {
        case DecorationType::kNone:
          TTASSERT(false);
          break;
        case DecorationType::kUnderLine: {
          // TODO(hfuttyh): Improve underline positioning by:
          // 1. Reading font's suggested position
          // 2. Allowing client to specify an offset
          constexpr float kDefaultUnderlineOffset = -2.f;
          line_y = line->GetLineBottom() + kDefaultUnderlineOffset;
          break;
        }
        case DecorationType::kOverline:
          break;
        case DecorationType::kLineThrough:
          line_y = (line->GetLineTop() + line->GetLineBottom()) / 2;
          break;
      }
      auto painter = canvas_->CreatePainter();
      painter->SetFillStyle(FillStyle::kStrokeAndFill);
      painter->SetColor(decorate_style.GetDecorationColor());
      auto stroke_width = painter->GetStrokeWidth();
      switch (decorate_style.GetDecorationStyle()) {
        case LineType::kSolid: {
          painter->SetStrokeWidth(
              default_text_style.GetTextSize() *
              default_text_style.GetTextScale() / 14.f *
              decorate_style.GetDecorationThicknessMultiplier());
          canvas_->DrawLine(rect[0], line_y, rect[0] + rect[2], line_y,
                            painter.get());
          break;
        }
        case LineType::kDashed: {
          painter->SetStrokeWidth(
              kUnderlineDefaultStrokeWidth *
              decorate_style.GetDecorationThicknessMultiplier());
          const float dash_len = kUnderlineDefaultDashLength;
          const float side_margin = std::clamp(((rect[2] - dash_len) / 2), 0.f,
                                               kUnderlineDefaultSideMargin);
          const float total_len = rect[2] - 2 * side_margin;
          float gap_len = kUnderlineDefaultGapLength;
          int num_full_dashes = static_cast<int>(
              std::floor((total_len + gap_len) / (dash_len + gap_len)));
          const int num_dashes = std::max(num_full_dashes, 1);
          if (num_dashes > 1) {
            const int num_gaps = num_dashes - 1;
            gap_len = (total_len - dash_len * num_full_dashes) / num_gaps;
          }
          for (int i = 0; i < num_dashes; i++) {
            const float x_start =
                rect[0] + side_margin + i * (dash_len + gap_len);
            const float x_end =
                std::min(x_start + dash_len, rect[0] + rect[2] - side_margin);
            canvas_->DrawLine(x_start, line_y, x_end, line_y, painter.get());
          }
          break;
        }
        default:
          break;
      }
      painter->SetStrokeWidth(stroke_width);
    }
    start_char = range.GetRange().GetEnd();
  }
}
void LayoutDrawer::DrawDrawerPiece(const TextLine* line,
                                   const DrawerPiece* run_range,
                                   uint32_t char_start_in_piece,
                                   uint32_t char_end_in_piece) const {
  const auto* run = run_range->GetRun();
  const auto& metrics = run->GetMetrics();
  if (run->IsTextRun() || (run->IsGhostRun() && run->GetCharCount() > 0)) {
    auto y_offset = run_range->GetYOffsetInLine();
    auto y = line->GetLineTop() + y_offset;
    RectF background_rect =
        RectF::MakeLTRB(run_range->GetXOffset() +
                            run_range->GetWidthInRange(0, char_start_in_piece),
                        line->GetLineTop(),
                        run_range->GetXOffset() +
                            run_range->GetWidthInRange(0, char_end_in_piece),
                        line->GetLineBottom());
    auto v_align = run->GetLayoutStyle().GetVerticalAlignment();
    if (v_align == CharacterVerticalAlignment::kSubScript ||
        v_align == CharacterVerticalAlignment::kSuperScript) {
      background_rect.SetTop(y + metrics.GetMaxAscent());
      background_rect.SetBottom(y + metrics.GetMaxDescent());
    } else {
      background_rect.SetTop(line->GetLineTop());
      background_rect.SetBottom(line->GetLineBottom());
    }

    auto piece_start =
        run_range->GetStartCharPosInParagraph() - run->GetStartCharPos();
    auto piece_end =
        run_range->GetEndCharPosInParagraph() - run->GetStartCharPos();
    auto& result = run->shape_result_;
    auto glyph_start = result.CharToGlyph(piece_start);
    auto glyph_end = result.CharToGlyph(piece_end);
    auto glyph_count = glyph_end - glyph_start;
    std::vector<float> pos_x(glyph_count);
    std::vector<float> pos_y(glyph_count);
    float char_x_pos = 0.f;
    float min_x = 0.f;
    for (uint32_t k = 0; k < glyph_count; k++) {
      auto invert_glyph_id =
          glyph_start + (run->IsRtl() ? glyph_count - k - 1 : k);
      pos_y[k] = result.Positions(invert_glyph_id)[1];
      pos_x[k] = char_x_pos;
      auto adv = result.Advances(invert_glyph_id)[0];
      char_x_pos += adv;
      min_x = std::fmin(min_x, char_x_pos);
      if (FloatsLarger(adv, 0)) {
        char_x_pos += run->GetLayoutStyle().GetLetterSpacing();
      }
    }
    for (auto& xx : pos_x) {
      xx -= min_x;
    }

    DrawTextRun(run, piece_start, piece_end, run_range->GetXOffset(), y,
                pos_x.data(), pos_y.data(),
                background_rect.ToArrayLTWH().data());
    background_rect.SetLeft(background_rect.GetRight());
    // TODO(hfuttyh) need to implement
    // if (run_range->GetHyphnateRun()) {
    //   const auto* hyphnate_run = run_range->GetHyphnateRun();
    //   background_rect.SetWidth(hyphnate_run->GetWidth(0, nullptr));
    //   DrawTextRun(hyphnate_run, 0, hyphnate_run->GetCharCount(),
    //               background_rect.GetLeft(), y, background_rect);
    // }
  }
  if (run->GetType() == RunType::kInlineObject) {
    auto y_offset = run_range->GetYOffsetInLine();
    auto y = line->GetLineTop() + y_offset;
    run->GetRunDelegate()->Draw(canvas_, run_range->GetXOffset(),
                                y + metrics.GetMaxAscent());
  }
}
float LayoutDrawer::DrawTextRun(const BaseRun* run, uint32_t start_char_in_run,
                                uint32_t end_char_in_run, float x, float y,
                                float* pos_x, float* pos_y,
                                float* background_rect) const {
  auto piece_start = start_char_in_run;
  auto char_end_pos = end_char_in_run;
  auto& layout_style = run->GetLayoutStyle();
  const auto* paragraph = run->GetParagraph();
  StyleManager* style_manager = nullptr;
  if (paragraph != nullptr) {
    style_manager = paragraph->style_manager_.get();
  }
  while (piece_start < char_end_pos) {
    const Style* style = nullptr;
    auto default_painter = canvas_->CreatePainter();
    Painter* painter = nullptr;
    auto piece_end = char_end_pos;
    StyleRange style_range;
    if (style_manager != nullptr) {
      style_manager->GetStyleRange(
          &style_range, run->GetStartCharPos() + piece_start,
          Style::ForegroundColorFlag() | Style::ForegroundPainterFlag() |
              Style::BaselineOffsetFlag());
      piece_end = std::min(char_end_pos, style_range.GetRange().GetEnd() -
                                             run->GetStartCharPos());
      style = &style_range.GetStyle();
      painter = style->GetForegroundPainter();
    } else {
      style = &run->GetLayoutStyle();
    }
    if (painter == nullptr) {
      painter = default_painter.get();
      auto& fd = layout_style.GetFontDescriptor();
      if (!fd.font_family_list_.empty()) {
        painter->SetFontFamily(fd.font_family_list_[0]);
      }
      painter->SetTextSize(layout_style.GetTextSize());
      painter->SetColor(style->GetForegroundColor().GetPlainColor());
      painter->SetBold(layout_style.GetBold());
      painter->SetItalic(layout_style.GetItalic());
    } else {
      // prefer text size in style object
      if (layout_style.HasTextSize()) {
        painter->SetTextSize(layout_style.GetTextSize());
      }
    }
    auto& result = run->shape_result_;
    auto glyph_start = result.CharToGlyph(piece_start);
    auto glyph_end = result.CharToGlyph(piece_end);
    auto glyph_count = glyph_end - glyph_start;
    if (glyph_start == glyph_end) {
      if (piece_start == piece_end) {
        piece_end++;
      }
    } else {
      y += style->GetBaselineOffset();
      std::vector<uint16_t> glyphs(glyph_count);
      std::vector<const ITypefaceHelper*> fonts(glyph_count);
      std::vector<float> advance(glyph_count);
      uint32_t prev_start = 0;
      uint32_t prev_glyph_id = run->IsRtl() ? glyph_count - 1 : 0;
      for (uint32_t k = 0; k < glyph_count; k++) {
        auto invert_glyph_id =
            glyph_start + (run->IsRtl() ? glyph_count - k - 1 : k);
        glyphs[k] = result.Glyphs(invert_glyph_id);
        fonts[k] = result.Font(invert_glyph_id).get();
        if (k > 0 && fonts[k] != fonts[k - 1]) {
          DrawGlyphsOrText(run, glyphs.data() + prev_start, k - prev_start,
                           prev_glyph_id + glyph_start, fonts[prev_start], x, y,
                           pos_x + prev_start, pos_y + prev_start, painter);
          prev_start = k;
          prev_glyph_id = invert_glyph_id - glyph_start;
        }
      }
      DrawGlyphsOrText(run, glyphs.data() + prev_start,
                       glyph_count - prev_start, prev_glyph_id + glyph_start,
                       fonts[prev_start], x, y, pos_x + prev_start,
                       pos_y + prev_start, painter);
    }
    piece_start = piece_end;
  }
  return x;
}

void LayoutDrawer::DrawGlyphsOrText(const BaseRun* run, uint16_t* glyphs,
                                    uint32_t glyph_count,
                                    uint32_t glyph_start_index,
                                    const ITypefaceHelper* font, float ox,
                                    float oy, float* pos_x, float* pos_y,
                                    Painter* p) const {
  if (glyph_count == 0) return;
  auto desired_font_style =
      run->GetShapeStyle().GetFontDescriptor().font_style_;
  if (desired_font_style != font->FontStyle()) {
    auto wantedWeight = desired_font_style.GetWeight();
    const bool fakeBold =
        wantedWeight >= FontStyle::Weight::kSemiBold_Weight &&
        (int32_t)wantedWeight - (int32_t)font->FontStyle().GetWeight() >= 200;
    auto need_italic =
        desired_font_style.GetSlant() == FontStyle::Slant::kItalic_Slant;
    const bool fakeItalic = need_italic && font->FontStyle().GetSlant() !=
                                               FontStyle::Slant::kItalic_Slant;
    p->SetBold(fakeBold);
    p->SetItalic(fakeItalic);
  }
  while (glyph_count > 0 && glyphs[glyph_count - 1] == 0) glyph_count--;
  /* tricky rtl implemention */
  canvas_->DrawGlyphs(font, glyph_count, glyphs, nullptr, run->IsRtl() ? 1 : 0,
                      ox, oy + run->baseline_offset_, pos_x, pos_y, p);
#if defined(DRAW_LINE_BOX)
  auto painter = canvas_->CreatePainter();
  painter->SetFillStyle(FillStyle::kStroke);
  painter->SetColor(0xffff0000);
  painter->SetStrokeWidth(1);
  canvas_->DrawLine(ox, oy, ox + pos_x[glyph_count - 1], oy, painter.get());
  painter->SetColor(0xFF00FF00);
  auto metrics = run->GetMetrics();
  canvas_->DrawRect(ox, oy + metrics.GetMaxAscent(),
                    ox + pos_x[glyph_count - 1], oy + metrics.GetMaxDescent(),
                    painter.get());
#endif
}
}  // namespace tttext
}  // namespace ttoffice
