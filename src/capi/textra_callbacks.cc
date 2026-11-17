// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <textra/c/textra_canvas_helper.h>
#include <textra/c/textra_font.h>
#include <textra/i_canvas_helper.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>

#include "src/capi/textra_capi_internal.h"

namespace {
using tttext::c_api_internal::Handle;
using tttext::c_api_internal::Raw;
using tttext::c_api_internal::Release;
using tttext::c_api_internal::Share;

struct CallbackData {
  CallbackData(void* data, textra_destroy_callback_t destroy)
      : data(data), destroy(destroy) {}
  ~CallbackData() {
    if (destroy) destroy(data);
  }
  void* data;
  textra_destroy_callback_t destroy;
};

// Borrow without allocating a shared_ptr control block or taking ownership.
// The C handle itself is stack-scoped and must not be retained by callbacks.
template <typename T>
std::shared_ptr<T> Borrow(const T* value) {
  return {std::shared_ptr<T>{}, const_cast<T*>(value)};
}

std::shared_ptr<tttext::ITypefaceHelper> TakeTypeface(
    textra_typeface_helper_t* result) {
  if (!result) return nullptr;
  auto value = result->value;
  Release(result);
  return value;
}

textra_font_style_t FontStyle(const tttext::FontStyle& s) {
  return {s.GetWeight(), s.GetWidth(), s.GetSlant()};
}

class CallbackCanvas final : public tttext::ICanvasHelper, public CallbackData {
 public:
  using CallbackData::CallbackData;
  tttext::ICanvasHelper* fallback = nullptr;
  textra_canvas_helper_start_paint_callback_t start_paint = nullptr;
  textra_canvas_helper_end_paint_callback_t end_paint = nullptr;
  textra_canvas_helper_save_callback_t save = nullptr;
  textra_canvas_helper_restore_callback_t restore = nullptr;
  textra_canvas_helper_translate_callback_t translate = nullptr;
  textra_canvas_helper_scale_callback_t scale = nullptr;
  textra_canvas_helper_rotate_callback_t rotate = nullptr;
  textra_canvas_helper_skew_callback_t skew = nullptr;
  textra_canvas_helper_clip_rect_callback_t clip_rect = nullptr;
  textra_canvas_helper_clear_callback_t clear = nullptr;
  textra_canvas_helper_clear_rect_callback_t clear_rect = nullptr;
  textra_canvas_helper_fill_rect_callback_t fill_rect = nullptr;
  textra_canvas_helper_draw_color_callback_t draw_color = nullptr;
  textra_canvas_helper_draw_line_callback_t draw_line = nullptr;
  textra_canvas_helper_draw_rect_callback_t draw_rect = nullptr;
  textra_canvas_helper_draw_oval_callback_t draw_oval = nullptr;
  textra_canvas_helper_draw_circle_callback_t draw_circle = nullptr;
  textra_canvas_helper_draw_arc_callback_t draw_arc = nullptr;
  textra_canvas_helper_draw_arc_to_callback_t draw_arc_to = nullptr;
  textra_canvas_helper_draw_round_rect_callback_t draw_round_rect = nullptr;
  textra_canvas_helper_create_painter_callback_t create_painter = nullptr;
  textra_canvas_helper_draw_text_callback_t draw_text = nullptr;
  textra_canvas_helper_draw_glyphs_callback_t draw_glyphs = nullptr;
  void StartPaint() override {
    if (start_paint)
      start_paint(data);
    else if (fallback)
      fallback->StartPaint();
  }
  void EndPaint() override {
    if (end_paint)
      end_paint(data);
    else if (fallback)
      fallback->EndPaint();
  }
  void Save() override {
    if (save)
      save(data);
    else if (fallback)
      fallback->Save();
  }
  void Restore() override {
    if (restore)
      restore(data);
    else if (fallback)
      fallback->Restore();
  }
  void Translate(float x, float y) override {
    if (translate)
      translate(data, x, y);
    else if (fallback)
      fallback->Translate(x, y);
  }
  void Scale(float x, float y) override {
    if (scale)
      scale(data, x, y);
    else if (fallback)
      fallback->Scale(x, y);
  }
  void Rotate(float degrees) override {
    if (rotate)
      rotate(data, degrees);
    else if (fallback)
      fallback->Rotate(degrees);
  }
  void Skew(float x, float y) override {
    if (skew)
      skew(data, x, y);
    else if (fallback)
      fallback->Skew(x, y);
  }
  void ClipRect(float left, float top, float right, float bottom,
                bool antialias) override {
    if (clip_rect)
      clip_rect(data, left, top, right, bottom, antialias);
    else if (fallback)
      fallback->ClipRect(left, top, right, bottom, antialias);
  }
  void Clear() override {
    if (clear)
      clear(data);
    else if (fallback)
      fallback->Clear();
  }
  void ClearRect(float left, float top, float right, float bottom) override {
    if (clear_rect)
      clear_rect(data, left, top, right, bottom);
    else if (fallback)
      fallback->ClearRect(left, top, right, bottom);
  }
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {
    if (fill_rect)
      fill_rect(data, left, top, right, bottom, color);
    else if (fallback)
      fallback->FillRect(left, top, right, bottom, color);
  }
  void DrawColor(uint32_t color) override {
    if (draw_color)
      draw_color(data, color);
    else if (fallback)
      fallback->DrawColor(color);
  }
  void DrawLine(float x1, float y1, float x2, float y2,
                tttext::Painter* painter) override {
    if (draw_line)
      draw_line(data, x1, y1, x2, y2, Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawLine(x1, y1, x2, y2, painter);
  }
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    if (draw_rect)
      draw_rect(data, left, top, right, bottom,
                Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawRect(left, top, right, bottom, painter);
  }
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    if (draw_oval)
      draw_oval(data, left, top, right, bottom,
                Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawOval(left, top, right, bottom, painter);
  }
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override {
    if (draw_circle)
      draw_circle(data, x, y, radius, Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawCircle(x, y, radius, painter);
  }
  void DrawArc(float left, float top, float right, float bottom,
               float start_angle, float sweep_angle, bool use_center,
               tttext::Painter* painter) override {
    if (draw_arc)
      draw_arc(data, left, top, right, bottom, start_angle, sweep_angle,
               use_center, Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawArc(left, top, right, bottom, start_angle, sweep_angle,
                        use_center, painter);
  }
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override {
    if (draw_arc_to)
      draw_arc_to(data, start_x, start_y, mid_x, mid_y, end_x, end_y, radius,
                  Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawArcTo(start_x, start_y, mid_x, mid_y, end_x, end_y, radius,
                          painter);
  }
  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tttext::Painter* painter) override {
    if (draw_round_rect)
      draw_round_rect(data, left, top, right, bottom, radius,
                      Handle<textra_painter_t>(painter));
    else if (fallback)
      fallback->DrawRoundRect(left, top, right, bottom, radius, painter);
  }
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override {
    if (fallback) fallback->DrawImage(src, left, top, right, bottom, painter);
  }
  std::unique_ptr<tttext::Painter> CreatePainter() override {
    if (create_painter)
      return std::unique_ptr<tttext::Painter>(
          Raw<tttext::Painter>(create_painter(data)));
    if (fallback) return fallback->CreatePainter();
    return std::make_unique<tttext::Painter>();
  }
  void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                uint32_t bytes, float x, float y,
                tttext::Painter* painter) override {
    if (draw_text) {
      textra_typeface_helper_t borrowed(Borrow(font));
      draw_text(data, &borrowed, text, bytes, x, y,
                Handle<textra_painter_t>(painter));
    } else if (fallback)
      fallback->DrawText(font, text, bytes, x, y, painter);
  }
  void DrawGlyphs(const tttext::ITypefaceHelper* font, uint32_t count,
                  const uint16_t* glyphs, const char* text, uint32_t bytes,
                  float ox, float oy, float* x, float* y,
                  tttext::Painter* painter) override {
    if (draw_glyphs) {
      textra_typeface_helper_t borrowed(Borrow(font));
      draw_glyphs(data, &borrowed, count, glyphs, text, bytes, ox, oy, x, y,
                  Handle<textra_painter_t>(painter));
    } else if (fallback)
      fallback->DrawGlyphs(font, count, glyphs, text, bytes, ox, oy, x, y,
                           painter);
  }
  // Deferred C operations still forward native calls to the fallback backend.
  void DrawPath(tttext::Path* path, tttext::Painter* painter) override {
    if (fallback) fallback->DrawPath(path, painter);
  }
  void DrawRunDelegate(const tttext::RunDelegate* run, float l, float t,
                       float r, float b, tttext::Painter* painter) override {
    if (fallback) fallback->DrawRunDelegate(run, l, t, r, b, painter);
  }
  void DrawBackgroundDelegate(const tttext::RunDelegate* run,
                              tttext::Painter* painter) override {
    if (fallback) fallback->DrawBackgroundDelegate(run, painter);
  }
  void DrawImageRect(const char* src, float sl, float st, float sr, float sb,
                     float dl, float dt, float dr, float db,
                     tttext::Painter* painter, bool percent) override {
    if (fallback)
      fallback->DrawImageRect(src, sl, st, sr, sb, dl, dt, dr, db, painter,
                              percent);
  }
};

class CallbackFontManager final : public tttext::IFontManager,
                                  public CallbackData {
 public:
  using CallbackData::CallbackData;
  std::shared_ptr<tttext::IFontManager> fallback;
  textra_font_manager_count_families_callback_t count_families = nullptr;
  textra_font_manager_match_family_style_callback_t match_family_style =
      nullptr;
  textra_font_manager_match_family_style_character_callback_t
      match_family_style_character = nullptr;
  textra_font_manager_make_from_file_callback_t make_from_file = nullptr;
  textra_font_manager_legacy_make_typeface_callback_t legacy_make_typeface =
      nullptr;
  textra_font_manager_create_typeface_from_platform_font_callback_t
      create_typeface_from_platform_font = nullptr;
  textra_font_manager_get_platform_font_from_typeface_callback_t
      get_platform_font_from_typeface = nullptr;
  textra_font_manager_match_generic_family_callback_t match_generic_family =
      nullptr;
  textra_font_manager_match_family_style_literal_callback_t
      match_family_style_literal = nullptr;
  textra_font_manager_legacy_make_typeface_literal_callback_t
      legacy_make_typeface_literal = nullptr;

  int countFamilies() const override {
    if (count_families) return count_families(data);
    return fallback ? fallback->countFamilies() : 0;
  }
  TypefaceRef matchFamilyStyle(const char* family,
                               const tttext::FontStyle& style) override {
    if (match_family_style)
      return TakeTypeface(match_family_style(data, family, FontStyle(style)));
    return fallback ? fallback->matchFamilyStyle(family, style) : nullptr;
  }
  TypefaceRef matchFamilyStyleCharacter(const char* family,
                                        const tttext::FontStyle& style,
                                        const char* locales[], int count,
                                        uint32_t character) override {
    if (match_family_style_character)
      return TakeTypeface(match_family_style_character(
          data, family, FontStyle(style), locales, count, character));
    return fallback ? fallback->matchFamilyStyleCharacter(
                          family, style, locales, count, character)
                    : nullptr;
  }
  TypefaceRef makeFromFile(const char* path, int index) override {
    if (make_from_file) return TakeTypeface(make_from_file(data, path, index));
    return fallback ? fallback->makeFromFile(path, index) : nullptr;
  }
  TypefaceRef legacyMakeTypeface(const char* family,
                                 tttext::FontStyle style) const override {
    if (legacy_make_typeface)
      return TakeTypeface(legacy_make_typeface(data, family, FontStyle(style)));
    return fallback ? fallback->legacyMakeTypeface(family, style) : nullptr;
  }
  TypefaceRef createTypefaceFromPlatformFont(const void* font) override {
    if (create_typeface_from_platform_font)
      return TakeTypeface(create_typeface_from_platform_font(data, font));
    return fallback ? fallback->createTypefaceFromPlatformFont(font) : nullptr;
  }
  void* getPlatformFontFromTypeface(TypefaceRef typeface) override {
    if (get_platform_font_from_typeface) {
      textra_typeface_helper_t borrowed(typeface);
      return get_platform_font_from_typeface(data,
                                             typeface ? &borrowed : nullptr);
    }
    return fallback ? fallback->getPlatformFontFromTypeface(std::move(typeface))
                    : nullptr;
  }
  TypefaceRef matchGenericFamily(const char* family,
                                 const tttext::FontStyle& style) override {
    if (match_generic_family)
      return TakeTypeface(match_generic_family(data, family, FontStyle(style)));
    return fallback ? fallback->matchGenericFamily(family, style) : nullptr;
  }
  TypefaceRef matchFamilyStyleLiteral(const char* family,
                                      const tttext::FontStyle& style) override {
    if (match_family_style_literal)
      return TakeTypeface(
          match_family_style_literal(data, family, FontStyle(style)));
    return fallback
               ? fallback->matchFamilyStyleLiteral(family, style)
               : tttext::IFontManager::matchFamilyStyleLiteral(family, style);
  }
  TypefaceRef legacyMakeTypefaceLiteral(
      const char* family, tttext::FontStyle style) const override {
    if (legacy_make_typeface_literal)
      return TakeTypeface(
          legacy_make_typeface_literal(data, family, FontStyle(style)));
    return fallback
               ? fallback->legacyMakeTypefaceLiteral(family, style)
               : tttext::IFontManager::legacyMakeTypefaceLiteral(family, style);
  }
};

class CallbackTypeface final : public tttext::ITypefaceHelper,
                               public CallbackData {
 public:
  CallbackTypeface(uint32_t id, textra_font_style_t style, void* data,
                   textra_destroy_callback_t destroy)
      : ITypefaceHelper(id), CallbackData(data, destroy) {
    font_style_ = tttext::FontStyle(style.weight, style.width, style.slant);
  }
  std::shared_ptr<tttext::ITypefaceHelper> fallback;
  textra_typeface_helper_get_horizontal_advance_callback_t
      get_horizontal_advance = nullptr;
  textra_typeface_helper_get_horizontal_advances_callback_t
      get_horizontal_advances = nullptr;
  textra_typeface_helper_get_width_bound_callback_t get_width_bound = nullptr;
  textra_typeface_helper_get_width_bounds_callback_t get_width_bounds = nullptr;
  textra_typeface_helper_unichar_to_glyph_callback_t unichar_to_glyph = nullptr;
  textra_typeface_helper_unichars_to_glyphs_callback_t unichars_to_glyphs =
      nullptr;
  textra_typeface_helper_get_font_data_callback_t get_font_data = nullptr;
  textra_typeface_helper_get_font_data_size_callback_t get_font_data_size =
      nullptr;
  textra_typeface_helper_get_font_index_callback_t get_font_index = nullptr;
  textra_typeface_helper_get_units_per_em_callback_t get_units_per_em = nullptr;
  textra_typeface_helper_create_font_info_callback_t create_font_info = nullptr;
  textra_typeface_helper_create_font_em_height_callback_t
      create_font_em_height = nullptr;

  void InvalidateMetrics() {
    std::scoped_lock lock(font_info_cache_lock_, font_em_height_cache_lock_);
    font_info_cache_.clear();
    font_em_height_cache_.clear();
  }
  float GetHorizontalAdvance(tttext::GlyphID glyph, float size) const override {
    if (get_horizontal_advance)
      return get_horizontal_advance(data, glyph, size);
    return fallback ? fallback->GetHorizontalAdvance(glyph, size) : 0;
  }
  void GetHorizontalAdvances(tttext::GlyphID glyphs[], uint32_t count,
                             float widths[], float size) const override {
    if (get_horizontal_advances)
      get_horizontal_advances(data, glyphs, count, widths, size);
    else if (fallback)
      fallback->GetHorizontalAdvances(glyphs, count, widths, size);
    else
      std::fill_n(widths, count, 0.f);
  }
  void GetWidthBound(float* rect, tttext::GlyphID glyph,
                     float size) const override {
    if (get_width_bound) {
      textra_rect_t r{};
      get_width_bound(data, glyph, size, &r);
      rect[0] = r.left;
      rect[1] = r.top;
      rect[2] = r.width;
      rect[3] = r.height;
    } else if (fallback)
      fallback->GetWidthBound(rect, glyph, size);
    else
      std::fill_n(rect, 4, 0.f);
  }
  void GetWidthBounds(float* rect, tttext::GlyphID glyphs[], uint32_t count,
                      float size) override {
    if (get_width_bounds) {
      textra_bounds_t r{};
      get_width_bounds(data, glyphs, count, size, &r);
      rect[0] = r.left;
      rect[1] = r.top;
      rect[2] = r.right;
      rect[3] = r.bottom;
    } else if (fallback)
      fallback->GetWidthBounds(rect, glyphs, count, size);
    else
      std::fill_n(rect, 4, 0.f);
  }
  const void* GetFontData() const override {
    if (get_font_data) return get_font_data(data);
    return fallback ? fallback->GetFontData() : nullptr;
  }
  size_t GetFontDataSize() const override {
    if (get_font_data_size) return get_font_data_size(data);
    return fallback ? fallback->GetFontDataSize() : 0;
  }
  int GetFontIndex() const override {
    if (get_font_index) return get_font_index(data);
    return fallback ? fallback->GetFontIndex() : 0;
  }
  uint16_t UnicharToGlyph(tttext::Unichar codepoint,
                          uint32_t selector) const override {
    if (unichar_to_glyph) return unichar_to_glyph(data, codepoint, selector);
    return fallback ? fallback->UnicharToGlyph(codepoint, selector) : 0;
  }
  void UnicharsToGlyphs(const tttext::Unichar* chars, uint32_t count,
                        tttext::GlyphID* glyphs) const override {
    if (unichars_to_glyphs)
      unichars_to_glyphs(data, chars, count, glyphs);
    else if (fallback)
      fallback->UnicharsToGlyphs(chars, count, glyphs);
    else
      std::fill_n(glyphs, count, 0);
  }
  uint32_t GetUnitsPerEm() const override {
    if (get_units_per_em) return get_units_per_em(data);
    return fallback ? fallback->GetUnitsPerEm() : 0;
  }

 protected:
  void OnCreateFontInfo(tttext::FontInfo* info, float size) const override {
    info->SetFontSize(size);
    if (create_font_info)
      create_font_info(data, Handle<textra_font_info_t>(info), size);
    else if (fallback)
      *info = fallback->GetFontInfo(size);
  }
  void OnCreateFontEmHeight(tttext::FontEmHeight* info,
                            float size) const override {
    if (create_font_em_height)
      create_font_em_height(data, Handle<textra_font_em_height_t>(info), size);
    else if (fallback)
      *info = fallback->GetFontEmHeight(size);
    else
      tttext::ITypefaceHelper::OnCreateFontEmHeight(info, size);
  }
};
}  // namespace

textra_status_t TEXTRA_CALL
textra_canvas_helper_create(void* data, textra_destroy_callback_t destroy,
                            textra_canvas_helper_t** out) noexcept {
  *out = Handle<textra_canvas_helper_t>(
      static_cast<tttext::ICanvasHelper*>(new CallbackCanvas(data, destroy)));
  return TEXTRA_STATUS_OK;
}
void TEXTRA_CALL textra_canvas_helper_set_fallback(
    textra_canvas_helper_t* value, textra_canvas_helper_t* fallback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->fallback =
      Raw<tttext::ICanvasHelper>(fallback);
}
textra_status_t TEXTRA_CALL
textra_font_manager_create(void* data, textra_destroy_callback_t destroy,
                           textra_font_manager_t** out) noexcept {
  *out = Share<textra_font_manager_t>(
      std::make_shared<CallbackFontManager>(data, destroy));
  return TEXTRA_STATUS_OK;
}
void TEXTRA_CALL textra_font_manager_set_fallback(
    textra_font_manager_t* value, textra_font_manager_t* fallback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->fallback =
      fallback ? fallback->value : nullptr;
}
textra_status_t TEXTRA_CALL
textra_typeface_helper_create(uint32_t id, textra_font_style_t style,
                              void* data, textra_destroy_callback_t destroy,
                              textra_typeface_helper_t** out) noexcept {
  *out = Share<textra_typeface_helper_t>(
      std::make_shared<CallbackTypeface>(id, style, data, destroy));
  return TEXTRA_STATUS_OK;
}
void TEXTRA_CALL textra_typeface_helper_set_fallback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_t* fallback) noexcept {
  auto* adapter = static_cast<CallbackTypeface*>(value->value.get());
  adapter->fallback = fallback ? fallback->value : nullptr;
  adapter->InvalidateMetrics();
}

void TEXTRA_CALL textra_canvas_helper_set_start_paint_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_start_paint_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->start_paint =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_end_paint_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_end_paint_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->end_paint =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_save_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_save_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->save =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_restore_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_restore_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->restore =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_translate_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_translate_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->translate =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_scale_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_scale_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->scale =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_rotate_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_rotate_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->rotate =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_skew_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_skew_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->skew =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_clip_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clip_rect_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->clip_rect =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_clear_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clear_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->clear =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_clear_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_clear_rect_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->clear_rect =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_fill_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_fill_rect_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->fill_rect =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_color_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_color_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_color =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_line_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_line_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_line =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_rect_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_rect =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_oval_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_oval_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_oval =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_circle_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_circle_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_circle =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_arc_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_arc_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_arc =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_arc_to_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_arc_to_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_arc_to =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_round_rect_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_round_rect_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))
      ->draw_round_rect = callback;
}

void TEXTRA_CALL textra_canvas_helper_set_create_painter_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_create_painter_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))
      ->create_painter = callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_text_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_text_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_text =
      callback;
}

void TEXTRA_CALL textra_canvas_helper_set_draw_glyphs_callback(
    textra_canvas_helper_t* value,
    textra_canvas_helper_draw_glyphs_callback_t callback) noexcept {
  static_cast<CallbackCanvas*>(Raw<tttext::ICanvasHelper>(value))->draw_glyphs =
      callback;
}

void TEXTRA_CALL textra_font_manager_set_count_families_callback(
    textra_font_manager_t* value,
    textra_font_manager_count_families_callback_t callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->count_families =
      callback;
}

void TEXTRA_CALL textra_font_manager_set_match_family_style_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_callback_t callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->match_family_style =
      callback;
}

void TEXTRA_CALL textra_font_manager_set_match_family_style_character_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_character_callback_t
        callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())
      ->match_family_style_character = callback;
}

void TEXTRA_CALL textra_font_manager_set_make_from_file_callback(
    textra_font_manager_t* value,
    textra_font_manager_make_from_file_callback_t callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->make_from_file =
      callback;
}

void TEXTRA_CALL textra_font_manager_set_legacy_make_typeface_callback(
    textra_font_manager_t* value,
    textra_font_manager_legacy_make_typeface_callback_t callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->legacy_make_typeface =
      callback;
}

void TEXTRA_CALL
textra_font_manager_set_create_typeface_from_platform_font_callback(
    textra_font_manager_t* value,
    textra_font_manager_create_typeface_from_platform_font_callback_t
        callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())
      ->create_typeface_from_platform_font = callback;
}

void TEXTRA_CALL
textra_font_manager_set_get_platform_font_from_typeface_callback(
    textra_font_manager_t* value,
    textra_font_manager_get_platform_font_from_typeface_callback_t
        callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())
      ->get_platform_font_from_typeface = callback;
}

void TEXTRA_CALL textra_font_manager_set_match_generic_family_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_generic_family_callback_t callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())->match_generic_family =
      callback;
}

void TEXTRA_CALL textra_font_manager_set_match_family_style_literal_callback(
    textra_font_manager_t* value,
    textra_font_manager_match_family_style_literal_callback_t
        callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())
      ->match_family_style_literal = callback;
}

void TEXTRA_CALL textra_font_manager_set_legacy_make_typeface_literal_callback(
    textra_font_manager_t* value,
    textra_font_manager_legacy_make_typeface_literal_callback_t
        callback) noexcept {
  static_cast<CallbackFontManager*>(value->value.get())
      ->legacy_make_typeface_literal = callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_horizontal_advance_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_horizontal_advance_callback_t
        callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_horizontal_advance =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_horizontal_advances_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_horizontal_advances_callback_t
        callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_horizontal_advances =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_width_bound_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_width_bound_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_width_bound =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_width_bounds_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_width_bounds_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_width_bounds =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_unichar_to_glyph_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_unichar_to_glyph_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->unichar_to_glyph =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_unichars_to_glyphs_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_unichars_to_glyphs_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->unichars_to_glyphs =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_font_data_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_data_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_font_data = callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_font_data_size_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_data_size_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_font_data_size =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_font_index_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_font_index_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_font_index = callback;
}

void TEXTRA_CALL textra_typeface_helper_set_get_units_per_em_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_get_units_per_em_callback_t callback) noexcept {
  static_cast<CallbackTypeface*>(value->value.get())->get_units_per_em =
      callback;
}

void TEXTRA_CALL textra_typeface_helper_set_create_font_info_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_create_font_info_callback_t callback) noexcept {
  auto* adapter = static_cast<CallbackTypeface*>(value->value.get());
  adapter->create_font_info = callback;
  adapter->InvalidateMetrics();
}

void TEXTRA_CALL textra_typeface_helper_set_create_font_em_height_callback(
    textra_typeface_helper_t* value,
    textra_typeface_helper_create_font_em_height_callback_t callback) noexcept {
  auto* adapter = static_cast<CallbackTypeface*>(value->value.get());
  adapter->create_font_em_height = callback;
  adapter->InvalidateMetrics();
}

void TEXTRA_CALL textra_font_info_set_top(textra_font_info_t* value,
                                          float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetTop(input);
}

void TEXTRA_CALL textra_font_info_set_ascent(textra_font_info_t* value,
                                             float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetAscent(input);
}

void TEXTRA_CALL textra_font_info_set_descent(textra_font_info_t* value,
                                              float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetDescent(input);
}

void TEXTRA_CALL textra_font_info_set_bottom(textra_font_info_t* value,
                                             float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetBottom(input);
}

void TEXTRA_CALL textra_font_info_set_leading(textra_font_info_t* value,
                                              float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetLeading(input);
}

void TEXTRA_CALL textra_font_info_set_font_size(textra_font_info_t* value,
                                                float input) noexcept {
  Raw<tttext::FontInfo>(value)->SetFontSize(input);
}

void TEXTRA_CALL textra_font_em_height_set_ascent(
    textra_font_em_height_t* value, float input) noexcept {
  Raw<tttext::FontEmHeight>(value)->SetAscent(input);
}

void TEXTRA_CALL textra_font_em_height_set_descent(
    textra_font_em_height_t* value, float input) noexcept {
  Raw<tttext::FontEmHeight>(value)->SetDescent(input);
}
