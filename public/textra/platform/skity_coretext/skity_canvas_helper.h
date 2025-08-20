// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_CANVAS_HELPER_H_

#include <textra/i_canvas_helper.h>
#include <textra/painter.h>
#include <textra/platform/skity_coretext/skity_typeface_helper.h>

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/render/canvas.hpp>
#include <skity/text/font.hpp>
namespace ttoffice {
namespace tttext {
/**
 * @brief A canvas helper implementation backed by the Skity library.
 */
class SkityCanvasHelper : public ICanvasHelper {
 public:
  explicit SkityCanvasHelper(skity::Canvas* canvas) : canvas_(canvas) {}

 public:
  std::unique_ptr<Painter> CreatePainter() override {
    auto painter = std::make_unique<SkityPainter>();
    painter->platform_painter_ = std::make_unique<skity::Paint>();
    return painter;
  }
  void StartPaint() override {}
  void EndPaint() override {}
  void Save() override { canvas_->Save(); }
  void Restore() override { canvas_->Restore(); }
  void Translate(float dx, float dy) override { canvas_->Translate(dx, dy); }
  void Scale(float sx, float sy) override { canvas_->Scale(sx, sy); }
  void Rotate(float degrees) override { canvas_->Rotate(degrees); }
  void Skew(float sx, float sy) override { canvas_->Skew(sx, sy); }
  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    canvas_->ClipRect(skity::Rect::MakeLTRB(left, top, right, bottom));
  }
  void Clear() override {}
  void ClearRect(float left, float top, float right, float bottom) override {}
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {}
  void DrawColor(uint32_t color) override { canvas_->DrawColor(color); }
  void DrawLine(float x1, float y1, float x2, float y2,
                TTPainter* painter) override {}
  void DrawRect(float left, float top, float right, float bottom,
                TTPainter* painter) override {
    // canvas_->DrawRect(skity::Rect::MakeLTRB(left, top, right, bottom),
    // paint_);
  }
  void DrawOval(float left, float top, float right, float bottom,
                TTPainter* painter) override {}
  void DrawCircle(float x, float y, float radius, TTPainter* painter) override {
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               TTPainter* painter) override {}
  void DrawPath(Path* path, TTPainter* painter) override {}
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 TTPainter* painter) override {}
  void DrawText(const TTTypeface* font, const char* text, uint32_t text_bytes,
                float x, float y, TTPainter* painter) override {}
  void DrawGlyphs(const TTTypeface* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float* x, float y, ttoffice::Painter* painter) override {
    const auto* skity_typeface_helper =
        (skity::textlayout::SkityTypefaceHelper*)font;
    skity::Font skity_font(skity_typeface_helper->GetTypeface(),
                           painter->GetTextSize());
    float* py = new float[glyph_count];
    auto* glyph_id = new skity::GlyphID[glyph_count];
    for (uint32_t k = 0; k < glyph_count; k++) {
      glyph_id[k] = glyphs[k];
      py[k] = y;
    }
    skity::Paint* paint = ToSkityPaint(painter);
    canvas_->DrawGlyphs(glyph_count, glyph_id, x, py, skity_font, *paint);
    delete[] glyph_id;
    delete[] py;
  }
  void DrawRunDelegate(const RunDelegate* delegate, float left, float top,
                       float right, float bottom, TTPainter* painter) override {
  }
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              TTPainter* painter) override {}
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, TTPainter* painter) override {}
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     TTPainter* painter, bool srcRectPercent) override {}
  skity::Paint* ToSkityPaint(TTPainter* painter) {
    auto* p = reinterpret_cast<SkityPainter*>(painter);
    if (p && p->platform_painter_) {
      return p->platform_painter_.get();
    }
    paint_.SetStrokeWidth(painter->GetStrokeWidth());
    paint_.SetTextSize(painter->GetTextSize());
    paint_.SetFillColor(painter->GetRed(), painter->GetGreen(),
                        painter->GetBlue(), painter->GetAlpha());
    return &paint_;
  }

 private:
  skity::Canvas* canvas_;
  skity::Paint paint_{};
};

class SkityPainter : public TTPainter {
 public:
  std::unique_ptr<skity::Paint> platform_painter_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_CORETEXT_SKITY_CANVAS_HELPER_H_
