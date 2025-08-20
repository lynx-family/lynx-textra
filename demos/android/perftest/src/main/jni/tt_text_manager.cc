// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "tt_text_manager.h"

#include <textra/platform/java/java_font_manager.h>

TTTextManager::TTTextManager()
    : font_collection_(std::make_shared<tttext::JavaFontManager>()) {
  layout_ =
      std::make_unique<TextLayout>(&font_collection_, ShaperType::kSystem);
  context_ = std::make_unique<TTTextContext>();
  context_->SetSkipSpacingBeforeFirstLine(false);
  context_->SetLastLineCanOverflow(false);
  canvas_ = std::make_unique<JavaCanvasHelper>();
}
void TTTextManager::CreateParagraph() {
  paragraph = Paragraph::Create();
  auto& ppr = paragraph->GetParagraphStyle();
  ppr.SetHorizontalAlign(ParagraphHorizontalAlignment::kLeft);
  region_ = nullptr;
}
void TTTextManager::AddText(const char* content, float font_size,
                            uint32_t color) {
  Style run_pr;
  run_pr.SetTextSize(font_size);
  run_pr.SetForegroundColor(TTColor(color));
  paragraph->AddTextRun(&run_pr, content);
}

LayoutRegion* TTTextManager::LayoutContent(float height) {
  context_->Reset();
  region_ = std::make_unique<LayoutRegion>(500, height, LayoutMode::kDefinite,
                                           LayoutMode::kAtMost);
  layout_->Layout(paragraph.get(), region_.get(), *context_);
  return region_.get();
}
