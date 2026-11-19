// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef SRC_CAPI_TEXTRA_CANVAS_PROXY_H_
#define SRC_CAPI_TEXTRA_CANVAS_PROXY_H_
#include <memory>

#include "src/capi/textra_internal.h"
namespace textra_capi {
class CanvasProxy final : public tt::ICanvasHelper {
 public:
  tt::ICanvasHelper* target;
  explicit CanvasProxy(tt::ICanvasHelper* p) : target(p) {}
  std::unique_ptr<tt::Painter> CreatePainter() override {
    // LayoutDrawer still configures painters after a callback reports an
    // error. Suppress drawing below, while preserving its value-object needs.
    return target->CreatePainter();
  }
  void StartPaint() override {
    if (operation && operation->error) return;
    target->StartPaint();
  }
  void EndPaint() override { target->EndPaint(); }
  void Save() override {
    if (operation && operation->error) return;
    target->Save();
  }
  void Restore() override { target->Restore(); }
  void Translate(float dx, float dy) override {
    if (operation && operation->error) return;
    target->Translate(dx, dy);
  }
  void Scale(float sx, float sy) override {
    if (operation && operation->error) return;
    target->Scale(sx, sy);
  }
  void Rotate(float degrees) override {
    if (operation && operation->error) return;
    target->Rotate(degrees);
  }
  void Skew(float sx, float sy) override {
    if (operation && operation->error) return;
    target->Skew(sx, sy);
  }
  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    if (operation && operation->error) return;
    target->ClipRect(left, top, right, bottom, doAntiAlias);
  }
  void Clear() override {
    if (operation && operation->error) return;
    target->Clear();
  }
  void ClearRect(float left, float top, float right, float bottom) override {
    if (operation && operation->error) return;
    target->ClearRect(left, top, right, bottom);
  }
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {
    if (operation && operation->error) return;
    target->FillRect(left, top, right, bottom, color);
  }
  void DrawColor(uint32_t color) override {
    if (operation && operation->error) return;
    target->DrawColor(color);
  }
  void DrawLine(float x1, float y1, float x2, float y2,
                tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawLine(x1, y1, x2, y2, painter);
  }
  void DrawRect(float left, float top, float right, float bottom,
                tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawRect(left, top, right, bottom, painter);
  }
  void DrawOval(float left, float top, float right, float bottom,
                tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawOval(left, top, right, bottom, painter);
  }
  void DrawCircle(float x, float y, float radius,
                  tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawCircle(x, y, radius, painter);
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawArc(left, top, right, bottom, startAngle, sweepAngle, useCenter,
                    painter);
  }
  void DrawPath(tt::Path* path, tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawPath(path, painter);
  }
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawArcTo(start_x, start_y, mid_x, mid_y, end_x, end_y, radius,
                      painter);
  }
  void DrawText(const tt::ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawText(font, text, text_bytes, x, y, painter);
  }
  void DrawGlyphs(const tt::ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float origin_x, float origin_y, float* x, float* y,
                  tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawGlyphs(font, glyph_count, glyphs, text, text_bytes, origin_x,
                       origin_y, x, y, painter);
  }
  void DrawRunDelegate(const tt::RunDelegate* delegate, float left, float top,
                       float right, float bottom,
                       tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawRunDelegate(delegate, left, top, right, bottom, painter);
  }
  void DrawBackgroundDelegate(const tt::RunDelegate* delegate,
                              tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawBackgroundDelegate(delegate, painter);
  }
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawImage(src, left, top, right, bottom, painter);
  }
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tt::Painter* painter, bool srcRectPercent) override {
    if (operation && operation->error) return;
    target->DrawImageRect(src, src_left, src_top, src_right, src_bottom,
                          dst_left, dst_top, dst_right, dst_bottom, painter,
                          srcRectPercent);
  }

  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tt::Painter* painter) override {
    if (operation && operation->error) return;
    target->DrawRoundRect(left, top, right, bottom, radius, painter);
  }
};
}  // namespace textra_capi
#endif  // SRC_CAPI_TEXTRA_CANVAS_PROXY_H_
