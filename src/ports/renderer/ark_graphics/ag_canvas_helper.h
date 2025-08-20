// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_CANVAS_HELPER_H_
#define SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_CANVAS_HELPER_H_
#include <multimedia/image_framework/image/image_source_native.h>
#include <native_drawing/drawing_brush.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_font.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_pen.h>
#include <native_drawing/drawing_pixel_map.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_rect.h>
#include <native_drawing/drawing_round_rect.h>
#include <native_drawing/drawing_sampling_options.h>
#include <native_drawing/drawing_text_blob.h>
#include <textra/i_canvas_helper.h>
#include <textra/macro.h>
#include <textra/painter.h>

#include <memory>

#include "src/ports/renderer/ark_graphics/ag_typeface_helper.h"

namespace ttoffice {
namespace tttext {
class AGCanvasHelper : public ICanvasHelper {
 public:
  explicit AGCanvasHelper(void* canvas)
      : canvas_(static_cast<OH_Drawing_Canvas*>(canvas)) {}
  ~AGCanvasHelper() override = default;

 public:
  std::unique_ptr<Painter> CreatePainter() override {
    return std::make_unique<Painter>();
  }
  void StartPaint() override {}
  void EndPaint() override {}

  // draw APIs
  void Save() override { OH_Drawing_CanvasSave(canvas_); }
  void Restore() override { OH_Drawing_CanvasRestore(canvas_); }
  void Translate(float dx, float dy) override {
    OH_Drawing_CanvasTranslate(canvas_, dx, dy);
  }
  void Scale(float sx, float sy) override {
    OH_Drawing_CanvasScale(canvas_, sx, sy);
  }
  void Rotate(float degrees) override {}
  void Skew(float sx, float sy) override {}
  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    auto* rect = OH_Drawing_RectCreate(left, top, right, bottom);
    OH_Drawing_CanvasClipRect(canvas_, rect, INTERSECT, doAntiAlias);
    OH_Drawing_RectDestroy(rect);
  }
  void Clear() override { OH_Drawing_CanvasClear(canvas_, 0xFFFFFFFF); }
  void ClearRect(float left, float top, float right, float bottom) override {
    TTASSERT(false);
  }
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {
    TTASSERT(false);
  }
  void DrawColor(uint32_t color) override {
    OH_Drawing_CanvasDrawColor(canvas_, color, BLEND_MODE_CLEAR);
  }
  void DrawLine(float x1, float y1, float x2, float y2,
                tttext::Painter* painter) override {
    auto* pen = OH_Drawing_PenCreate();
    OH_Drawing_PenSetColor(pen, painter->GetColor());
    OH_Drawing_PenSetWidth(pen, painter->GetStrokeWidth());
    OH_Drawing_CanvasAttachPen(canvas_, pen);
    OH_Drawing_CanvasDrawLine(canvas_, x1, y1, x2, y2);
    OH_Drawing_CanvasDetachPen(canvas_);
    // release resources
    OH_Drawing_PenDestroy(pen);
  }
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    auto* brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, painter->GetColor());
    auto* pen = OH_Drawing_PenCreate();
    OH_Drawing_PenSetColor(pen, painter->GetColor());
    OH_Drawing_PenSetWidth(pen, painter->GetStrokeWidth());
    auto fill_style = painter->GetFillStyle();
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachBrush(canvas_, brush);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachPen(canvas_, pen);
    }
    auto* rect = OH_Drawing_RectCreate(left, top, right, bottom);
    OH_Drawing_CanvasDrawRect(canvas_, rect);
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachBrush(canvas_);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachPen(canvas_);
    }
    OH_Drawing_BrushDestroy(brush);
    OH_Drawing_PenDestroy(pen);
    OH_Drawing_RectDestroy(rect);
  }
  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tttext::Painter* painter) override {
    auto* brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, painter->GetColor());
    auto* pen = OH_Drawing_PenCreate();
    OH_Drawing_PenSetColor(pen, painter->GetColor());
    OH_Drawing_PenSetWidth(pen, painter->GetStrokeWidth());
    auto fill_style = painter->GetFillStyle();
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachBrush(canvas_, brush);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachPen(canvas_, pen);
    }
    auto* rect = OH_Drawing_RectCreate(left, top, right, bottom);
    auto* round_rect = OH_Drawing_RoundRectCreate(rect, radius, radius);
    OH_Drawing_CanvasDrawRoundRect(canvas_, round_rect);
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachBrush(canvas_);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachPen(canvas_);
    }
    OH_Drawing_BrushDestroy(brush);
    OH_Drawing_PenDestroy(pen);
    OH_Drawing_RectDestroy(rect);
    OH_Drawing_RoundRectDestroy(round_rect);
  }
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override {
    auto* brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, painter->GetColor());
    auto* pen = OH_Drawing_PenCreate();
    OH_Drawing_PenSetColor(pen, painter->GetColor());
    OH_Drawing_PenSetWidth(pen, painter->GetStrokeWidth());
    auto fill_style = painter->GetFillStyle();
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachBrush(canvas_, brush);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasAttachPen(canvas_, pen);
    }
    auto* point = OH_Drawing_PointCreate(x, y);
    OH_Drawing_CanvasDrawCircle(canvas_, point, radius);
    if (fill_style == FillStyle::kFill ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachBrush(canvas_);
    }
    if (fill_style == FillStyle::kStroke ||
        fill_style == FillStyle::kStrokeAndFill) {
      OH_Drawing_CanvasDetachPen(canvas_);
    }
    OH_Drawing_BrushDestroy(brush);
    OH_Drawing_PenDestroy(pen);
    OH_Drawing_PointDestroy(point);
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void BuildPath(OH_Drawing_Path* ag_path, Path* path) {
    switch (path->type_) {
      case PathType::kMoveTo: {
        auto* point = reinterpret_cast<MoveToPath*>(path);
        OH_Drawing_PathMoveTo(ag_path, point->point_[0], point->point_[1]);
        break;
      }
      case PathType::kLines: {
        auto lines = reinterpret_cast<LinesPath*>(path);
        if (lines->points_.empty()) return;
        for (auto iter = lines->points_.begin(); iter != lines->points_.end();
             iter++) {
          OH_Drawing_PathLineTo(ag_path, (*iter)[0], (*iter)[1]);
        }
      } break;
      case PathType::kArc: {
        // auto arc = reinterpret_cast<ArcPath*>(path);
        TTASSERT(false);
        // OH_Drawing_PathLineTo(ag_path, arc->start_[0], arc->start_[1]);
        // OH_Drawing_PathArcTo(ag_path, arc->mid_.x_, arc->mid_.y_,
        //                        arc->end_.x_, arc->end_.y_, arc->radius_);
      } break;
      case PathType::kBezier: {
        auto bezier = reinterpret_cast<BezierPath*>(path);
        auto count = bezier->points_.size();
        if (count < 3) return;
        if (count == 3) {
          OH_Drawing_PathLineTo(ag_path, bezier->points_[0][0],
                                bezier->points_[0][1]);
          OH_Drawing_PathQuadTo(ag_path, bezier->points_[1][0],
                                bezier->points_[1][1], bezier->points_[2][0],
                                bezier->points_[2][1]);
        } else {
          OH_Drawing_PathLineTo(ag_path, bezier->points_[0][0],
                                bezier->points_[0][1]);
          OH_Drawing_PathCubicTo(ag_path, bezier->points_[1][0],
                                 bezier->points_[1][1], bezier->points_[2][0],
                                 bezier->points_[2][1], bezier->points_[3][0],
                                 bezier->points_[3][1]);
        }
      } break;
      case PathType::kMultiPath: {
        auto multi = reinterpret_cast<MultiPath*>(path);
        for (auto& one_path : multi->path_list_) {
          BuildPath(ag_path, one_path.get());
        }
      } break;
      default:
        break;
    }
  }
  /*
   * Draw path
   * */
  void DrawPath(Path* path, tttext::Painter* painter) override {
    auto* ag_path = OH_Drawing_PathCreate();
    OH_Drawing_Brush* brush = OH_Drawing_BrushCreate();
    OH_Drawing_Pen* pen = OH_Drawing_PenCreate();
    if (painter->GetFillStyle() == FillStyle::kFill) {
      // Create brush, this example fills closed paths with color, so brush is
      // needed
      OH_Drawing_BrushSetColor(brush, painter->GetColor());
      // Set brush in canvas
      OH_Drawing_CanvasAttachBrush(canvas_, brush);
    } else {
      // Create pen object
      // Set pen stroke color
      OH_Drawing_PenSetColor(pen, painter->GetColor());
      // Set pen line width
      OH_Drawing_PenSetWidth(pen, painter->GetStrokeWidth());
      // Set pen corner style
      OH_Drawing_PenSetJoin(pen, LINE_ROUND_JOIN);
      // Set pen in canvas
      OH_Drawing_CanvasAttachPen(canvas_, pen);
    }
    BuildPath(ag_path, path);
    OH_Drawing_CanvasDrawPath(canvas_, ag_path);
    OH_Drawing_PathDestroy(ag_path);
    OH_Drawing_PenDestroy(pen);
    OH_Drawing_BrushDestroy(brush);
  }
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void DrawGlyphs(const tttext::ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float origin_x, float origin_y, float* x, float* y,
                  Painter* painter) override {
    OH_Drawing_CanvasSave(canvas_);
    auto* font_helper = TTDYNAMIC_CAST<const AGTypefaceHelper*>(font);
    auto* oh_font = font_helper->GetTypeface();
    OH_Drawing_TextBlobBuilder* builder = OH_Drawing_TextBlobBuilderCreate();
    const OH_Drawing_RunBuffer* runBuffer =
        OH_Drawing_TextBlobBuilderAllocRunPos(builder, oh_font, glyph_count,
                                              nullptr);
    for (auto idx = 0u; idx < glyph_count; idx++) {
      runBuffer->glyphs[idx] = glyphs[idx];
      runBuffer->pos[idx * 2] = x[idx];
      runBuffer->pos[idx * 2 + 1] = y[idx];
    }
    OH_Drawing_TextBlob* textBlob = OH_Drawing_TextBlobBuilderMake(builder);
    auto* brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, painter->GetColor());
    OH_Drawing_CanvasAttachBrush(canvas_, brush);

    OH_Drawing_Pen* pen = nullptr;
    if (painter->IsBold()) {
      pen = OH_Drawing_PenCreate();
      OH_Drawing_PenSetColor(pen, painter->GetColor());
      OH_Drawing_PenSetWidth(pen, painter->GetTextSize() / 40);
      OH_Drawing_CanvasAttachPen(canvas_, pen);
    }
    OH_Drawing_CanvasTranslate(canvas_, origin_x, origin_y);
    if (painter->IsItalic()) {
      OH_Drawing_CanvasSkew(canvas_, -0.3f, 0);
    }
    OH_Drawing_CanvasDrawTextBlob(canvas_, textBlob, 0, 0);
    OH_Drawing_CanvasDetachBrush(canvas_);
    OH_Drawing_BrushDestroy(brush);

    if (pen != nullptr) {
      OH_Drawing_CanvasDetachPen(canvas_);
      OH_Drawing_PenDestroy(pen);
    }
    OH_Drawing_CanvasRestore(canvas_);
  }
  void DrawRunDelegate(const RunDelegate* delegate, float left, float top,
                       float right, float bottom,
                       tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              tttext::Painter* painter) override {
    TTASSERT(false);
  }
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override {
    auto* pix = reinterpret_cast<const OH_PixelmapNative*>(src);
    OH_Drawing_PixelMap* pixelMap = OH_Drawing_PixelMapGetFromOhPixelMapNative(
        const_cast<OH_PixelmapNative*>(pix));
    // Pixel extraction area in PixelMap
    OH_Drawing_Rect* src_rect =
        OH_Drawing_RectCreate(0, 0, right - left, bottom - top);
    // Display area in canvas
    OH_Drawing_Rect* dst_rect = OH_Drawing_RectCreate(left, top, right, bottom);
    // Sampling options object
    OH_Drawing_SamplingOptions* samplingOptions =
        OH_Drawing_SamplingOptionsCreate(
            OH_Drawing_FilterMode::FILTER_MODE_LINEAR,
            OH_Drawing_MipmapMode::MIPMAP_MODE_LINEAR);
    OH_Drawing_CanvasDrawPixelMapRect(canvas_, pixelMap, src_rect, dst_rect,
                                      samplingOptions);
    OH_Drawing_PixelMapDissolve(pixelMap);
  }

  /*
   * Note: if srcRectPercent is ture and value of src_left/top/right/bottom is
   * less than 1.0f will be treated as Percentage
   */
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tttext::Painter* painter,
                     bool srcRectPercent = false) override {
    TTASSERT(false);
  }

 private:
  OH_Drawing_Canvas* canvas_;
};
};  // namespace tttext
};  // namespace ttoffice

#endif  // SRC_PORTS_RENDERER_ARK_GRAPHICS_AG_CANVAS_HELPER_H_
