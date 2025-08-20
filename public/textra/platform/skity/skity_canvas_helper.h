// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_CANVAS_HELPER_H_

#include <textra/i_canvas_helper.h>
#include <textra/painter.h>
#include <textra/platform/skity/skity_typeface_helper.h>

#include <memory>
#include <skity/effect/mask_filter.hpp>
#include <skity/graphic/paint.hpp>
#include <skity/render/canvas.hpp>
#include <skity/text/font.hpp>
#include <skity/text/text_blob.hpp>
#include <skity/text/text_run.hpp>
#include <vector>
namespace ttoffice {
namespace tttext {
class SkityPainter : public tttext::Painter {
 public:
  std::unique_ptr<skity::Paint> platform_painter_;
};
/**
 * @brief A canvas helper implementation backed by the Skity library.
 */
class SkityCanvasHelper : public ICanvasHelper {
 public:
  explicit SkityCanvasHelper(skity::Canvas* canvas) : canvas_(canvas) {}

 public:
  std::unique_ptr<Painter> CreatePainter() override {
    auto painter = std::make_unique<SkityPainter>();
    // painter->platform_painter_ = std::make_unique<skity::Paint>();
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
                tttext::Painter* painter) override {
    skity::Paint* paint = ToSkityPaint(painter);
    canvas_->DrawLine(x1, y1, x2, y2, *paint);
  }
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    skity::Paint* paint = ToSkityPaint(painter);
    canvas_->DrawRect(skity::Rect::MakeLTRB(left, top, right, bottom), *paint);
  }
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {}
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override {}
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter) override {}
  void DrawPath(Path* path, tttext::Painter* painter) override {}
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override {}
  void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                tttext::Painter* painter) override {}
  void DrawGlyphs(const tttext::ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float ox, float oy, float* pos_x, float* pos_y,
                  tttext::Painter* painter) override {
    const auto* typeface =
        reinterpret_cast<const skity::textlayout::SkityTypefaceHelper*>(font);
    skity::Paint* paint = ToSkityPaint(painter);

    std::vector<skity::GlyphID> glyph_vector;
    std::vector<float> p_x;
    std::vector<float> p_y;
    for (uint32_t index = 0; index < glyph_count; index++) {
      glyph_vector.push_back(glyphs[index]);
      p_x.push_back(pos_x[index]);
      p_y.push_back(-pos_y[index]);
    }
    std::vector<skity::TextRun> runs;
    skity::Font skity_font(typeface->GetTypeface(), paint->GetTextSize());
    skity_font.SetEmbolden(painter->IsBold());
    skity_font.SetSkewX(painter->IsItalic() ? -0.25f : 0);
    runs.emplace_back(skity_font, glyph_vector, p_x, p_y);

    std::shared_ptr<skity::TextBlob> text_blob =
        std::make_shared<skity::TextBlob>(runs);

    const auto& shadow_list = painter->GetShadowList();
    if (!shadow_list.empty()) {
      for (auto& shadow : shadow_list) {
        paint->SetColor(shadow.color_.GetPlainColor());
        if (shadow.blur_radius_ != 0) {
          std::shared_ptr<skity::MaskFilter> filter =
              skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal,
                                          shadow.blur_radius_);
          paint->SetMaskFilter(filter);
        }
        canvas_->DrawTextBlob(text_blob, ox, oy, *paint);
      }
    }

    canvas_->DrawTextBlob(text_blob, ox, oy, *paint);
  }
  void DrawRunDelegate(const RunDelegate* delegate, float left, float top,
                       float right, float bottom,
                       tttext::Painter* painter) override {}
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              tttext::Painter* painter) override {}
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override {}
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tttext::Painter* painter, bool srcRectPercent) override {}
  skity::Paint* ToSkityPaint(tttext::Painter* painter) {
    auto* p = reinterpret_cast<SkityPainter*>(painter);
    if (p && p->platform_painter_) {
      p->platform_painter_->SetTextSize(painter->GetTextSize());
      return p->platform_painter_.get();
    }
    paint_.Reset();
    paint_.SetStrokeWidth(painter->GetStrokeWidth());
    paint_.SetTextSize(painter->GetTextSize());
    paint_.SetColor(painter->GetColor());
    switch (painter->GetFillStyle()) {
      case FillStyle::kFill:
        paint_.SetStyle(skity::Paint::kFill_Style);
        break;
      case FillStyle::kStroke:
        paint_.SetStyle(skity::Paint::kStroke_Style);
        break;
      case FillStyle::kStrokeAndFill:
        paint_.SetStyle(skity::Paint::kStrokeAndFill_Style);
        break;
      default:
        paint_.SetStyle(skity::Paint::kFill_Style);
    }
    return &paint_;
  }

 private:
  skity::Canvas* canvas_;
  skity::Paint paint_{};
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_PLATFORM_SKITY_SKITY_CANVAS_HELPER_H_
