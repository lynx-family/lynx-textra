// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_TT_TEXT_MANAGER_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_TT_TEXT_MANAGER_H_

#include <textra/font_info.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>
#include <textra/icu_wrapper.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/paragraph_style.h>
#include <textra/platform/java/java_canvas_helper.h>
#include <textra/platform/java/tttext_jni_proxy.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <memory>

using namespace tttext;

class TTTextManager {
 public:
  TTTextManager();
  ~TTTextManager() = default;

  void CreateParagraph();
  void AddText(const char* content, float font_size, uint32_t color);
  LayoutRegion* LayoutContent(float height);
  FontManagerRef GetDefaultFontManager() {
    return font_collection_.GetDefaultFontManager();
  }
  JavaCanvasHelper* GetCanvas() { return canvas_.get(); }
  LayoutRegion* GetRetgion() { return region_.get(); }

 private:
  std::unique_ptr<Paragraph> paragraph;
  std::unique_ptr<TextLayout> layout_;
  std::unique_ptr<TTTextContext> context_;
  FontmgrCollection font_collection_;
  std::unique_ptr<LayoutRegion> region_;
  std::unique_ptr<JavaCanvasHelper> canvas_;
};

#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_TT_TEXT_MANAGER_H_
