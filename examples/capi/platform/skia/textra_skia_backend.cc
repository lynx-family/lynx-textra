// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <examples/capi/platform/skia/textra_skia_backend.h>
#include <include/core/SkBlurTypes.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkPath.h>
#include <include/core/SkStream.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkTypeface.h>

#include <examples/capi/platform/skia/textra_skia_gradient_impl.hpp>
#include <utility>
#include <vector>
namespace textra_native {
namespace skia_backend {
struct Face {
  sk_sp<SkTypeface> font;
  int index = 0;
  std::vector<uint8_t> bytes;
  explicit Face(sk_sp<SkTypeface> f) : font(std::move(f)) {
    auto stream = font->openStream(&index);
    if (stream) {
      bytes.resize(stream->getLength());
      auto n = stream->read(bytes.data(), bytes.size());
      bytes.resize(n);
    }
  }
};
inline textra_status_t Wrap(sk_sp<SkTypeface> f, void** out) {
  *out = nullptr;
  if (f) *out = new Face(std::move(f));
  return 0;
}
inline textra_status_t TEXTRA_CALL CreateManager(void*, void* p, void** out) {
  *out = nullptr;

  if (!p) return TEXTRA_STATUS_UNSUPPORTED;
  auto manager = static_cast<SkFontMgr*>(p);
  manager->ref();
  *out = manager;
  return TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL Import(void*, const void* p, void** out) {
  *out = nullptr;

  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  return Wrap(sk_ref_sp(static_cast<SkTypeface*>(const_cast<void*>(p))), out);
}
// Canvas remains borrowed from the host.
inline void TEXTRA_CALL DestroyCanvas(void*, void*) {}
inline void TEXTRA_CALL DestroyManager(void*, void* p) {
  static_cast<SkFontMgr*>(p)->unref();
}
inline void TEXTRA_CALL DestroyFace(void*, void* p) {
  delete static_cast<Face*>(p);
}
inline textra_status_t TEXTRA_CALL Count(void*, void* p, int32_t* out) {
  *out = static_cast<SkFontMgr*>(p)->countFamilies();
  return 0;
}
inline textra_status_t TEXTRA_CALL Native(void*, void* p, const void** out) {
  *out = static_cast<Face*>(p)->font.get();
  return 0;
}
inline textra_status_t TEXTRA_CALL Platform(void* u, void*, void* p,
                                            const void** out) {
  return Native(u, p, out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyle(void* u, void* p, const char* family,
                 const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->matchFamilyStyle(family, font_style), out);
}
inline textra_status_t TEXTRA_CALL
MatchGenericFamily(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyleLiteral(void* u, void* p, const char* family,
                        const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL MatchFamilyStyleCharacter(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    const char* const* languages, uint32_t language_count, uint32_t character,
    void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->matchFamilyStyleCharacter(
                  family, font_style, const_cast<const char**>(languages),
                  static_cast<int>(language_count), character),
              out);
}
inline textra_status_t TEXTRA_CALL MakeFromFile(void* u, void* p,
                                                const char* path,
                                                int32_t collection_index,
                                                void** out) {
  *out = nullptr;
  return Wrap(static_cast<SkFontMgr*>(p)->makeFromFile(path, collection_index),
              out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypeface(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto* manager = static_cast<SkFontMgr*>(p);
  SkFontStyle font_style(style->weight, style->width,
                         static_cast<SkFontStyle::Slant>(style->slant));
  return Wrap(manager->legacyMakeTypeface(family, font_style), out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypefaceLiteral(void* u, void* p, const char* family,
                          const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return LegacyMakeTypeface(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL CreateTypefaceFromPlatformFont(
    void* u, void* p, const void* native_font, void** out) {
  *out = nullptr;
  return Import(u, native_font, out);
}

inline SkFont Font(void* p, float z) {
  return SkFont(static_cast<Face*>(p)->font, z);
}
inline textra_status_t TEXTRA_CALL Advance(void*, void* p, uint16_t g, float z,
                                           float* out) {
  Font(p, z).getWidths(&g, 1, out, nullptr);
  return 0;
}
inline textra_status_t TEXTRA_CALL Advances(void*, void* p, const uint16_t* g,
                                            uint32_t n, float z, float* out) {
  Font(p, z).getWidths(g, static_cast<int>(n), out, nullptr);
  return 0;
}
inline textra_status_t TEXTRA_CALL Bound(void*, void* p, uint16_t g, float z,
                                         textra_rect_t* out) {
  SkRect r;
  Font(p, z).getWidths(&g, 1, nullptr, &r);
  *out = {r.left(), r.top(), r.width(), r.height()};
  return 0;
}
inline textra_status_t TEXTRA_CALL Bounds(void*, void* p, const uint16_t* g,
                                          uint32_t n, float z,
                                          textra_rect_t* out) {
  *out = {};
  if (!n) return TEXTRA_STATUS_OK;
  std::vector<SkRect> r(n);
  Font(p, z).getWidths(g, static_cast<int>(n), nullptr, r.data());
  auto joined = r[0];
  for (auto& rect : r) joined.join(rect);
  *out = {joined.left(), joined.top(), joined.right(), joined.bottom()};
  return TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL Data(void*, void* p, const void** out) {
  auto& bytes = static_cast<Face*>(p)->bytes;
  *out = bytes.data();
  return bytes.empty() ? TEXTRA_STATUS_UNSUPPORTED : TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL DataSize(void*, void* p, uint64_t* out) {
  auto& bytes = static_cast<Face*>(p)->bytes;
  *out = bytes.size();
  return bytes.empty() ? TEXTRA_STATUS_UNSUPPORTED : TEXTRA_STATUS_OK;
}

inline textra_status_t TEXTRA_CALL Index(void*, void* p, int32_t* out) {
  *out = static_cast<Face*>(p)->index;
  return 0;
}
inline textra_status_t TEXTRA_CALL Glyph(void*, void* p, uint32_t c,
                                         uint32_t selector, uint16_t* out) {
  if (selector) return TEXTRA_STATUS_UNSUPPORTED;
  *out = static_cast<Face*>(p)->font->unicharToGlyph(static_cast<SkUnichar>(c));
  return 0;
}
inline textra_status_t TEXTRA_CALL Glyphs(void*, void* p, const uint32_t* c,
                                          uint32_t n, uint16_t* out) {
  std::vector<SkUnichar> chars;
  if (n) chars.assign(c, c + n);
  static_cast<Face*>(p)->font->unicharsToGlyphs(chars.data(),
                                                static_cast<int>(n), out);
  return TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL Units(void*, void* p, uint32_t* out) {
  *out = static_cast<Face*>(p)->font->getUnitsPerEm();
  return 0;
}
inline textra_status_t TEXTRA_CALL Info(void*, void* p, float z,
                                        textra_backend_font_metrics_t* out) {
  SkFontMetrics m{};
  Font(p, z).getMetrics(&m);
  out->top = m.fTop;
  out->ascent = m.fAscent;
  out->descent = m.fDescent;
  out->bottom = m.fBottom;
  out->leading = m.fLeading;
  out->font_size = z;
  return 0;
}
inline textra_status_t TEXTRA_CALL Em(void* u, void* p, float z,
                                      textra_backend_em_height_t* out) {
  textra_backend_font_metrics_t m{sizeof(m), 0, 0, 0, 0, 0, z};
  auto s = Info(u, p, z, &m);
  if (!s) {
    out->ascent = -m.ascent;
    out->descent = m.descent;
  }
  return s;
}
inline textra_status_t TEXTRA_CALL Style(void*, void* p,
                                         textra_font_style_t* out) {
  auto s = static_cast<Face*>(p)->font->fontStyle();
  *out = {static_cast<uint16_t>(s.weight()), static_cast<uint8_t>(s.width()),
          static_cast<uint8_t>(s.slant())};
  return 0;
}
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
inline textra_status_t Path(SkPath& out, const textra_backend_path_t* p) {
  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto pts = p->points;
  switch (p->kind) {
    case 0:
      for (uint32_t i = 0; i < p->point_count; ++i)
        out.lineTo(pts[i].x, pts[i].y);
      break;
    case 1:
      if (p->point_count != 3) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.lineTo(pts[0].x, pts[0].y);
      out.arcTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, p->radius);
      break;
    case 2:
      if (p->point_count < 3 || p->point_count > 4)
        return TEXTRA_STATUS_UNSUPPORTED;
      out.lineTo(pts[0].x, pts[0].y);
      if (p->point_count == 3)
        out.quadTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y);
      else
        out.cubicTo(pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[3].x, pts[3].y);
      break;
    case 3:
      if (p->point_count != 1) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.moveTo(pts[0].x, pts[0].y);
      break;
    case 4:
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
  static const textra_backend_canvas_api_t api = {
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
  return api;
}

inline const textra_backend_font_manager_api_t& ManagerApi() {
  static const textra_backend_font_manager_api_t api = {
      sizeof(textra_backend_font_manager_api_t),
      TEXTRA_FONT_MANAGER_API_VERSION,
      DestroyManager,
      Count,
      MatchFamilyStyle,
      MatchGenericFamily,
      MatchFamilyStyleLiteral,
      MatchFamilyStyleCharacter,
      MakeFromFile,
      LegacyMakeTypeface,
      LegacyMakeTypefaceLiteral,
      CreateTypefaceFromPlatformFont,
      Platform};
  return api;
}

inline const textra_backend_typeface_api_t& FaceApi() {
  static const textra_backend_typeface_api_t api = {
      sizeof(textra_backend_typeface_api_t),
      TEXTRA_TYPEFACE_API_VERSION,
      DestroyFace,
      Advance,
      Advances,
      Bound,
      Bounds,
      Data,
      DataSize,
      Index,
      Glyph,
      Glyphs,
      Units,
      Info,
      Em,
      Style,
      Native};
  return api;
}
}  // namespace skia_backend
}  // namespace textra_native
extern "C" textra_status_t TEXTRA_CALL
textra_skia_canvas_callbacks_initialize(textra_canvas_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      !options->native_canvas)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  options->backend = TEXTRA_RENDERING_BACKEND_SKIA;
  options->callbacks = &textra_native::skia_backend::CanvasApi();
  return TEXTRA_STATUS_OK;
}
extern "C" textra_status_t TEXTRA_CALL
textra_skia_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options) {
  namespace native = textra_native::skia_backend;
  return textra_native::InitializeManager(
      options, native::ManagerApi(), native::FaceApi(), native::CreateManager);
}
extern "C" textra_status_t TEXTRA_CALL
textra_skia_typeface_callbacks_initialize(textra_typeface_options_t* options) {
  namespace native = textra_native::skia_backend;
  return textra_native::InitializeTypeface(options,
                                           TEXTRA_RENDERING_BACKEND_SKIA,
                                           native::FaceApi(), native::Import);
}
