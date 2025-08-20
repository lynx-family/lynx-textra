// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/text_layout_impl.h"

#include <textra/tttext_context.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "src/textlayout/layout_measurer.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/text_line_impl.h"
#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
LayoutResult TextLayoutImpl::LayoutEx(Paragraph* i_para, LayoutRegion* page,
                                      TTTextContext& context,
                                      TTShaper* shaper) {
  LayoutResult result = LayoutResult::kNormal;
  auto* para = TTDYNAMIC_CAST<ParagraphImpl*>(i_para);
  para->SetShaper(shaper);
  para->FormatRunList();
  auto& pos = context.GetPositionRef();
  std::unique_ptr<TextLineImpl> current_line = nullptr;
  if (page->GetPageWidth() <= 0 || page->GetPageHeight() <= 0) return result;
  while (pos.GetRunIdx() < para->GetRunCount() &&
         result == LayoutResult::kNormal && !page->IsFullFilled()) {
    if (current_line == nullptr) {
      current_line = ProcessNewLine(para, page, context);
    }
    pos = ProcessBreakableRunList(*para, pos, page, current_line.get(), context,
                                  &result);
    if (pos.GetRunIdx() == para->GetRunCount() ||
        result == LayoutResult::kBreakLine ||
        result == LayoutResult::kBreakPage ||
        result == LayoutResult::kBreakColumn) {
      if (current_line != nullptr && !current_line->IsEmpty()) {
        FinishLineLayout(page, std::move(current_line), context, &result);
      }
    } else if (result == LayoutResult::kRelayoutLine) {
      pos = current_line->GetStartLayoutPosition();
      current_line->ClearForRelayout();
      result = LayoutResult::kNormal;
    } else if (result == LayoutResult::kRelayoutPage) {
    }
  }
  return result;
}

std::unique_ptr<TextLineImpl> TextLayoutImpl::ProcessNewLine(
    ParagraphImpl* para, LayoutRegion* page, TTTextContext& context) {
  auto new_line =
      std::make_unique<TextLineImpl>(para, page, context.GetPositionRef());
  auto line_top =
      context.GetLayoutBottom() + ProcessLineGap(page, new_line.get(), context);
  new_line->Initialize(line_top);
  return new_line;
}
float TextLayoutImpl::ProcessLineGap(LayoutRegion* page, TextLine* line,
                                     TTTextContext& context) {
  if (page->IsEmpty() && context.IsSkipSpacingBeforeFirstLine()) return 0;
  float last_line_spacing = 0;
  if (!page->IsEmpty()) {
    last_line_spacing = page->GetLine(page->GetLineCount() - 1)
                            ->GetParagraph()
                            ->GetParagraphStyle()
                            .GetLineSpaceAfterPx();
  }
  return line->GetParagraph()->GetParagraphStyle().GetLineSpaceBeforePx() +
         last_line_spacing;
}
void TextLayoutImpl::FinishLineLayout(LayoutRegion* page,
                                      std::unique_ptr<TextLineImpl> line,
                                      TTTextContext& context,
                                      LayoutResult* result) {
  line->SetLayouted();
  auto& pos = context.GetPositionRef();
  line->CreateDrawerPiece();
  line->ApplyAlignment();
  /*
   * Extra inserted single-line images with no spacing (stick to top/bottom)
   * */
  auto layouted_height = line->GetLineBottom();
  bool break_page = true;
  bool keep_line = true;
  if (page->GetLineCount() + 1 >=
      line->GetParagraph()->GetParagraphStyle().GetMaxLines()) {
    page->SetExceededMaxLines(line->GetEndLayoutPosition().GetRunIdx() <
                              line->GetParagraph()->GetRunCount());
  } else if (FloatsLargerOrEqual(page->GetPageHeight(), layouted_height)) {
    break_page = false;
  } else if (FloatsLarger(page->GetPageHeight(), line->GetLineTop()) &&
             context.IsLastLineCanOverflow()) {
  } else {
    keep_line = false;
  }

  TTASSERT(keep_line || break_page);
  auto* current_line = line.get();
  if (keep_line) {
    context.UpdateContextSpace(line.get());
    *result = page->AddLine(std::move(line), context);
  } else {
    pos.Update(line->GetStartLayoutPosition());
  }
  if (break_page) {
    page->SetFullFilled(true);
    *result = LayoutResult::kBreakPage;
    auto* last_line = page->GetLine(page->GetLineCount() - 1);
    if (last_line != nullptr) {
      auto* last_line_paragraph = last_line->GetParagraph();
      if (current_line != last_line ||
          pos.GetRunIdx() != last_line_paragraph->GetRunCount()) {
        last_line->StripByEllipsis(nullptr);
        page->UpdateLayoutedSize(last_line, context);
      }
    }
  } else {
    page->SetFullFilled(false);
  }
}
LayoutPosition TextLayoutImpl::ProcessSpecialRun(const ParagraphImpl& paragraph,
                                                 const LayoutPosition& position,
                                                 LayoutRegion* page,
                                                 TextLine* line,
                                                 TTTextContext& context,
                                                 LayoutResult* result) {
  auto pos = position;
  auto* run = paragraph.GetRun(pos.GetRunIdx());
  if (run == nullptr) return pos;
  return pos;
}
LayoutPosition TextLayoutImpl::ProcessBreakableRunList(
    const ParagraphImpl& paragraph, const LayoutPosition& position,
    LayoutRegion* page, TextLineImpl* line, TTTextContext& context,
    LayoutResult* result) {
  TTASSERT(!line->IsLayouted());
  auto pos =
      AddBreakableRunsToLine(paragraph, position, line, page, context, result);
  return pos;
}
bool TextLayoutImpl::CheckLineNeedRelayout(LayoutRegion* region,
                                           TextLineImpl* line, float new_height,
                                           float& next_line_top) {
  next_line_top = line->line_top_;
  std::vector<std::array<float, 2>> list;
  if (line->range_lst_.empty()) {
    list = region->GetRangeList(&next_line_top, new_height,
                                line->GetStartIndent(), line->GetEndIndent());
    TTASSERT(!list.empty());
    line->SetRangeLst(list);
  } else if (FloatsLarger(new_height, line->GetLineHeight())) {
    list = region->GetRangeList(&next_line_top, new_height,
                                line->GetStartIndent(), line->GetEndIndent());
    TTASSERT(!list.empty());
    if (!line->IsEqualRangeList(list)) {
      // Line reflow
      line->SetRangeLst(list);
      return true;
    }
  }
  return false;
}
LayoutPosition TextLayoutImpl::AddBreakableRunsToLine(
    const ParagraphImpl& paragraph, const LayoutPosition& position,
    TextLineImpl* line, LayoutRegion* region, TTTextContext& context,
    LayoutResult* result) {
  auto pos = position;
  *result = LayoutResult::kNormal;
  const auto* run = paragraph.GetRun(pos.GetRunIdx());
  float next_line_top = line->line_top_;
  auto metrics = LayoutMetrics(-line->GetMaxAscent(), line->GetMaxDescent());
  auto new_height = TryAddRun(metrics, run);

  if (CheckLineNeedRelayout(region, line, new_height, next_line_top)) {
    *result = LayoutResult::kRelayoutLine;
    return position;
  }

  //  TTASSERT(paragraph.GetBoundaryTypeBefore(pos) >= BoundaryType::kWord);
  auto range_idx = line->current_available_range_index_;
  auto greedy_break_pos = pos;
  auto range_lst_size = static_cast<int>(line->range_lst_.size());
  while (range_idx < range_lst_size) {
    auto& range = line->range_lst_[range_idx];
    auto range_width = range->GetAvailableWidth();
    greedy_break_pos = FindBreakPosInWord(paragraph, pos, &range_width, result);
    // break at default breakable position if not the last line, otherwise
    // break at any position.
    bool is_last_line = region->GetLineCount() + 1 >=
                        line->GetParagraph()->GetParagraphStyle().GetMaxLines();
    auto line_break_pos =
        is_last_line ? greedy_break_pos
                     : paragraph.FindPrevBoundary(greedy_break_pos,
                                                  BoundaryType::kLineBreakable);
    if (line_break_pos < pos) line_break_pos = pos;
    if (line_break_pos > pos) {
      auto d_height = AddWordListToRunRange(range.get(), paragraph, pos,
                                            line_break_pos, &metrics);
      if (CheckLineNeedRelayout(region, line, d_height, next_line_top)) {
        *result = LayoutResult::kRelayoutLine;
        return position;
      }
      line->UpdateLine(line_break_pos, metrics.GetMaxAscent(),
                       metrics.GetMaxDescent(), d_height);
      pos = line_break_pos;
    }
    if (line_break_pos.GetRunIdx() >= paragraph.GetRunCount() ||
        *result != LayoutResult::kNormal) {
      // all of the run has been processed
      return line_break_pos;
    }
    if (range_idx + 1 == range_lst_size) {
      break;
    }
    line->current_available_range_index_++;
    range_idx = line->current_available_range_index_;
  }
  auto break_pos =
      BreakWordForWidth(paragraph, pos, greedy_break_pos,
                        line->GetCurrentRange()->GetAvailableWidth());

  // Not a complete empty line, move line down below floating elements for
  // relayout
  if (break_pos == pos && line->IsEmpty()) {
    if (line->range_lst_.size() > 1 ||
        FloatsLarger(region->GetPageWidth(),
                     line->GetCurrentRange()->GetRangeWidth() +
                         line->GetStartIndent() + line->GetEndIndent())) {
      auto list =
          region->GetRangeList(&next_line_top, line->GetLineHeight(),
                               line->GetStartIndent(), line->GetEndIndent());
      if (FloatsLarger(next_line_top, region->GetPageHeight())) {
        *result = LayoutResult::kBreakPage;
      } else {
        *result = LayoutResult::kRelayoutLine;
      }
      line->range_lst_.clear();
      line->UpdateLineTop(next_line_top);
      return position;
    }
    // Force Layout
    break_pos = ForceBreakWord(paragraph, pos, greedy_break_pos,
                               paragraph.GetParagraphStyle().GetOverflowWrap());
    TTASSERT(break_pos > pos);
  }

  if (break_pos > pos) {
    auto d_height = AddWordListToRunRange(line->GetCurrentRange(), paragraph,
                                          pos, break_pos, &metrics);
    if (CheckLineNeedRelayout(region, line, d_height, next_line_top)) {
      *result = LayoutResult::kRelayoutLine;
      return position;
    }
    line->UpdateLine(break_pos, metrics.GetMaxAscent(), metrics.GetMaxDescent(),
                     d_height);
  }
  *result = LayoutResult::kBreakLine;
  return break_pos;
}
float TextLayoutImpl::TryAddRun(LayoutMetrics line_metrics,
                                const BaseRun* run) {
  auto paragraph = run->GetParagraph();
  auto& para_style = paragraph->GetParagraphStyle();
  if (para_style.GetLineHeightRule() == RulerType::kExact) {
    return para_style.GetLineHeightInPx();
  }
  auto run_metrics = run->GetMetrics();
  auto desired_height = run_metrics.GetHeight();
  if (!run->IsObjectRun()) {
    desired_height = LayoutMeasurer::CalcRunHeightByLinePitch(
        para_style.GetSpacing(), run_metrics.GetHeight(), 0);
    run_metrics.ApplyDesiredHeight(desired_height);
  }
  float baseline_offset =
      paragraph->style_manager_->GetBaselineOffset(run->GetStartCharPos());
  run_metrics.ApplyBaselineOffset(baseline_offset);
  line_metrics.UpdateMax(run_metrics);
  return line_metrics.GetHeight();
}
float TextLayoutImpl::AddWordListToRunRange(LineRange* range,
                                            const ParagraphImpl& paragraph,
                                            const LayoutPosition& start_pos,
                                            const LayoutPosition& end_pos,
                                            LayoutMetrics* metrics) {
  float max_desired_height = LAYOUT_MIN_UNITS;
  for (auto pos = start_pos; pos < end_pos; pos.NextRun()) {
    auto* run = paragraph.GetRun(pos.GetRunIdx());
    auto desired_height = TryAddRun(*metrics, run);
    max_desired_height = std::fmax(max_desired_height, desired_height);
    auto run_metrics = run->GetMetrics();
    float baseline_offset =
        paragraph.style_manager_->GetBaselineOffset(run->GetStartCharPos());
    run_metrics.ApplyBaselineOffset(baseline_offset);
    metrics->UpdateMax(run_metrics);
    auto end_char_in_run = pos.GetRunIdx() == end_pos.GetRunIdx()
                               ? end_pos.GetCharIdx()
                               : run->GetCharCount();
    std::unique_ptr<RunRange> run_range(
        new RunRange{run, range, pos.GetCharIdx(), end_char_in_run});
    range->AddWordRange(std::move(run_range));
  }
  return max_desired_height;
}
LayoutPosition TextLayoutImpl::FindBreakPosInWord(
    const ParagraphImpl& paragraph, const LayoutPosition& position,
    float* max_width, LayoutResult* result) {
  auto break_pos = position;
  const auto end_pos =
      paragraph.FindNextBoundary(position, BoundaryType::kMustLineBreak);
  while (break_pos < end_pos) {
    const auto* break_run = paragraph.GetRun(break_pos.GetRunIdx());
    auto break_pos_in_run = break_pos.GetCharIdx();
    if (break_run->GetCharCount() == 0 ||
        break_run->GetType() == RunType::kInlineObject) {
      TTASSERT(break_pos_in_run == 0);
      const auto width = break_run->GetWidth(0);
      if (FloatsLarger(width, *max_width)) {
        return break_pos;
      }
      *max_width -= width;
      break_pos_in_run = break_run->GetCharCount();
    } else {
      const auto width =
          break_run->MeasureRunByWidth(break_pos_in_run, *max_width);
      if (break_pos_in_run == break_pos.GetCharIdx()) {
        TTASSERT(width == 0);
        break;
        // return break_pos;
      }
      *max_width -= width;
    }
    if (break_pos_in_run == break_run->GetCharCount()) {
      break_pos.NextRun();
    } else {
      break_pos.SetCharIdx(break_pos_in_run);
    }
  }
  // If there are consecutive spaces \r\n after line break position, no need to
  // move to next line
  while (break_pos < end_pos) {
    const auto* break_run = paragraph.GetRun(break_pos.GetRunIdx());
    auto break_pos_in_run = break_pos.GetCharIdx();
    auto max_break_pos_in_run =
        break_pos.GetRunIdx() == end_pos.GetRunIdx()
            ? std::min(end_pos.GetCharIdx(), break_pos.GetCharIdx())
            : break_run->GetCharCount();
    auto start_char = break_run->GetStartCharPos();
    while (break_pos_in_run < max_break_pos_in_run) {
      auto ch32 = paragraph.content_.GetUnicode(start_char + break_pos_in_run);
      if (!base::IsSpaceChar(ch32)) {
        break;
      }
      break_pos_in_run++;
    }
    TTASSERT(break_pos_in_run <= max_break_pos_in_run);
    if (break_pos_in_run == break_run->GetCharCount()) {
      break_pos.NextRun();
    } else {
      break_pos.SetCharIdx(break_pos_in_run);
      break;
    }
  }
  if (break_pos > end_pos) {
    break_pos = end_pos;
    *result = LayoutResult::kBreakLine;
  }
  return break_pos;
}
LayoutPosition TextLayoutImpl::BreakWordForWidth(
    const ParagraphImpl& paragraph, const LayoutPosition& position,
    const LayoutPosition& greedy_break_pos, float max_width,
    OverflowWrap overflow_wrap) {
  const auto* run = paragraph.GetRun(position.GetRunIdx());
  if (run->IsGhostRun()) {
    return position;
  }
  auto break_pos = greedy_break_pos;
  if (break_pos == position) return position;
  const auto word_start_pos =
      paragraph.FindPrevBoundary(break_pos, BoundaryType::kWord);
  const auto word_end_pos =
      paragraph.FindNextBoundary(break_pos, BoundaryType::kWord);

#ifdef TTTEXT_DEBUG
  run = paragraph.GetRun(word_end_pos.GetRunIdx());
  TTASSERT(run == nullptr || run->GetCharCount() == 0 ||
           word_end_pos.GetCharIdx() < run->GetCharCount());
#endif
  const auto word_start_in_para =
      paragraph.LayoutPositionToCharPos(word_start_pos);
  const auto word_end_in_para = paragraph.LayoutPositionToCharPos(word_end_pos);
  const auto word = paragraph.GetContent(word_start_in_para,
                                         word_end_in_para - word_start_in_para);
  LayoutMetrics tmp_metrics{};
  for (auto pos = position; pos < break_pos; pos.NextRun()) {
    run = paragraph.GetRun(pos.GetRunIdx());
    tmp_metrics.UpdateMax(run->GetMetrics());
  }
  const auto* break_run = paragraph.GetRun(break_pos.GetRunIdx());
  TTASSERT(break_run != nullptr);
  TTASSERT(break_pos < word_end_pos);
  if (break_run->GetCharCount() == 0) {
    return position;
  }
  TTASSERT(break_run->GetType() == RunType::kTextRun);
  if (paragraph.style_manager_->GetWordBreak(break_run->GetStartCharPos() +
                                             break_pos.GetCharIdx() - 1) ==
      WordBreakType::kBreakAll) {
    return break_pos;
  }
  return position;
}
LayoutPosition TextLayoutImpl::ForceBreakWord(
    const ParagraphImpl& paragraph, const LayoutPosition& position,
    const LayoutPosition& greedy_break_pos, OverflowWrap overflow_wrap) {
  if (overflow_wrap == OverflowWrap::kNormal) {
    return paragraph.FindNextBoundary(position, BoundaryType::kLineBreakable);
  }
  auto break_pos = greedy_break_pos;
  if (break_pos == position) {
    break_pos.NextChar();
  }
  const auto* break_run = paragraph.GetRun(break_pos.GetRunIdx());
  TTASSERT(break_run != nullptr);
  if (break_run->GetCharCount() == 0) {
    return paragraph.FindNextBoundary(position, BoundaryType::kLineBreakable);
  }
  TTASSERT(break_pos.GetCharIdx() <= break_run->GetCharCount());
  if (break_pos.GetCharIdx() == break_run->GetCharCount()) {
    break_pos.NextRun();
  }
  TTASSERT(break_pos > position);
  return break_pos;
}
}  // namespace tttext
}  // namespace ttoffice
