// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "textra_test.h"

#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/platform_helper.h>
#include <textra/style.h>
#include <textra/tt_color.h>

void TextraBuildParagraph(void* ctx, const std::string& content) {
  auto context = (TextraContext*)ctx;
  context->context_ = std::make_unique<tttext::TTTextContext>();
  auto font_manager =
      tttext::PlatformHelper::CreateFontManager(tttext::PlatformType::kSystem);
  tttext::FontmgrCollection fc(font_manager);
  context->layout_ =
      std::make_unique<tttext::TextLayout>(&fc, tttext::ShaperType::kSystem);
  context->paragraph_ = tttext::Paragraph::Create();
  tttext::Style style;
  style.SetTextSize(24);
  style.SetForegroundColor(tttext::TTColor(0xFFFF0000));
  context->paragraph_->AddTextRun(&style, content.c_str());
}
void TextraLayoutParagraph(void* ctx, double width) {
  auto context = (TextraContext*)ctx;
  context->region_ =
      std::make_unique<tttext::LayoutRegion>(width, tttext::LAYOUT_MAX_UNITS);
  context->layout_->Layout(context->paragraph_.get(), context->region_.get(),
                           *(context->context_));
}
void TextraDrawParagraph(void* ctx, OH_Drawing_Canvas* canvas) {
  auto context = (TextraContext*)ctx;
  if (context == nullptr || context->region_ == nullptr || canvas == nullptr)
    return;
  auto textra_canvas = tttext::PlatformHelper::CreateCanvasHelper(
      tttext::PlatformType::kSystem, canvas);
  textra_canvas->Translate(0, 0);
  tttext::LayoutDrawer drawer(textra_canvas.get());
  drawer.DrawLayoutPage(context->region_.get());
}
