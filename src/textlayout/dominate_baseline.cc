// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/dominate_baseline.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/textlayout/text_line_impl.h"

namespace ttoffice {
namespace tttext {

float DominateBaselineHelper::CalcBaselineOffset(DominantBaseline baseline,
                                                 float ascent, float descent) {
  switch (baseline) {
    case DominantBaseline::kTop:
      return 0.f;
    case DominantBaseline::kMiddle:
      return 0.5f * (descent - ascent);
    case DominantBaseline::kHanging:
      return 0.2f * -ascent;
    case DominantBaseline::kBottom:
    case DominantBaseline::kIdeographic:
      return descent - ascent;
    default:
      return -ascent;
  }
}

std::pair<float, float> DominateBaselineHelper::CalcRunEmHeight(
    const BaseRun* run) {
  auto run_metrics = run->GetMetrics();
  float run_em_ascent = run_metrics.GetMaxAscent();
  float run_em_descent = run_metrics.GetMaxDescent();
  if (!run->shape_result_.Valid()) {
    return std::make_pair(run_em_ascent, run_em_descent);
  }
  float min_em_ascent = 0.f;
  float max_em_descent = 0.f;
  bool has_font = false;
  float font_size = run->GetLayoutStyle().GetTextSize();
  auto glyph_count = run->shape_result_.GlyphCount();
  for (uint32_t idx = 0; idx < glyph_count; ++idx) {
    auto font = run->shape_result_.Font(idx);
    if (!font) {
      continue;
    }
    has_font = true;
    auto em = font->GetFontEmHeight(font_size);
    min_em_ascent = std::min(min_em_ascent, -em.GetAscent());
    max_em_descent = std::max(max_em_descent, em.GetDescent());
  }
  if (has_font) {
    run_em_ascent = min_em_ascent;
    run_em_descent = max_em_descent;
  }
  return std::make_pair(run_em_ascent, run_em_descent);
}

void DominateBaselineHelper::ApplyDominateBaseline(TextLineImpl* line) {
  auto dominate_baseline =
      line->paragraph_->GetParagraphStyleImpl().GetDominantBaseline();
  if (dominate_baseline == DominantBaseline::kAlphabetic) {
    return;
  }
  std::vector<std::pair<float, float>> piece_em_heights;
  piece_em_heights.reserve(line->drawer_list_.size());

  float line_em_ascent = 0.f;
  float line_em_descent = 0.f;
  for (auto& piece : line->drawer_list_) {
    auto run_em = CalcRunEmHeight(piece->GetRun());
    piece_em_heights.push_back(run_em);
    line_em_ascent = std::min(line_em_ascent, run_em.first);
    line_em_descent = std::max(line_em_descent, run_em.second);
  }
  if (piece_em_heights.empty()) {
    line_em_ascent = -line->max_ascent_;
    line_em_descent = line->max_descent_;
  }

  float line_baseline =
      line->max_ascent_ + line_em_ascent +
      CalcBaselineOffset(dominate_baseline, line_em_ascent, line_em_descent);
  line_baseline = std::clamp(line_baseline, 0.f, line->GetLineHeight());
  float line_top = line->GetLineTop() + line->top_extra_;
  for (size_t piece_idx = 0; piece_idx < line->drawer_list_.size();
       ++piece_idx) {
    auto& piece = line->drawer_list_[piece_idx];
    auto* run = piece->GetRun();
    auto metrics = run->GetMetrics();
    auto run_em = piece_em_heights[piece_idx];
    float piece_baseline =
        CalcBaselineOffset(dominate_baseline, run_em.first, run_em.second);
    float old_piece_baseline = -run_em.first;
    float piece_offset_in_line =
        line_baseline + old_piece_baseline - piece_baseline;
    piece->SetYOffsetInLine(piece_offset_in_line);
    if (auto type = run->GetType();
        type == RunType::kInlineObject && run->GetRunDelegate() != nullptr) {
      auto delegate = run->GetRunDelegate();
      delegate->SetOffset(
          delegate->GetXOffset(),
          line_top + piece_offset_in_line + metrics.GetMaxAscent());
    }
  }
  float line_height = line->GetLineHeight();
  line->max_ascent_ = line_baseline;
  line->max_descent_ = line_height - line_baseline;
}

}  // namespace tttext
}  // namespace ttoffice
