// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#ifndef PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_TYPEFACE_IMPL_HPP_
#define PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_TYPEFACE_IMPL_HPP_

#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkStream.h>
#include <include/core/SkTypeface.h>
#include <textra/capi/textra_platform.h>

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
inline textra_status_t TEXTRA_CALL Import(void*, const void* p, void** out) {
  *out = nullptr;

  if (!p) return TEXTRA_STATUS_INVALID_ARGUMENT;
  return Wrap(sk_ref_sp(static_cast<SkTypeface*>(const_cast<void*>(p))), out);
}
inline void TEXTRA_CALL DestroyFace(void*, void* p) {
  delete static_cast<Face*>(p);
}
inline textra_status_t TEXTRA_CALL Native(void*, void* p, const void** out) {
  *out = static_cast<Face*>(p)->font.get();
  return 0;
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
inline const textra_backend_typeface_api_t& FaceApi() {
  static const textra_backend_typeface_api_t kTypefaceApi = {
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
  return kTypefaceApi;
}
}  // namespace skia_backend
}  // namespace textra_native
#endif  // PUBLIC_TEXTRA_CAPI_PLATFORM_SKIA_TEXTRA_SKIA_TYPEFACE_IMPL_HPP_
