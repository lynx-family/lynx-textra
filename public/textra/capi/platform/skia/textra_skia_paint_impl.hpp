// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_PAINT_IMPL_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_PAINT_IMPL_HPP_

#include <include/core/SkPaint.h>

#include <textra/capi/platform/skia/textra_skia_gradient_impl.hpp>

namespace textra_native {
namespace skia_backend {
inline SkPaint Paint(const textra_backend_paint_t& p) {
  SkPaint out;
  out.setAntiAlias(true);
  out.setStrokeCap(static_cast<SkPaint::Cap>(p.cap));
  out.setStrokeJoin(static_cast<SkPaint::Join>(p.join));
  out.setStrokeMiter(p.stroke_miter);
  out.setStrokeWidth(p.stroke_width);
  if (p.fill_color != kUndefined) {
    out.setColor(p.fill_color);
    out.setStyle(SkPaint::kFill_Style);
  }
  if (p.stroke_color != kUndefined) {
    out.setColor(p.stroke_color);
    out.setStyle(p.fill_color != kUndefined ? SkPaint::kStrokeAndFill_Style
                                            : SkPaint::kStroke_Style);
  }
  if (p.bold) {
    out.setStyle(SkPaint::kStrokeAndFill_Style);
    out.setStrokeWidth(p.stroke_width + p.text_size / 20.f);
  }
  if (p.gradient) {
    out.setStyle(SkPaint::kFill_Style);
    out.setColor(SK_ColorWHITE);
    out.setShader(MakeSkiaGradient(Gradient(*p.gradient)));
  }
  return out;
}
template <class F>
inline void Draw(const textra_backend_paint_t& p, const SkPaint& paint,
                 F draw) {
  draw(paint);
  if (p.gradient && p.stroke_color != kUndefined) {
    auto stroke = paint;
    stroke.setShader(nullptr);
    stroke.setColor(p.stroke_color);
    stroke.setStyle(SkPaint::kStroke_Style);
    draw(stroke);
  }
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_PAINT_IMPL_HPP_
