// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_CANVAS_HELPER_H_

#include <textra/i_canvas_helper.h>
#include <textra/layout_drawer_listener.h>
#include <textra/macro.h>
#include <textra/platform/skia/skia_painter.h>
#include <textra/platform/skia/skia_typeface_helper.h>

#include <memory>

#include "include/core/SkBlurTypes.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"

class SkCanvas;
class SkPath;
/**
 * @brief A canvas helper implementation backed by the Skia library.
 *
 * This class is mostly a straightforward pass-through to the SkCanvas API.
 * It is primarily used on macOS.
 */
class L_EXPORT SkiaCanvasHelper : public tttext::ICanvasHelper {
 public:
  explicit SkiaCanvasHelper(SkCanvas* canvas) : canvas_(canvas) {}
  ~SkiaCanvasHelper() override = default;

 public:
  std::unique_ptr<tttext::Painter> CreatePainter() override {
    auto painter = std::make_unique<tttext::SkiaPainter>();
    return painter;
  }
  void StartPaint() override {}
  void EndPaint() override {}
  void Save() override { canvas_->save(); }
  void Restore() override { canvas_->restore(); }
  void Translate(float dx, float dy) override { canvas_->translate(dx, dy); }
  void Scale(float sx, float sy) override { canvas_->scale(sx, sy); }
  void Rotate(float degrees) override { canvas_->rotate(degrees); }
  void Skew(float sx, float sy) override { canvas_->skew(sx, sy); }
  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    canvas_->clipRect({left, top, right, bottom}, SkClipOp::kIntersect, true);
  }
  void Clear() override {}
  void ClearRect(float left, float top, float right, float bottom) override {}
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {
    SkPaint paint;
    paint.setColor(color);
    paint.setStyle(SkPaint::Style::kFill_Style);
    canvas_->drawRect({left, top, right, bottom}, paint);
  }
  void DrawColor(uint32_t color) override { canvas_->clear(color); }
  void DrawLine(float x1, float y1, float x2, float y2,
                tttext::Painter* painter) override {
    canvas_->drawLine({x1, y1}, {x2, y2}, ToSkPaint(painter));
  }
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    canvas_->drawRect({left, top, right, bottom}, ToSkPaint(painter));
  }
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    canvas_->drawOval({left, top, right, bottom}, ToSkPaint(painter));
  }
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override {
    canvas_->drawCircle(x, y, radius, ToSkPaint(painter));
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter) override {
    canvas_->drawArc({left, top, right, bottom}, startAngle, sweepAngle,
                     useCenter, ToSkPaint(painter));
  }
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override {
    SkPath path;
    SkPoint start{start_x, start_y};
    SkPoint mid{mid_x, mid_y};
    SkPoint end{end_x, end_y};
    path.moveTo(start);
    path.arcTo(mid, end, radius);
    canvas_->drawPath(path, ToSkPaint(painter));
  }
  void DrawPath(tttext::Path* path, tttext::Painter* painter) override {
    // SkPath sk_path;
    // switch (path->type_) {
    //   case ttoffice::PathType::kMultiPath: {
    //     auto* multi = reinterpret_cast<ttoffice::MultiPath*>(path);
    //     for (auto& one_path : multi->path_list_) {
    //       AddSubPath(one_path.get(), &sk_path);
    //     }
    //     break;
    //   }
    //   default:
    //     AddSubPath(path, &sk_path);
    //     break;
    // }
    // canvas_->drawPath(sk_path, ToSkPaint(painter));
  }
  void AddSubPath(tttext::Path* path, SkPath* sk_path) {
    // switch (path->type_) {
    //   case ttoffice::PathType::kMoveTo: {
    //     auto* point = reinterpret_cast<ttoffice::MoveToPath*>(path);
    //     sk_path->moveTo(point->point_.x_, point->point_.y_);
    //     break;
    //   }
    //   case ttoffice::PathType::kLines: {
    //     auto* lines = reinterpret_cast<ttoffice::LinesPath*>(path);
    //     for (auto& iter : lines->points_) {
    //       SkPoint point{iter.x_, iter.y_};
    //       sk_path->lineTo(point);
    //     }
    //     break;
    //   }
    //   case ttoffice::PathType::kArc: {
    //     auto* arc = reinterpret_cast<ttoffice::ArcPath*>(path);
    //     sk_path->lineTo(SkPoint{arc->start_.x_, arc->start_.y_});
    //     sk_path->arcTo(SkPoint{arc->mid_.x_, arc->mid_.y_},
    //                    SkPoint{arc->end_.x_, arc->end_.y_}, arc->radius_);
    //     break;
    //   }
    //   case ttoffice::PathType::kBezier: {
    //     auto* bezier = reinterpret_cast<ttoffice::BezierPath*>(path);
    //     if (bezier->points_.size() < 3) break;
    //     if (bezier->points_.size() == 3) {
    //       SkPoint points[]{{bezier->points_[0].x_, bezier->points_[0].y_},
    //                        {bezier->points_[1].x_, bezier->points_[1].y_},
    //                        {bezier->points_[2].x_, bezier->points_[2].y_}};
    //       sk_path->lineTo(points[0]);
    //       sk_path->quadTo(points[1], points[2]);
    //       break;
    //     }
    //     if (bezier->points_.size() == 4) {
    //       SkPoint points[]{{bezier->points_[0].x_, bezier->points_[0].y_},
    //                        {bezier->points_[1].x_, bezier->points_[1].y_},
    //                        {bezier->points_[2].x_, bezier->points_[2].y_},
    //                        {bezier->points_[3].x_, bezier->points_[3].y_}};
    //       sk_path->lineTo(points[0]);
    //       sk_path->cubicTo(points[1], points[2], points[3]);
    //       break;
    //     }
    //   }
    //   case ttoffice::PathType::kMultiPath:
    //   default:
    //     break;
    // }
  }
  void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                tttext::Painter* painter) override {
    auto typeface = reinterpret_cast<const tttext::SkiaTypefaceHelper*>(font);
    SkFont sk_font(typeface->GetSkTypeface(), painter->GetTextSize());
    canvas_->drawSimpleText(text, text_bytes, SkTextEncoding::kUTF8, x, y,
                            sk_font, ToSkPaint(painter));
    if (painter->IsUnderLine()) {
      auto width = sk_font.measureText(text, text_bytes, SkTextEncoding::kUTF8);
      SkFontMetrics sk_font_metrics{};
      sk_font.getMetrics(&sk_font_metrics);
      canvas_->drawLine(x, y + sk_font_metrics.fDescent, x + width,
                        y + sk_font_metrics.fDescent, ToSkPaint(painter));
    }
  }
  void DrawGlyphs(const tttext::ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float origin_x, float origin_y, float* x, float* y,
                  tttext::Painter* painter) override {
    if (glyph_count == 0) {
      return;
    }
    auto typeface = reinterpret_cast<const tttext::SkiaTypefaceHelper*>(font);
    SkFont sk_font(typeface->GetSkTypeface(), painter->GetTextSize(), 1,
                   painter->IsItalic() ? -0.25 : 0);
    SkTextBlobBuilder builder;
    auto run_buffer = builder.allocRunPosH(sk_font, glyph_count, 0, nullptr);
    for (uint32_t k = 0; k < glyph_count; k++) {
      run_buffer.glyphs[k] = tttext::GlyphID(glyphs[k]);
      run_buffer.pos[k] = x[k];
    }
    auto blob = builder.make();
    const auto& shadow_list = painter->GetShadowList();
    if (!shadow_list.empty()) {
      for (auto& shadow : shadow_list) {
        SkPaint sk_paint;
        sk_paint.setColor(shadow.color_.GetPlainColor());
        if (shadow.blur_radius_ != 0) {
          auto filter = SkMaskFilter::MakeBlur(
              kNormal_SkBlurStyle, SkDoubleToScalar(shadow.blur_radius_),
              false);
          sk_paint.setMaskFilter(filter);
        }
        canvas_->drawTextBlob(blob, origin_x, origin_y, sk_paint);
      }
    }

    canvas_->drawTextBlob(blob, origin_x, origin_y, ToSkPaint(painter));
  }
  void DrawRunDelegate(const tttext::RunDelegate* run_delegate, float left,
                       float top, float right, float bottom,
                       tttext::Painter* painter) override {}
  void DrawBackgroundDelegate(const tttext::RunDelegate* delegate,
                              tttext::Painter* painter) override {}
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override {}
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tttext::Painter* painter, bool srcRectPercent) override {}

  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tttext::Painter* painter) override {
    canvas_->drawRoundRect({left, top, right, bottom}, radius, radius,
                           ToSkPaint(painter));
  }

  SkCanvas* GetSkCanvas() const { return canvas_; }

 private:
  SkPaint& ToSkPaint(tttext::Painter* painter) {
    auto* skia_painter = TTDYNAMIC_CAST<tttext::SkiaPainter*>(painter);
    if (skia_painter != nullptr && skia_painter->sk_paint_ != nullptr) {
      return *skia_painter->sk_paint_;
    }
    painter_.setAntiAlias(true);
    switch (painter->GetFillStyle()) {
      case tttext::FillStyle::kFill:
        painter_.setStyle(SkPaint::kFill_Style);
        break;
      case tttext::FillStyle::kStroke:
        painter_.setStyle(SkPaint::kStroke_Style);
        break;
      case tttext::FillStyle::kStrokeAndFill:
        painter_.setStyle(SkPaint::kStrokeAndFill_Style);
        break;
    }
    painter_.setColor(painter->GetColor());
    if (painter->IsBold()) {
      painter_.setStyle(SkPaint::kStrokeAndFill_Style);
      painter_.setStrokeWidth(painter->GetStrokeWidth() +
                              painter->GetTextSize() *
                                  tttext::FAKE_BOLD_RATE());
    } else {
      painter_.setStrokeWidth(painter->GetStrokeWidth());
    }
    return painter_;
  }

 private:
  SkPaint painter_;
  SkCanvas* canvas_;
};

#endif  // PUBLIC_TEXTRA_PLATFORM_SKIA_SKIA_CANVAS_HELPER_H_
