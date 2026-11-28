// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_CANVAS_IMPL_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_CANVAS_IMPL_HPP_

#include <include/core/SkBlurTypes.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkPath.h>
#include <include/core/SkTextBlob.h>

#include <textra/capi/platform/skia/textra_skia_paint_impl.hpp>
#include <textra/capi/platform/skia/textra_skia_typeface_impl.hpp>

namespace textra_native {
namespace skia_backend {
// Canvas remains borrowed from the host.
inline void TEXTRA_CALL DestroyCanvas(void*, void*) {}
inline textra_status_t Path(SkPath& out, const textra_backend_path_t* p) {
  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto pts = p->points;
  switch (p->kind) {
    case TEXTRA_PATH_KIND_LINES:
      for (uint32_t i = 0; i < p->point_count; ++i)
        out.lineTo(pts[i].x, pts[i].y);
      break;
    case TEXTRA_PATH_KIND_ARC:
      if (p->point_count != 3) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.lineTo(pts[0].x, pts[0].y);
      out.arcTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, p->radius);
      break;
    case TEXTRA_PATH_KIND_BEZIER:
      if (p->point_count < 3 || p->point_count > 4)
        return TEXTRA_STATUS_UNSUPPORTED;
      out.lineTo(pts[0].x, pts[0].y);
      if (p->point_count == 3)
        out.quadTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y);
      else
        out.cubicTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[3].x, pts[3].y);
      break;
    case TEXTRA_PATH_KIND_MOVE_TO:
      if (p->point_count != 1) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.moveTo(pts[0].x, pts[0].y);
      break;
    case TEXTRA_PATH_KIND_MULTI_PATH:
      for (uint32_t i = 0; i < p->child_count; ++i) {
        auto s = Path(out, p->children + i);
        if (s) return s;
      }
      break;
    default:
      return TEXTRA_STATUS_UNSUPPORTED;
  }
  return 0;
}
inline textra_status_t TEXTRA_CALL StartPaint(void* userdata, void* object) {
  return 0;
}

inline textra_status_t TEXTRA_CALL EndPaint(void* userdata, void* object) {
  return 0;
}

inline textra_status_t TEXTRA_CALL Save(void* userdata, void* object) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->save();
  return 0;
}

inline textra_status_t TEXTRA_CALL Restore(void* userdata, void* object) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->restore();
  return 0;
}

inline textra_status_t TEXTRA_CALL Translate(void* userdata, void* object,
                                             float dx, float dy) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->translate(dx, dy);
  return 0;
}

inline textra_status_t TEXTRA_CALL Scale(void* userdata, void* object, float sx,
                                         float sy) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->scale(sx, sy);
  return 0;
}

inline textra_status_t TEXTRA_CALL Rotate(void* userdata, void* object,
                                          float degrees) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->rotate(degrees);
  return 0;
}

inline textra_status_t TEXTRA_CALL Skew(void* userdata, void* object, float sx,
                                        float sy) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->skew(sx, sy);
  return 0;
}

inline textra_status_t TEXTRA_CALL ClipRect(void* userdata, void* object,
                                            float left, float top, float right,
                                            float bottom,
                                            textra_bool_t antialias) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->clipRect(SkRect::MakeLTRB(left, top, right, bottom),
                   SkClipOp::kIntersect, antialias != 0);
  return 0;
}

inline textra_status_t TEXTRA_CALL Clear(void* userdata, void* object) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->clear(0);
  return 0;
}

inline textra_status_t TEXTRA_CALL ClearRect(void* userdata, void* object,
                                             float left, float top, float right,
                                             float bottom) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->save();
  canvas->clipRect(SkRect::MakeLTRB(left, top, right, bottom));
  canvas->drawColor(0, SkBlendMode::kClear);
  canvas->restore();
  return 0;
}

inline textra_status_t TEXTRA_CALL FillRect(void* userdata, void* object,
                                            float left, float top, float right,
                                            float bottom, uint32_t color) {
  auto canvas = static_cast<SkCanvas*>(object);
  SkPaint fill;
  fill.setColor(color);
  canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), fill);
  return 0;
}

inline textra_status_t TEXTRA_CALL DrawColor(void* userdata, void* object,
                                             uint32_t color) {
  auto canvas = static_cast<SkCanvas*>(object);
  canvas->drawColor(color);
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawLine(void* userdata, void* object, float x1, float y1, float x2, float y2,
         const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  if (p.gradient) {
    native_paint.setShader(nullptr);
    native_paint.setColor(p.stroke_color);
  }
  canvas->drawLine(x1, y1, x2, y2, native_paint);
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawRect(void* userdata, void* object, float left, float top, float right,
         float bottom, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawOval(void* userdata, void* object, float left, float top, float right,
         float bottom, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawOval(SkRect::MakeLTRB(left, top, right, bottom), draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawCircle(void* userdata, void* object, float x, float y, float radius,
           const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint,
       [&](const SkPaint& draw) { canvas->drawCircle(x, y, radius, draw); });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawArc(void* userdata, void* object, float left, float top, float right,
        float bottom, float start_angle, float sweep_angle,
        textra_bool_t use_center, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawArc(SkRect::MakeLTRB(left, top, right, bottom), start_angle,
                    sweep_angle, use_center != 0, draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawPath(void* userdata, void* object, const textra_backend_path_t* path,
         const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  SkPath native_path;
  auto status = Path(native_path, path);
  if (status) return status;
  Draw(p, native_paint,
       [&](const SkPaint& draw) { canvas->drawPath(native_path, draw); });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawArcTo(void* userdata, void* object, float start_x, float start_y,
          float mid_x, float mid_y, float end_x, float end_y, float radius,
          const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  SkPath native_path;
  native_path.moveTo(start_x, start_y);
  native_path.arcTo(mid_x, mid_y, end_x, end_y, radius);
  Draw(p, native_paint,
       [&](const SkPaint& draw) { canvas->drawPath(native_path, draw); });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawText(void* userdata, void* object, void* typeface, const char* text,
         uint32_t text_bytes, float x, float y,
         const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  if (!typeface) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto font = Font(typeface, p.text_size);
  font.setSkewX((p.italic ? -0.25f : 0) + p.text_skew);
  if (p.gradient) font.setEmbolden(p.bold != 0);
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawSimpleText(text, text_bytes, SkTextEncoding::kUTF8, x, y, font,
                           draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawGlyphs(void* userdata, void* object, void* typeface, uint32_t glyph_count,
           const uint16_t* glyphs, const char* text, uint32_t text_bytes,
           float origin_x, float origin_y, float* x, float* y,
           const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  if (!typeface) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto font = Font(typeface, p.text_size);
  font.setSkewX((p.italic ? -0.25f : 0) + p.text_skew);
  if (p.gradient) font.setEmbolden(p.bold != 0);
  if (!glyph_count) return 0;
  SkTextBlobBuilder builder;
  auto run =
      builder.allocRunPosH(font, static_cast<int>(glyph_count), 0, nullptr);
  for (uint32_t i = 0; i < glyph_count; ++i) {
    run.glyphs[i] = glyphs[i];
    run.pos[i] = x[i];
  }
  auto blob = builder.make();
  for (uint32_t i = 0; i < p.shadow_count; ++i) {
    auto shadow = native_paint;
    auto& s = p.shadows[i];
    shadow.setShader(nullptr);
    shadow.setColor(s.color);
    shadow.setMaskFilter(
        s.blur_radius
            ? SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                     static_cast<float>(s.blur_radius), false)
            : nullptr);
    canvas->drawTextBlob(blob, origin_x + s.offset_x, origin_y + s.offset_y,
                         shadow);
  }
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawTextBlob(blob, origin_x, origin_y, draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL DrawImage(
    void* userdata, void* object, const char* source, float left, float top,
    float right, float bottom, const textra_backend_paint_t* paint) {
  return TEXTRA_STATUS_UNSUPPORTED;
}

inline textra_status_t TEXTRA_CALL DrawImageRect(
    void* userdata, void* object, const char* source, float src_left,
    float src_top, float src_right, float src_bottom, float dst_left,
    float dst_top, float dst_right, float dst_bottom,
    const textra_backend_paint_t* paint, textra_bool_t source_rect_percent) {
  return TEXTRA_STATUS_UNSUPPORTED;
}

inline textra_status_t TEXTRA_CALL
DrawRoundRect(void* userdata, void* object, float left, float top, float right,
              float bottom, float radius, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<SkCanvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const SkPaint& draw) {
    canvas->drawRoundRect(SkRect::MakeLTRB(left, top, right, bottom), radius,
                          radius, draw);
  });
  return 0;
}

inline const textra_backend_canvas_api_t& CanvasApi() {
  static const textra_backend_canvas_api_t kCanvasApi = {
      sizeof(textra_backend_canvas_api_t),
      TEXTRA_CANVAS_API_VERSION,
      DestroyCanvas,
      StartPaint,
      EndPaint,
      Save,
      Restore,
      Translate,
      Scale,
      Rotate,
      Skew,
      ClipRect,
      Clear,
      ClearRect,
      FillRect,
      DrawColor,
      DrawLine,
      DrawRect,
      DrawOval,
      DrawCircle,
      DrawArc,
      DrawPath,
      DrawArcTo,
      DrawText,
      DrawGlyphs,
      DrawImage,
      DrawImageRect,
      DrawRoundRect};
  return kCanvasApi;
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_CANVAS_IMPL_HPP_
