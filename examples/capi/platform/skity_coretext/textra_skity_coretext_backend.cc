// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <examples/capi/platform/skity_coretext/textra_skity_coretext_backend.h>

#include <examples/capi/platform/skity_coretext/textra_skity_coretext_em_height_impl.hpp>
#include <examples/capi/platform/skity_coretext/textra_skity_coretext_gradient_impl.hpp>
#include <memory>
#include <skity/effect/mask_filter.hpp>
#include <skity/graphic/path.hpp>
#include <skity/render/canvas.hpp>
#include <skity/text/font_manager.hpp>
#include <skity/text/ports/typeface_ct.hpp>
#include <skity/text/text_blob.hpp>
#include <skity/text/text_run.hpp>
#include <utility>
#include <vector>
namespace textra_native {
namespace skity_coretext_backend {
namespace {
struct Manager {
  std::shared_ptr<skity::FontManager> owned;
  skity::FontManager* manager;
};
struct Face {
  std::shared_ptr<skity::Typeface> font;
  skity::capi_coretext::EmHeight em;
  std::shared_ptr<skity::Data> data;
  explicit Face(std::shared_ptr<skity::Typeface> f)
      : font(std::move(f)), em(font) {}
};
textra_status_t Wrap(std::shared_ptr<skity::Typeface> f, void** out) {
  *out = nullptr;
  if (f) *out = new Face(std::move(f));
  return 0;
}
textra_status_t TEXTRA_CALL CreateManager(void*, void* p, void** out) {
  *out = nullptr;

  auto box = std::make_unique<Manager>();
  if (p) {
    box->manager = static_cast<skity::FontManager*>(p);
  } else {
    box->owned = skity::FontManager::RefDefault();
    box->manager = box->owned.get();
  }
  if (!box->manager) return TEXTRA_STATUS_UNSUPPORTED;
  *out = box.release();
  return TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL Import(void*, const void* p, void** out) {
  *out = nullptr;

  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  return Wrap(skity::TypefaceCT::TypefaceFromCTFont(static_cast<CTFontRef>(p)),
              out);
}
// The host owns the native Canvas; releasing an adapter does not release it.
inline void TEXTRA_CALL DestroyCanvas(void*, void*) {}
inline void TEXTRA_CALL DestroyManager(void*, void* p) {
  delete static_cast<Manager*>(p);
}
inline void TEXTRA_CALL DestroyFace(void*, void* p) {
  delete static_cast<Face*>(p);
}
inline textra_status_t TEXTRA_CALL Count(void*, void* p, int32_t* out) {
  *out = static_cast<Manager*>(p)->manager->CountFamilies();
  return 0;
}
inline textra_status_t TEXTRA_CALL Native(void*, void* p, const void** out) {
  *out = skity::TypefaceCT::CTFontFromTypeface(static_cast<Face*>(p)->font);
  return *out ? TEXTRA_STATUS_OK : TEXTRA_STATUS_UNSUPPORTED;
}
inline textra_status_t TEXTRA_CALL Platform(void* u, void*, void* p,
                                            const void** out) {
  return Native(u, p, out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyleLiteral(void* u, void* p, const char* family,
                        const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto& manager = static_cast<Manager*>(p)->manager;
  skity::FontStyle font_style(
      style->weight, style->width,
      static_cast<skity::FontStyle::Slant>(style->slant));
  return Wrap(family ? manager->MatchFamilyStyle(family, font_style)
                     : manager->GetDefaultTypeface(font_style),
              out);
}
inline textra_status_t TEXTRA_CALL
MatchFamilyStyle(void* u, void* p, const char* family,
                 const textra_font_style_t* style, void** out) {
  *out = nullptr;
  auto& manager = static_cast<Manager*>(p)->manager;
  skity::FontStyle font_style(
      style->weight, style->width,
      static_cast<skity::FontStyle::Slant>(style->slant));
  if (!family || !strcmp(family, "sans-serif"))
    return Wrap(manager->GetDefaultTypeface(font_style), out);
  return MatchFamilyStyleLiteral(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL
MatchGenericFamily(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL MatchFamilyStyleCharacter(
    void* u, void* p, const char* family, const textra_font_style_t* style,
    const char* const* languages, uint32_t language_count, uint32_t character,
    void** out) {
  *out = nullptr;
  auto& manager = static_cast<Manager*>(p)->manager;
  skity::FontStyle font_style(
      style->weight, style->width,
      static_cast<skity::FontStyle::Slant>(style->slant));
  return Wrap(manager->MatchFamilyStyleCharacter(
                  family, font_style, const_cast<const char**>(languages),
                  static_cast<int>(language_count), character),
              out);
}
inline textra_status_t TEXTRA_CALL MakeFromFile(void* u, void* p,
                                                const char* path,
                                                int32_t collection_index,
                                                void** out) {
  *out = nullptr;
  return Wrap(
      static_cast<Manager*>(p)->manager->MakeFromFile(path, collection_index),
      out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypeface(void* u, void* p, const char* family,
                   const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyle(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL
LegacyMakeTypefaceLiteral(void* u, void* p, const char* family,
                          const textra_font_style_t* style, void** out) {
  *out = nullptr;
  return MatchFamilyStyleLiteral(u, p, family, style, out);
}
inline textra_status_t TEXTRA_CALL CreateTypefaceFromPlatformFont(
    void* u, void* p, const void* native_font, void** out) {
  *out = nullptr;
  return Import(u, native_font, out);
}

inline skity::Font Font(void* p, float z) {
  skity::Font f(static_cast<Face*>(p)->font, z);
  f.SetHinting(skity::Font::FontHinting::kSlight);
  return f;
}
inline textra_status_t TEXTRA_CALL Advance(void*, void* p, uint16_t g, float z,
                                           float* out) {
  Font(p, z).GetWidths(&g, 1, out, nullptr);
  return 0;
}
inline textra_status_t TEXTRA_CALL Advances(void*, void* p, const uint16_t* g,
                                            uint32_t n, float z, float* out) {
  Font(p, z).GetWidths(g, n, out, nullptr);
  return 0;
}
inline textra_status_t TEXTRA_CALL Bound(void*, void* p, uint16_t g, float z,
                                         textra_rect_t* out) {
  skity::Rect r;
  Font(p, z).GetWidths(&g, 1, nullptr, &r);
  *out = {r.Left(), r.Top(), r.Width(), r.Height()};
  return 0;
}
inline textra_status_t TEXTRA_CALL Bounds(void*, void* p, const uint16_t* g,
                                          uint32_t n, float z,
                                          textra_rect_t* out) {
  *out = {};
  if (!n) return TEXTRA_STATUS_OK;
  std::vector<skity::Rect> r(n);
  Font(p, z).GetWidths(g, n, nullptr, r.data());
  float width = 0, top = r[0].Top(), bottom = r[0].Bottom();
  for (auto& a : r) {
    width += a.Width();
    top = std::fmin(top, a.Top());
    bottom = std::fmax(bottom, a.Bottom());
  }
  *out = {r[0].Left(), top, r[0].Left() + width, bottom};
  return TEXTRA_STATUS_OK;
}
inline textra_status_t TEXTRA_CALL Data(void*, void* p, const void** out) {
  auto& f = *static_cast<Face*>(p);
  if (!f.data) f.data = f.font->GetData();
  *out = f.data ? f.data->RawData() : nullptr;
  return f.data ? TEXTRA_STATUS_OK : TEXTRA_STATUS_UNSUPPORTED;
}
inline textra_status_t TEXTRA_CALL DataSize(void*, void* p, uint64_t* out) {
  auto& f = *static_cast<Face*>(p);
  if (!f.data) f.data = f.font->GetData();
  *out = f.data ? f.data->Size() : 0;
  return f.data ? TEXTRA_STATUS_OK : TEXTRA_STATUS_UNSUPPORTED;
}

inline textra_status_t TEXTRA_CALL Index(void*, void* p, int32_t* out) {
  *out = static_cast<Face*>(p)->font->GetFontDescriptor().collection_index;
  return 0;
}
inline textra_status_t TEXTRA_CALL Glyph(void*, void* p, uint32_t c,
                                         uint32_t selector, uint16_t* out) {
  if (selector) return TEXTRA_STATUS_UNSUPPORTED;
  *out = static_cast<Face*>(p)->font->UnicharToGlyph(c);
  return 0;
}
inline textra_status_t TEXTRA_CALL Glyphs(void*, void* p, const uint32_t* chars,
                                          uint32_t n, uint16_t* out) {
  static_cast<Face*>(p)->font->UnicharsToGlyphs(chars, static_cast<int>(n),
                                                out);
  return 0;
}
inline textra_status_t TEXTRA_CALL Units(void*, void* p, uint32_t* out) {
  *out = static_cast<Face*>(p)->font->GetUnitsPerEm();
  return 0;
}
inline textra_status_t TEXTRA_CALL Info(void*, void* p, float z,
                                        textra_backend_font_metrics_t* out) {
  skity::FontMetrics m{};
  Font(p, z).GetMetrics(&m);
  out->top = 0;
  out->ascent = m.ascent_;
  out->descent = m.descent_;
  out->bottom = 0;
  out->leading = 0;
  out->font_size = z;
  return 0;
}
inline textra_status_t TEXTRA_CALL Em(void*, void* p, float z,
                                      textra_backend_em_height_t* out) {
  auto v = static_cast<Face*>(p)->em.GetAscentAndDescent(z);
  out->ascent = v.first;
  out->descent = v.second;
  return 0;
}
inline textra_status_t TEXTRA_CALL Style(void*, void* p,
                                         textra_font_style_t* out) {
  auto s = static_cast<Face*>(p)->font->GetFontStyle();
  *out = {static_cast<uint16_t>(s.weight()), static_cast<uint8_t>(s.width()),
          static_cast<uint8_t>(s.slant())};
  return 0;
}
inline skity::Paint Paint(const textra_backend_paint_t& p) {
  skity::Paint out;
  out.SetTextSize(p.text_size);
  out.SetStrokeWidth(p.stroke_width);
  out.SetStrokeMiter(p.stroke_miter);
  out.SetStrokeCap(static_cast<skity::Paint::Cap>(p.cap));
  out.SetStrokeJoin(static_cast<skity::Paint::Join>(p.join));
  if (p.fill_color != kUndefined) {
    out.SetFillColor(p.fill_color);
    out.SetStyle(skity::Paint::kFill_Style);
  }
  if (p.stroke_color != kUndefined) {
    out.SetStrokeColor(p.stroke_color);
    out.SetStyle(p.fill_color != kUndefined ? skity::Paint::kStrokeAndFill_Style
                                            : skity::Paint::kStroke_Style);
  }
  if (p.gradient) {
    out.SetStyle(skity::Paint::kFill_Style);
    out.SetColor(0xffffffff);
    out.SetShader(MakeSkityGradient(Gradient(*p.gradient)));
  }
  return out;
}
template <class F>
inline void Draw(const textra_backend_paint_t& p, const skity::Paint& paint,
                 F draw) {
  draw(paint);
  if (p.gradient && p.stroke_color != kUndefined) {
    auto stroke = paint;
    stroke.SetShader(nullptr);
    stroke.SetColor(p.stroke_color);
    stroke.SetStyle(skity::Paint::kStroke_Style);
    draw(stroke);
  }
}
inline textra_status_t Path(skity::Path& out, const textra_backend_path_t* p) {
  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto points = p->points;
  switch (p->kind) {
    case 0:
      for (uint32_t i = 0; i < p->point_count; ++i)
        out.LineTo(points[i].x, points[i].y);
      break;
    case 1:
      if (p->point_count != 3) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.LineTo(points[0].x, points[0].y);
      out.ArcTo(points[1].x, points[1].y, points[2].x, points[2].y, p->radius);
      break;
    case 2:
      if (p->point_count < 3 || p->point_count > 4)
        return TEXTRA_STATUS_UNSUPPORTED;
      out.LineTo(points[0].x, points[0].y);
      if (p->point_count == 3)
        out.QuadTo(points[1].x, points[1].y, points[2].x, points[2].y);
      else
        out.CubicTo(points[1].x, points[1].y, points[2].x, points[2].y,
                    points[3].x, points[3].y);
      break;
    case 3:
      if (p->point_count != 1) return TEXTRA_STATUS_INVALID_ARGUMENT;
      out.MoveTo(points[0].x, points[0].y);
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
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Save();
  return 0;
}

inline textra_status_t TEXTRA_CALL Restore(void* userdata, void* object) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Restore();
  return 0;
}

inline textra_status_t TEXTRA_CALL Translate(void* userdata, void* object,
                                             float dx, float dy) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Translate(dx, dy);
  return 0;
}

inline textra_status_t TEXTRA_CALL Scale(void* userdata, void* object, float sx,
                                         float sy) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Scale(sx, sy);
  return 0;
}

inline textra_status_t TEXTRA_CALL Rotate(void* userdata, void* object,
                                          float degrees) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Rotate(degrees);
  return 0;
}

inline textra_status_t TEXTRA_CALL Skew(void* userdata, void* object, float sx,
                                        float sy) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Skew(sx, sy);
  return 0;
}

inline textra_status_t TEXTRA_CALL ClipRect(void* userdata, void* object,
                                            float left, float top, float right,
                                            float bottom,
                                            textra_bool_t antialias) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->ClipRect(skity::Rect::MakeLTRB(left, top, right, bottom));
  return 0;
}

inline textra_status_t TEXTRA_CALL Clear(void* userdata, void* object) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->DrawColor(0, skity::BlendMode::kClear);
  return 0;
}

inline textra_status_t TEXTRA_CALL ClearRect(void* userdata, void* object,
                                             float left, float top, float right,
                                             float bottom) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->Save();
  canvas->ClipRect(skity::Rect::MakeLTRB(left, top, right, bottom));
  canvas->DrawColor(0, skity::BlendMode::kClear);
  canvas->Restore();
  return 0;
}

inline textra_status_t TEXTRA_CALL FillRect(void* userdata, void* object,
                                            float left, float top, float right,
                                            float bottom, uint32_t color) {
  auto canvas = static_cast<skity::Canvas*>(object);
  skity::Paint fill;
  fill.SetColor(color);
  canvas->DrawRect(skity::Rect::MakeLTRB(left, top, right, bottom), fill);
  return 0;
}

inline textra_status_t TEXTRA_CALL DrawColor(void* userdata, void* object,
                                             uint32_t color) {
  auto canvas = static_cast<skity::Canvas*>(object);
  canvas->DrawColor(color);
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawLine(void* userdata, void* object, float x1, float y1, float x2, float y2,
         const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  if (p.gradient) {
    native_paint.SetShader(nullptr);
    native_paint.SetColor(p.stroke_color);
  }
  canvas->DrawLine(x1, y1, x2, y2, native_paint);
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawRect(void* userdata, void* object, float left, float top, float right,
         float bottom, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawRect(skity::Rect::MakeLTRB(left, top, right, bottom), draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawOval(void* userdata, void* object, float left, float top, float right,
         float bottom, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawOval(skity::Rect::MakeLTRB(left, top, right, bottom), draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawCircle(void* userdata, void* object, float x, float y, float radius,
           const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawCircle(x, y, radius, draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawArc(void* userdata, void* object, float left, float top, float right,
        float bottom, float start_angle, float sweep_angle,
        textra_bool_t use_center, const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawArc(skity::Rect::MakeLTRB(left, top, right, bottom),
                    start_angle, sweep_angle, use_center != 0, draw);
  });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawPath(void* userdata, void* object, const textra_backend_path_t* path,
         const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  skity::Path native_path;
  auto status = Path(native_path, path);
  if (status) return status;
  Draw(p, native_paint,
       [&](const skity::Paint& draw) { canvas->DrawPath(native_path, draw); });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawArcTo(void* userdata, void* object, float start_x, float start_y,
          float mid_x, float mid_y, float end_x, float end_y, float radius,
          const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  skity::Path native_path;
  native_path.MoveTo(start_x, start_y);
  native_path.ArcTo(mid_x, mid_y, end_x, end_y, radius);
  Draw(p, native_paint,
       [&](const skity::Paint& draw) { canvas->DrawPath(native_path, draw); });
  return 0;
}

inline textra_status_t TEXTRA_CALL
DrawText(void* userdata, void* object, void* typeface, const char* text,
         uint32_t text_bytes, float x, float y,
         const textra_backend_paint_t* paint) {
  return TEXTRA_STATUS_UNSUPPORTED;
}

inline textra_status_t TEXTRA_CALL
DrawGlyphs(void* userdata, void* object, void* typeface, uint32_t glyph_count,
           const uint16_t* glyphs, const char* text, uint32_t text_bytes,
           float origin_x, float origin_y, float* x, float* y,
           const textra_backend_paint_t* paint) {
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  if (!typeface) return TEXTRA_STATUS_INVALID_ARGUMENT;
  auto font = Font(typeface, p.text_size);
  font.SetSubpixel(true);
  font.SetBaselineSnap(true);
  font.SetEmbolden(p.bold != 0);
  font.SetSkewX((p.italic ? -0.25f : 0) + p.text_skew);
  std::vector<skity::GlyphID> ids;
  if (glyph_count) ids.assign(glyphs, glyphs + glyph_count);
  std::vector<float> positions_x(glyph_count), positions_y(glyph_count);
  for (uint32_t i = 0; i < glyph_count; ++i) {
    positions_x[i] = x[i];
    positions_y[i] = -y[i];
  }
  std::vector<skity::TextRun> runs;
  runs.emplace_back(font, ids, positions_x, positions_y);
  auto blob = std::make_shared<skity::TextBlob>(runs);
  for (uint32_t i = 0; i < p.shadow_count; ++i) {
    auto shadow = native_paint;
    auto& s = p.shadows[i];
    shadow.SetShader(nullptr);
    shadow.SetColor(s.color);
    shadow.SetMaskFilter(s.blur_radius ? skity::MaskFilter::MakeBlur(
                                             skity::BlurStyle::kNormal,
                                             static_cast<float>(s.blur_radius))
                                       : nullptr);
    canvas->DrawTextBlob(blob, origin_x + s.offset_x, origin_y + s.offset_y,
                         shadow);
  }
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawTextBlob(blob, origin_x, origin_y, draw);
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
  auto canvas = static_cast<skity::Canvas*>(object);
  if (!paint) return TEXTRA_STATUS_INVALID_ARGUMENT;
  const auto& p = *paint;
  auto native_paint = Paint(p);
  Draw(p, native_paint, [&](const skity::Paint& draw) {
    canvas->DrawRoundRect(skity::Rect::MakeLTRB(left, top, right, bottom),
                          radius, radius, draw);
  });
  return 0;
}

const textra_backend_canvas_api_t kCanvasApi = {
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

const textra_backend_font_manager_api_t kFontManagerApi = {
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

const textra_backend_typeface_api_t kTypefaceApi = {
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
}  // namespace
}  // namespace skity_coretext_backend
}  // namespace textra_native
extern "C" textra_status_t TEXTRA_CALL
textra_skity_coretext_canvas_callbacks_initialize(
    textra_canvas_options_t* options) {
  if (!options || options->struct_size < sizeof(*options) || options->flags ||
      !options->native_canvas)
    return TEXTRA_STATUS_INVALID_ARGUMENT;
  options->backend = TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT;
  options->callbacks = &textra_native::skity_coretext_backend::kCanvasApi;
  return TEXTRA_STATUS_OK;
}
extern "C" textra_status_t TEXTRA_CALL
textra_skity_coretext_font_manager_callbacks_initialize(
    textra_font_manager_options_t* options) {
  namespace native = textra_native::skity_coretext_backend;
  return textra_native::InitializeManager(options, native::kFontManagerApi,
                                          native::kTypefaceApi,
                                          native::CreateManager);
}
extern "C" textra_status_t TEXTRA_CALL
textra_skity_coretext_typeface_callbacks_initialize(
    textra_typeface_options_t* options) {
  namespace native = textra_native::skity_coretext_backend;
  return textra_native::InitializeTypeface(
      options, TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT, native::kTypefaceApi,
      native::Import);
}
