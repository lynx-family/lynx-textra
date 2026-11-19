// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/capi/textra_callback_utils.h"
#include "src/capi/textra_internal.h"

using namespace textra_capi;

namespace ttoffice {
namespace tttext {
namespace backend {
namespace {
struct PaintView {
  textra_backend_paint_t value{};
  textra_backend_gradient_view_t gradient{};
  std::vector<textra_backend_shadow_t> shadows;
  explicit PaintView(const Painter* p) {
    value.struct_size = sizeof(value);
    if (!p) return;
    value.fill_color = p->GetFillColor().GetPlainColor();
    value.stroke_color = p->GetStrokeColor().GetPlainColor();
    value.stroke_width = p->GetStrokeWidth();
    value.stroke_miter = p->GetStrokeMiter();
    value.text_size = p->GetTextSize();
    value.text_skew = p->GetTextSkew();
    value.font_family = p->GetFontFamily().c_str();
    value.bold = p->IsBold();
    value.italic = p->IsItalic();
    value.underline = p->IsUnderLine();
    value.cap = static_cast<uint32_t>(p->GetCap());
    value.join = static_cast<uint32_t>(p->GetJoin());
    for (auto& s : p->GetShadowList())
      shadows.push_back({s.color_.GetPlainColor(), s.offset_[0], s.offset_[1],
                         s.blur_radius_});
    value.shadows = shadows.data();
    value.shadow_count = static_cast<uint32_t>(shadows.size());
    if (auto g = p->GetFillGradient()) {
      gradient.struct_size = sizeof(gradient);
      gradient.type = static_cast<textra_gradient_type_t>(g->type);
      gradient.bounds = {g->bounds.left, g->bounds.top, g->bounds.width,
                         g->bounds.height};
      gradient.start = {g->start.x, g->start.y};
      gradient.end = {g->end.x, g->end.y};
      gradient.center = {g->center.x, g->center.y};
      gradient.radius_x = g->radius_x;
      gradient.radius_y = g->radius_y;
      gradient.start_angle = g->start_angle;
      gradient.colors = g->colors.data();
      gradient.positions = g->positions.data();
      gradient.stop_count = static_cast<uint32_t>(g->colors.size());
      value.gradient = &gradient;
    }
  }
};
struct PathView {
  textra_backend_path_t value{};
  std::vector<textra_point_t> points;
  std::vector<PathView> children;
  std::vector<textra_backend_path_t> nodes;
  explicit PathView(const Path* p) {
    value.struct_size = sizeof(value);
    if (!p) return;
    value.kind = static_cast<uint32_t>(p->type_);
    auto push = [&](const PathPointF& q) { points.push_back({q[0], q[1]}); };
    switch (p->type_) {
      case PathType::kLines:
        for (auto& q : static_cast<const LinesPath*>(p)->points_) push(q);
        break;
      case PathType::kBezier:
        for (auto& q : static_cast<const BezierPath*>(p)->points_) push(q);
        break;
      case PathType::kMoveTo:
        push(static_cast<const MoveToPath*>(p)->point_);
        break;
      case PathType::kArc: {
        auto a = static_cast<const ArcPath*>(p);
        push(a->start_);
        push(a->mid_);
        push(a->end_);
        value.radius = a->radius_;
        break;
      }
      case PathType::kMultiPath: {
        auto m = static_cast<const MultiPath*>(p);
        children.reserve(m->path_list_.size());
        for (auto& c : m->path_list_) children.emplace_back(c.get());
        for (auto& c : children) nodes.push_back(c.value);
        break;
      }
    }
    value.points = points.data();
    value.point_count = static_cast<uint32_t>(points.size());
    value.children = nodes.data();
    value.child_count = static_cast<uint32_t>(nodes.size());
  }
};
}  // namespace

CallbackCanvas::CallbackCanvas(const textra_canvas_options_t& options)
    : userdata_(options.userdata), native_canvas_(options.native_canvas) {
  CopyCallbacks(callbacks_, options.callbacks);
}

CallbackCanvas::~CallbackCanvas() {
  callbacks_.destroy(userdata_, native_canvas_);
}

std::unique_ptr<Painter> CallbackCanvas::CreatePainter() {
  return std::make_unique<Painter>();
}

void CallbackCanvas::StartPaint() {
  auto callback = callbacks_.start_paint;
  if (callback) callback(userdata_, native_canvas_);
}

void CallbackCanvas::EndPaint() {
  auto callback = callbacks_.end_paint;
  if (callback) callback(userdata_, native_canvas_);
}

void CallbackCanvas::Save() {
  auto callback = callbacks_.save;
  if (callback) callback(userdata_, native_canvas_);
}

void CallbackCanvas::Restore() {
  auto callback = callbacks_.restore;
  if (callback) callback(userdata_, native_canvas_);
}

void CallbackCanvas::Translate(float dx, float dy) {
  auto callback = callbacks_.translate;
  if (callback) callback(userdata_, native_canvas_, dx, dy);
}

void CallbackCanvas::Scale(float sx, float sy) {
  auto callback = callbacks_.scale;
  if (callback) callback(userdata_, native_canvas_, sx, sy);
}

void CallbackCanvas::Rotate(float degrees) {
  auto callback = callbacks_.rotate;
  if (callback) callback(userdata_, native_canvas_, degrees);
}

void CallbackCanvas::Skew(float sx, float sy) {
  auto callback = callbacks_.skew;
  if (callback) callback(userdata_, native_canvas_, sx, sy);
}

void CallbackCanvas::ClipRect(float left, float top, float right, float bottom,
                              bool antialias) {
  auto callback = callbacks_.clip_rect;
  if (callback)
    callback(userdata_, native_canvas_, left, top, right, bottom, antialias);
}

void CallbackCanvas::Clear() {
  auto callback = callbacks_.clear;
  if (callback) callback(userdata_, native_canvas_);
}

void CallbackCanvas::ClearRect(float left, float top, float right,
                               float bottom) {
  auto callback = callbacks_.clear_rect;
  if (callback) callback(userdata_, native_canvas_, left, top, right, bottom);
}

void CallbackCanvas::FillRect(float left, float top, float right, float bottom,
                              uint32_t color) {
  auto callback = callbacks_.fill_rect;
  if (callback)
    callback(userdata_, native_canvas_, left, top, right, bottom, color);
}

void CallbackCanvas::DrawColor(uint32_t color) {
  auto callback = callbacks_.draw_color;
  if (callback) callback(userdata_, native_canvas_, color);
}

void CallbackCanvas::DrawLine(float x1, float y1, float x2, float y2,
                              Painter* painter) {
  auto callback = callbacks_.draw_line;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, x1, y1, x2, y2,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawRect(float left, float top, float right, float bottom,
                              Painter* painter) {
  auto callback = callbacks_.draw_rect;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, left, top, right, bottom,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawOval(float left, float top, float right, float bottom,
                              Painter* painter) {
  auto callback = callbacks_.draw_oval;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, left, top, right, bottom,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawCircle(float x, float y, float radius,
                                Painter* painter) {
  auto callback = callbacks_.draw_circle;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, x, y, radius,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawArc(float left, float top, float right, float bottom,
                             float start_angle, float sweep_angle,
                             bool use_center, Painter* painter) {
  auto callback = callbacks_.draw_arc;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, left, top, right, bottom, start_angle,
           sweep_angle, use_center, painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawPath(Path* path, Painter* painter) {
  auto callback = callbacks_.draw_path;
  if (!callback) return;
  PathView path_view(path);
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, &path_view.value,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawArcTo(float start_x, float start_y, float mid_x,
                               float mid_y, float end_x, float end_y,
                               float radius, Painter* painter) {
  auto callback = callbacks_.draw_arc_to;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, start_x, start_y, mid_x, mid_y, end_x,
           end_y, radius, painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawText(const ITypefaceHelper* typeface, const char* text,
                              uint32_t text_bytes, float x, float y,
                              Painter* painter) {
  auto face = static_cast<const CallbackTypeface*>(typeface);
  auto callback = callbacks_.draw_text;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, face->NativeObject(), text, text_bytes, x,
           y, painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawGlyphs(const ITypefaceHelper* typeface,
                                uint32_t glyph_count, const uint16_t* glyphs,
                                const char* text, uint32_t text_bytes,
                                float origin_x, float origin_y, float* x,
                                float* y, Painter* painter) {
  auto face = static_cast<const CallbackTypeface*>(typeface);
  auto callback = callbacks_.draw_glyphs;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, face->NativeObject(), glyph_count, glyphs,
           text, text_bytes, origin_x, origin_y, x, y,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawImage(const char* source, float left, float top,
                               float right, float bottom, Painter* painter) {
  auto callback = callbacks_.draw_image;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, source, left, top, right, bottom,
           painter ? &paint_view.value : nullptr);
}

void CallbackCanvas::DrawImageRect(const char* source, float src_left,
                                   float src_top, float src_right,
                                   float src_bottom, float dst_left,
                                   float dst_top, float dst_right,
                                   float dst_bottom, Painter* painter,
                                   bool source_rect_percent) {
  auto callback = callbacks_.draw_image_rect;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, source, src_left, src_top, src_right,
           src_bottom, dst_left, dst_top, dst_right, dst_bottom,
           painter ? &paint_view.value : nullptr, source_rect_percent);
}

void CallbackCanvas::DrawRoundRect(float left, float top, float right,
                                   float bottom, float radius,
                                   Painter* painter) {
  auto callback = callbacks_.draw_round_rect;
  if (!callback) return;
  PaintView paint_view(painter);
  callback(userdata_, native_canvas_, left, top, right, bottom, radius,
           painter ? &paint_view.value : nullptr);
}

}  // namespace backend
}  // namespace tttext
}  // namespace ttoffice

textra_status_t TEXTRA_CALL textra_canvas_create(
    const textra_canvas_options_t* options, textra_canvas_t** out) {
  *out = nullptr;
  auto resolved = *options;
  resolved.backend =
      static_cast<uint32_t>(tt::backend::ResolveBackend(options->backend));
  auto object = std::make_unique<textra_canvas>();
  object->impl = tt::PlatformHelper::CreateCanvasHelper(resolved);
  if (!object->impl) return TEXTRA_STATUS_UNSUPPORTED;
  object->backend = resolved.backend;
  *out = object.release();
  return TEXTRA_STATUS_OK;
}
