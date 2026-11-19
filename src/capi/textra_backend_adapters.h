// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef SRC_CAPI_TEXTRA_BACKEND_ADAPTERS_H_
#define SRC_CAPI_TEXTRA_BACKEND_ADAPTERS_H_

#include <textra/capi/textra_platform.h>
#include <textra/i_canvas_helper.h>
#include <textra/i_font_manager.h>

#include <memory>
#include <mutex>

namespace ttoffice {
namespace tttext {
namespace backend {
// Private adapters invoke per-instance callbacks. C handles own
// these types; the engine receives their existing I* base interfaces.
class CallbackTypeface final : public ITypefaceHelper {
 public:
  CallbackTypeface(const textra_backend_typeface_api_t& callbacks,
                   void* userdata, void* native_font, uint32_t id,
                   bool native_tokens);
  ~CallbackTypeface() override;
  void* NativeObject() const;
  void* PlatformFont() const;
  float GetHorizontalAdvance(GlyphID g, float z) const override;
  void GetHorizontalAdvances(GlyphID* g, uint32_t n, float* v,
                             float z) const override;
  void GetWidthBound(float* v, GlyphID g, float z) const override;
  void GetWidthBounds(float* v, GlyphID* g, uint32_t n, float z) override;
  const void* GetFontData() const override;
  size_t GetFontDataSize() const override;
  int GetFontIndex() const override;
  uint16_t UnicharToGlyph(Unichar c, uint32_t s) const override;
  void UnicharsToGlyphs(const Unichar* c, uint32_t n,
                        GlyphID* v) const override;
  uint32_t GetUnitsPerEm() const override;
  const void* Native() const;

 protected:
  void OnCreateFontInfo(FontInfo* out, float z) const override;
  void OnCreateFontEmHeight(FontEmHeight* out, float z) const override;

 private:
  textra_backend_typeface_api_t callbacks_;
  void* userdata_;
  void* native_font_;
  bool native_tokens_;
  void* Userdata() const;
  mutable std::mutex data_mutex_;
  mutable bool data_loaded_ = false;
  mutable bool data_size_loaded_ = false;
  mutable const void* data_ = nullptr;
  mutable size_t data_size_ = 0;
};

class CallbackFontManager final : public IFontManager {
 public:
  explicit CallbackFontManager(const textra_font_manager_options_t& options,
                               RenderingBackend backend);
  void SetOwnsManager(bool owns) { owns_manager_ = owns; }
  ~CallbackFontManager() override;
  CallbackFontManager(const CallbackFontManager&) = delete;
  CallbackFontManager& operator=(const CallbackFontManager&) = delete;
  int countFamilies() const override;

  TypefaceRef matchFamilyStyle(const char* s,
                               const tttext::FontStyle& f) override;
  TypefaceRef matchGenericFamily(const char* s,
                                 const tttext::FontStyle& f) override;
  TypefaceRef matchFamilyStyleLiteral(const char* s,
                                      const tttext::FontStyle& f) override;
  TypefaceRef matchFamilyStyleCharacter(const char* s,
                                        const tttext::FontStyle& f,
                                        const char** l, int n,
                                        uint32_t c) override;
  TypefaceRef makeFromFile(const char* s, int i) override;
  TypefaceRef legacyMakeTypeface(const char* s,
                                 tttext::FontStyle f) const override;
  TypefaceRef legacyMakeTypefaceLiteral(const char* s,
                                        tttext::FontStyle f) const override;
  TypefaceRef createTypefaceFromPlatformFont(const void* p) override;
  void* getPlatformFontFromTypeface(TypefaceRef t) override;

 private:
  std::shared_ptr<CallbackTypeface> AdoptTypeface(void* face,
                                                  textra_status_t status) const;
  textra_backend_font_manager_api_t callbacks_{};
  textra_backend_typeface_api_t typeface_callbacks_{};
  bool native_tokens_;
  void* userdata_;
  void* native_manager_;
  bool owns_manager_ = true;
};

class CallbackCanvas final : public ICanvasHelper {
 public:
  explicit CallbackCanvas(const textra_canvas_options_t& options);
  ~CallbackCanvas() override;
  CallbackCanvas(const CallbackCanvas&) = delete;
  CallbackCanvas& operator=(const CallbackCanvas&) = delete;
  const textra_backend_canvas_api_t& Callbacks() const { return callbacks_; }
  void* Userdata() const { return userdata_; }
  void* NativeObject() const { return native_canvas_; }
  std::unique_ptr<Painter> CreatePainter() override;
  void StartPaint() override;
  void EndPaint() override;
  void Save() override;
  void Restore() override;
  void Translate(float dx, float dy) override;
  void Scale(float sx, float sy) override;
  void Rotate(float degrees) override;
  void Skew(float sx, float sy) override;
  void ClipRect(float left, float top, float right, float bottom,
                bool antialias) override;
  void Clear() override;
  void ClearRect(float left, float top, float right, float bottom) override;
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override;
  void DrawColor(uint32_t color) override;
  void DrawLine(float x1, float y1, float x2, float y2,
                Painter* painter) override;
  void DrawRect(float left, float top, float right, float bottom,
                Painter* painter) override;
  void DrawOval(float left, float top, float right, float bottom,
                Painter* painter) override;
  void DrawCircle(float x, float y, float radius, Painter* painter) override;
  void DrawArc(float left, float top, float right, float bottom,
               float start_angle, float sweep_angle, bool use_center,
               Painter* painter) override;
  void DrawPath(Path* path, Painter* painter) override;
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 Painter* painter) override;
  void DrawText(const ITypefaceHelper* typeface, const char* text,
                uint32_t text_bytes, float x, float y,
                Painter* painter) override;
  void DrawGlyphs(const ITypefaceHelper* typeface, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float origin_x, float origin_y, float* x, float* y,
                  Painter* painter) override;
  // Required by ICanvasHelper; not exposed through the C callback table.
  void DrawRunDelegate(const RunDelegate*, float, float, float, float,
                       Painter*) override {}
  void DrawBackgroundDelegate(const RunDelegate*, Painter*) override {}
  void DrawImage(const char* source, float left, float top, float right,
                 float bottom, Painter* painter) override;
  void DrawImageRect(const char* source, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     Painter* painter, bool source_rect_percent) override;
  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, Painter* painter) override;

 private:
  textra_backend_canvas_api_t callbacks_{};
  void* userdata_;
  void* native_canvas_;
};

std::shared_ptr<CallbackTypeface> CreateTypeface(
    const textra_typeface_options_t& options);
}  // namespace backend
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_CAPI_TEXTRA_BACKEND_ADAPTERS_H_
