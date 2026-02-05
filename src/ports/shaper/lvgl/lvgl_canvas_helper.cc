// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <lvgl/src/draw/lv_draw.h>
#include <lvgl/src/draw/lv_draw_arc.h>
#include <lvgl/src/draw/lv_draw_label.h>
#include <lvgl/src/draw/lv_draw_line.h>
#include <lvgl/src/draw/lv_draw_rect.h>
#include <lvgl/src/draw/lv_draw_vector.h>
#include <lvgl/src/misc/lv_area.h>
#include <lvgl/src/misc/lv_area_private.h>
#include <textra/platform/lvgl/lvgl_canvas_helper.h>

#include <cmath>

#include "src/textlayout/utils/u_8_string.h"
#include "textra/platform/lvgl/lvgl_typeface.h"

#define RELAX_USE_LV_MATRIX 0

namespace ttoffice {
namespace tttext {
namespace {

lv_color_t ToLvColor(tttext::TTColor color) {
  return lv_color_make(color.GetRed(), color.GetGreen(), color.GetBlue());
}

lv_opa_t ToLvOpa(tttext::TTColor color) {
  return static_cast<lv_opa_t>(color.GetAlpha());
}

tttext::TTColor ResolveLineColor(const tttext::Painter* painter) {
  if (painter == nullptr) {
    return tttext::TTColor::UNDEFINED;
  }
  auto stroke_color = painter->GetStrokeColor();
  if (stroke_color != tttext::TTColor::UNDEFINED) {
    return stroke_color;
  }
  return painter->GetFillColor();
}

int32_t NormalizeLineWidth(float width) {
  if (width <= 0.f) {
    return 1;
  }
  return std::max<int32_t>(1, static_cast<int32_t>(std::lround(width)));
}

uint8_t ShouldRoundCap(tttext::Cap cap) {
  return cap == tttext::Cap::kRound_Cap ? 1 : 0;
}

bool InitRectDscFromPainter(lv_draw_rect_dsc_t* rect_dsc,
                            const tttext::Painter* painter) {
  if (rect_dsc == nullptr || painter == nullptr) {
    return false;
  }
  auto fill_color = painter->GetFillColor();
  auto stroke_color = painter->GetStrokeColor();
  if (fill_color == tttext::TTColor::UNDEFINED &&
      stroke_color == tttext::TTColor::UNDEFINED) {
    return false;
  }
  lv_draw_rect_dsc_init(rect_dsc);
  if (fill_color != tttext::TTColor::UNDEFINED) {
    rect_dsc->bg_color = ToLvColor(fill_color);
    rect_dsc->bg_opa = ToLvOpa(fill_color);
  } else {
    rect_dsc->bg_opa = LV_OPA_TRANSP;
  }
  if (stroke_color != tttext::TTColor::UNDEFINED) {
    rect_dsc->border_color = ToLvColor(stroke_color);
    rect_dsc->border_opa = ToLvOpa(stroke_color);
    rect_dsc->border_width = NormalizeLineWidth(painter->GetStrokeWidth());
    rect_dsc->border_side = LV_BORDER_SIDE_FULL;
  } else {
    rect_dsc->border_width = 0;
    rect_dsc->border_opa = LV_OPA_TRANSP;
  }
  return true;
}

}  // namespace

LVGLCanvasHelper::LVGLCanvasHelper(lv_layer_t* layer) : layer_(layer) {
  lv_matrix_identity(&current_);
  ApplyMatrixToLayer();
}

std::unique_ptr<Painter> LVGLCanvasHelper::CreatePainter() {
  return std::make_unique<Painter>();
}

void LVGLCanvasHelper::ApplyMatrixToLayer() {
#if RELAX_USE_LV_MATRIX
  if (layer_ != nullptr) {
    layer_->matrix = current_;
  }
#endif
}

lv_fpoint_t LVGLCanvasHelper::TransformPoint(float x, float y) {
  lv_fpoint_t point{x, y};
  lv_matrix_transform_point(&current_, &point);
  return point;
}

lv_area_t LVGLCanvasHelper::TransformRectToArea(float left, float top,
                                                float right, float bottom) {
#if !RELAX_USE_LV_MATRIX
  auto p1 = TransformPoint(left, top);
  auto p2 = TransformPoint(right, top);
  auto p3 = TransformPoint(right, bottom);
  auto p4 = TransformPoint(left, bottom);
  float min_x = std::min(std::min(p1.x, p2.x), std::min(p3.x, p4.x));
  float max_x = std::max(std::max(p1.x, p2.x), std::max(p3.x, p4.x));
  float min_y = std::min(std::min(p1.y, p2.y), std::min(p3.y, p4.y));
  float max_y = std::max(std::max(p1.y, p2.y), std::max(p3.y, p4.y));
  lv_area_t coords;
  coords.x1 = static_cast<int32_t>(std::lround(min_x));
  coords.y1 = static_cast<int32_t>(std::lround(min_y));
  coords.x2 = static_cast<int32_t>(std::lround(max_x));
  coords.y2 = static_cast<int32_t>(std::lround(max_y));
  return coords;
#else
  return lv_area_t{static_cast<int32_t>(std::lround(left)),
                   static_cast<int32_t>(std::lround(top)),
                   static_cast<int32_t>(std::lround(right)),
                   static_cast<int32_t>(std::lround(bottom))};
#endif
}

LVGLCanvasHelper::~LVGLCanvasHelper() = default;

void LVGLCanvasHelper::StartPaint() {
  lv_matrix_identity(&current_);
  stack_.clear();
  clip_stack_.clear();
  ApplyMatrixToLayer();
}

void LVGLCanvasHelper::EndPaint() {}

void LVGLCanvasHelper::Save() {
  stack_.push_back(current_);
  if (layer_ != nullptr) {
    clip_stack_.push_back(layer_->_clip_area);
  }
}

void LVGLCanvasHelper::Restore() {
  if (stack_.empty()) {
    return;
  }
  current_ = stack_.back();
  stack_.pop_back();
  ApplyMatrixToLayer();

  if (layer_ != nullptr && !clip_stack_.empty()) {
    layer_->_clip_area = clip_stack_.back();
    clip_stack_.pop_back();
  }
}

void LVGLCanvasHelper::Translate(float dx, float dy) {
  lv_matrix_translate(&current_, dx, dy);
  ApplyMatrixToLayer();
}

void LVGLCanvasHelper::Scale(float sx, float sy) {
  lv_matrix_scale(&current_, sx, sy);
  ApplyMatrixToLayer();
}

void LVGLCanvasHelper::Rotate(float degrees) {
  lv_matrix_rotate(&current_, degrees);
  ApplyMatrixToLayer();
}

void LVGLCanvasHelper::Skew(float sx, float sy) {
  lv_matrix_skew(&current_, sx, sy);
  ApplyMatrixToLayer();
}

void LVGLCanvasHelper::ClipRect(float left, float top, float right,
                                float bottom, bool doAntiAlias) {
  if (layer_ == nullptr) {
    return;
  }
  auto p1 = TransformPoint(left, top);
  auto p2 = TransformPoint(right, top);
  auto p3 = TransformPoint(right, bottom);
  auto p4 = TransformPoint(left, bottom);
  float min_x = std::min(std::min(p1.x, p2.x), std::min(p3.x, p4.x));
  float max_x = std::max(std::max(p1.x, p2.x), std::max(p3.x, p4.x));
  float min_y = std::min(std::min(p1.y, p2.y), std::min(p3.y, p4.y));
  float max_y = std::max(std::max(p1.y, p2.y), std::max(p3.y, p4.y));
  lv_area_t clip_area;
  clip_area.x1 = static_cast<int32_t>(std::lround(min_x));
  clip_area.y1 = static_cast<int32_t>(std::lround(min_y));
  clip_area.x2 = static_cast<int32_t>(std::lround(max_x));
  clip_area.y2 = static_cast<int32_t>(std::lround(max_y));
  lv_area_t intersected;
  if (lv_area_intersect(&intersected, &layer_->_clip_area, &clip_area)) {
    layer_->_clip_area = intersected;
  } else {
    layer_->_clip_area.x1 = clip_area.x1;
    layer_->_clip_area.y1 = clip_area.y1;
    layer_->_clip_area.x2 = clip_area.x1 - 1;
    layer_->_clip_area.y2 = clip_area.y1 - 1;
  }
}

void LVGLCanvasHelper::Clear() {
  if (layer_ == nullptr || layer_->draw_buf == nullptr) {
    return;
  }
  lv_draw_buf_clear(layer_->draw_buf, nullptr);
}

void LVGLCanvasHelper::ClearRect(float left, float top, float right,
                                 float bottom) {
  printf("ClearRect is not implemented~\n");
}

void LVGLCanvasHelper::FillRect(float left, float top, float right,
                                float bottom, uint32_t color) {
  printf("FillRect is not implemented~\n");
}

void LVGLCanvasHelper::DrawColor(uint32_t color) {
  if (layer_ == nullptr) {
    return;
  }
  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  tttext::TTColor tt_color(color);
  rect_dsc.bg_color = ToLvColor(tt_color);
  rect_dsc.bg_opa = ToLvOpa(tt_color);
  rect_dsc.border_width = 0;
  rect_dsc.border_opa = LV_OPA_TRANSP;
  lv_draw_rect(layer_, &rect_dsc, &layer_->buf_area);
}

void LVGLCanvasHelper::DrawLine(float x1, float y1, float x2, float y2,
                                tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr) {
    return;
  }
  auto color = ResolveLineColor(painter);
  if (color == tttext::TTColor::UNDEFINED) {
    return;
  }
  lv_draw_line_dsc_t line_dsc;
  lv_draw_line_dsc_init(&line_dsc);
  auto p1 = TransformPoint(x1, y1);
  auto p2 = TransformPoint(x2 + 1, y2 + 1);
  line_dsc.p1.x = static_cast<int32_t>(std::lround(p1.x));
  line_dsc.p1.y = static_cast<int32_t>(std::lround(p1.y));
  line_dsc.p2.x = static_cast<int32_t>(std::lround(p2.x));
  line_dsc.p2.y = static_cast<int32_t>(std::lround(p2.y));
  line_dsc.color = ToLvColor(color);
  line_dsc.opa = ToLvOpa(color);
  line_dsc.width = NormalizeLineWidth(painter->GetStrokeWidth());
  line_dsc.round_start = ShouldRoundCap(painter->GetCap());
  line_dsc.round_end = line_dsc.round_start;
  lv_draw_line(layer_, &line_dsc);
}

void LVGLCanvasHelper::DrawRect(float left, float top, float right,
                                float bottom, tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr) {
    return;
  }
  lv_draw_rect_dsc_t rect_dsc;
  if (!InitRectDscFromPainter(&rect_dsc, painter)) {
    return;
  }
  lv_area_t coords = TransformRectToArea(left, top, right, bottom);
  lv_draw_rect(layer_, &rect_dsc, &coords);
}

void LVGLCanvasHelper::DrawRoundRect(float left, float top, float right,
                                     float bottom, float radius,
                                     tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr) {
    return;
  }
  lv_draw_rect_dsc_t rect_dsc;
  if (!InitRectDscFromPainter(&rect_dsc, painter)) {
    return;
  }
  rect_dsc.radius = static_cast<int32_t>(std::lround(radius));
  lv_area_t coords = TransformRectToArea(left, top, right, bottom);
  lv_draw_rect(layer_, &rect_dsc, &coords);
}

void LVGLCanvasHelper::DrawOval(float left, float top, float right,
                                float bottom, tttext::Painter* painter) {
  printf("DrawOval is not implemented~\n");
}

void LVGLCanvasHelper::DrawCircle(float x, float y, float radius,
                                  tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr || radius <= 0.0f) {
    return;
  }
  lv_draw_rect_dsc_t rect_dsc;
  if (!InitRectDscFromPainter(&rect_dsc, painter)) {
    return;
  }
  rect_dsc.radius = LV_RADIUS_CIRCLE;
  lv_area_t coords =
      TransformRectToArea(x - radius, y - radius, x + radius, y + radius);
  lv_draw_rect(layer_, &rect_dsc, &coords);
}

void LVGLCanvasHelper::DrawArc(float left, float top, float right, float bottom,
                               float startAngle, float sweepAngle,
                               bool useCenter, tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr) {
    return;
  }
  auto color = ResolveLineColor(painter);
  if (color == tttext::TTColor::UNDEFINED) {
    return;
  }
  lv_area_t area = TransformRectToArea(left, top, right, bottom);
  int32_t width = area.x2 - area.x1 + 1;
  int32_t height = area.y2 - area.y1 + 1;
  int32_t radius = LV_MIN(width, height) / 2;
  if (radius <= 0) {
    return;
  }
  lv_draw_arc_dsc_t arc_dsc;
  lv_draw_arc_dsc_init(&arc_dsc);
  arc_dsc.color = ToLvColor(color);
  arc_dsc.opa = ToLvOpa(color);
  arc_dsc.width = NormalizeLineWidth(painter->GetStrokeWidth());
  arc_dsc.rounded = ShouldRoundCap(painter->GetCap());
  arc_dsc.center.x = area.x1 + width / 2;
  arc_dsc.center.y = area.y1 + height / 2;
  arc_dsc.radius = static_cast<uint16_t>(radius);
  arc_dsc.start_angle = static_cast<int16_t>(std::lround(startAngle));
  arc_dsc.end_angle =
      static_cast<int16_t>(std::lround(startAngle + sweepAngle));
  lv_draw_arc(layer_, &arc_dsc);
}

void LVGLCanvasHelper::DrawPath(tttext::Path* path, tttext::Painter* painter) {
  printf("DrawPath is not implemented~\n");
}

void LVGLCanvasHelper::DrawArcTo(float start_x, float start_y, float mid_x,
                                 float mid_y, float end_x, float end_y,
                                 float radius, tttext::Painter* painter) {
  printf("DrawArcTo is not implemented~\n");
}

void LVGLCanvasHelper::DrawText(const tttext::ITypefaceHelper* font,
                                const char* text, uint32_t text_bytes, float x,
                                float y, tttext::Painter* painter) {
  printf("DrawText is not implemented~\n");
}

void LVGLCanvasHelper::DrawGlyphs(const tttext::ITypefaceHelper* font,
                                  uint32_t glyph_count, const uint16_t* glyphs,
                                  const char* text, uint32_t text_bytes,
                                  float origin_x, float origin_y, float* x,
                                  float* y, tttext::Painter* painter) {
  if (layer_ == nullptr || painter == nullptr) {
    return;
  }
  TTColor color = ResolveLineColor(painter);
  if (color == TTColor::UNDEFINED) {
    return;
  }

  auto* lv_typeface = static_cast<const LVGLTypeface*>(font);
  lv_draw_letter_dsc_t letter_dsc;
  lv_draw_letter_dsc_init(&letter_dsc);
  letter_dsc.color = ToLvColor(color);
  letter_dsc.opa = ToLvOpa(color);
  letter_dsc.font = lv_typeface->GetLvFont(painter->GetTextSize());
  if (painter->IsUnderLine()) {
    letter_dsc.decor = LV_TEXT_DECOR_UNDERLINE;
  }
  auto xx = origin_x;
  auto yy = origin_y;

  uint32_t idx = 0;
  char32_t char32 = 0;
  while (idx < glyph_count) {
    lv_point_t point = {static_cast<int32_t>(std::lround(xx + x[idx])),
                        static_cast<int32_t>(std::lround(yy + y[idx]))};
    idx += base::U16CharToU32(reinterpret_cast<const char16_t*>(glyphs + idx),
                              glyph_count - idx, &char32);
    letter_dsc.unicode = static_cast<uint32_t>(char32);
    letter_dsc.color = lv_color_hex(painter->GetFillColor());
    lv_draw_letter(layer_, &letter_dsc, &point);
  }
}

void LVGLCanvasHelper::DrawRunDelegate(const tttext::RunDelegate* delegate,
                                       float left, float top, float right,
                                       float bottom, tttext::Painter* painter) {
  printf("DrawRunDelegate is not implemented~\n");
}

void LVGLCanvasHelper::DrawBackgroundDelegate(
    const tttext::RunDelegate* delegate, tttext::Painter* painter) {
  printf("DrawBackgroundDelegate is not implemented~\n");
}

void LVGLCanvasHelper::DrawImage(const char* src, float left, float top,
                                 float right, float bottom,
                                 tttext::Painter* painter) {
  printf("DrawImage is not implemented~\n");
}

void LVGLCanvasHelper::DrawImageRect(const char* src, float src_left,
                                     float src_top, float src_right,
                                     float src_bottom, float dst_left,
                                     float dst_top, float dst_right,
                                     float dst_bottom, tttext::Painter* painter,
                                     bool srcRectPercent) {
  printf("DrawImageRect is not implemented~\n");
}
}  // namespace tttext
}  // namespace ttoffice
