// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_CANVAS_HELPER_H_
#include <lvgl/src/misc/lv_matrix.h>
#include <textra/i_canvas_helper.h>

#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class L_EXPORT LVGLCanvasHelper : public ICanvasHelper {
 public:
  explicit LVGLCanvasHelper(lv_layer_t* layer);
  ~LVGLCanvasHelper() override;

 public:
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
                bool doAntiAlias) override;
  void Clear() override;
  void ClearRect(float left, float top, float right, float bottom) override;
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override;
  void DrawColor(uint32_t color) override;
  void DrawLine(float x1, float y1, float x2, float y2,
                tttext::Painter* painter) override;
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override;
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override;
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override;
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter) override;
  void DrawPath(Path* path, tttext::Painter* painter) override;
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override;
  void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                tttext::Painter* painter) override;
  void DrawGlyphs(const tttext::ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float origin_x, float origin_y, float* x, float* y,
                  Painter* painter) override;
  void DrawRunDelegate(const RunDelegate* delegate, float left, float top,
                       float right, float bottom,
                       tttext::Painter* painter) override;
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              tttext::Painter* painter) override;
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override;
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tttext::Painter* painter, bool srcRectPercent) override;
  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tttext::Painter* painter) override;

 protected:
  lv_fpoint_t TransformPoint(float x, float y);
  lv_area_t TransformRectToArea(float left, float top, float right,
                                float bottom);
  void ApplyMatrixToLayer();

  lv_layer_t* layer_ = nullptr;
  lv_matrix_t current_;
  std::vector<lv_matrix_t> stack_;
  std::vector<lv_area_t> clip_stack_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_LVGL_LVGL_CANVAS_HELPER_H_
