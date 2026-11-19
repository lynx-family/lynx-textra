// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef DEMOS_CAPI_TEXTRA_DEMO_CPP_HPP_
#define DEMOS_CAPI_TEXTRA_DEMO_CPP_HPP_
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <cmath>
#include <cstdio>

#include "textra_demo.h"

namespace textra_demo {
inline bool RenderCpp(ttoffice::tttext::RenderingBackend backend,
                      FontManagerRef manager, const void* native_font,
                      ttoffice::tttext::ICanvasHelper* canvas,
                      uint32_t sample_index, float width, float height,
                      textra_demo_metrics_t* metrics, bool rounded = false,
                      float font_size = 24) {
  using namespace ttoffice::tttext;
  const auto& sample = textra_demo_cases[sample_index];
  FontmgrCollection fonts(manager);
  TextLayout engine(&fonts, backend);
  auto paragraph =
      Paragraph::Create(sample.single_line ? ParagraphType::kSingleLine
                                           : ParagraphType::kGeneral);
  ttoffice::tttext::Style style;
  FontDescriptor fd;
  fd.platform_font_ = reinterpret_cast<uintptr_t>(native_font);
  style.SetFontDescriptor(fd);
  style.SetTextSize(font_size);
  style.SetForegroundColor(TTColor{0xff203040});
  style.SetBold(sample.bold != 0);
  style.SetItalic(sample.italic != 0);
  paragraph->AddTextRun(&style, sample.text,
                        static_cast<uint32_t>(strlen(sample.text)));
  TTTextContext context;
  context.SetAndroidMetricsRound(rounded);
  LayoutDrawer drawer(canvas);
  if (sample.single_line) {
    if (!engine.LayoutSingleLine(paragraph.get(), context)) return false;
    metrics->width = paragraph->GetWidth();
    metrics->height = paragraph->GetHeight();
    metrics->lines = 1;
    return drawer.DrawSingleLine(*paragraph, 0, 0);
  }
  LayoutRegion region(width, height, LayoutMode::kDefinite,
                      LayoutMode::kDefinite);
  if (engine.Layout(paragraph.get(), &region, context) != LayoutResult::kNormal)
    return false;
  metrics->width = region.GetLayoutedWidth();
  metrics->height = region.GetLayoutedHeight();
  metrics->lines = region.GetLineCount();
  drawer.DrawLayoutPage(&region);
  return true;
}
inline bool EqualMetrics(const textra_demo_metrics_t& a,
                         const textra_demo_metrics_t& b) {
  return a.lines == b.lines && std::abs(a.width - b.width) < 0.001f &&
         std::abs(a.height - b.height) < 0.001f;
}
}  // namespace textra_demo
#endif  // DEMOS_CAPI_TEXTRA_DEMO_CPP_HPP_
