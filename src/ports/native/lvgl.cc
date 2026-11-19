// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <atomic>

#include "src/ports/native/lvgl/lvgl_canvas_helper.h"
#include "src/ports/native/lvgl/lvgl_font_manager.h"
#include "src/ports/native/lvgl/lvgl_typeface.h"
#include "src/ports/native/native_helpers.h"

namespace ttoffice {
namespace tttext {
namespace native {
namespace {
// Native fonts remain borrowed. The engine owns only this Typeface adapter.
class ImportedLVGLTypeface final : public tttext::LVGLTypeface {
 public:
  explicit ImportedLVGLTypeface(const lv_font_t* font)
      : tttext::LVGLTypeface(next_id_.fetch_add(1)), font_(font) {}
  const lv_font_t* GetLvFont(float) const override { return font_; }

 private:
  static std::atomic<uint32_t> next_id_;
  const lv_font_t* font_;
};
std::atomic<uint32_t> ImportedLVGLTypeface::next_id_{0x20000000u};
}  // namespace

FontManagerRef LVGLFontManager() {
  return std::make_shared<tttext::LVGLFontManager>();
}
std::unique_ptr<ICanvasHelper> LVGLCanvas(void* target, const FontManagerRef&) {
  return std::make_unique<LVGLCanvasHelper>(static_cast<lv_layer_t*>(target));
}
TypefaceRef LVGLTypeface(const void* font, const FontManagerRef&) {
  return std::make_shared<ImportedLVGLTypeface>(
      static_cast<const lv_font_t*>(font));
}
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
