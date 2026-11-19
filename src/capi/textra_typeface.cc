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
std::atomic<uint64_t> next_id{0x40000000u};
}  // namespace

CallbackTypeface::CallbackTypeface(
    const textra_backend_typeface_api_t& callbacks, void* userdata,
    void* native_font, uint32_t id, bool native_tokens)
    : ITypefaceHelper(id),
      callbacks_(callbacks),
      userdata_(userdata),
      native_font_(native_font),
      native_tokens_(native_tokens) {
  textra_font_style_t s{TEXTRA_FONT_WEIGHT_NORMAL, TEXTRA_FONT_WIDTH_NORMAL,
                        TEXTRA_FONT_SLANT_UPRIGHT};
  if (callbacks_.font_style(Userdata(), NativeObject(), &s) ==
      TEXTRA_STATUS_OK) {
    if (s.weight > TEXTRA_FONT_WEIGHT_EXTRA_BLACK ||
        s.width < TEXTRA_FONT_WIDTH_ULTRA_CONDENSED ||
        s.width > TEXTRA_FONT_WIDTH_ULTRA_EXPANDED ||
        s.slant > TEXTRA_FONT_SLANT_OBLIQUE) {
      return;
    }
    font_style_ = tttext::FontStyle(static_cast<Weight>(s.weight),
                                    static_cast<Width>(s.width),
                                    static_cast<Slant>(s.slant));
  }
}

CallbackTypeface::~CallbackTypeface() {
  callbacks_.destroy(userdata_, native_font_);
}

void* CallbackTypeface::Userdata() const { return userdata_; }

void* CallbackTypeface::NativeObject() const { return native_font_; }

void* CallbackTypeface::PlatformFont() const {
  return native_tokens_ ? const_cast<void*>(Native())
                        : const_cast<CallbackTypeface*>(this);
}

float CallbackTypeface::GetHorizontalAdvance(GlyphID g, float z) const {
  float v = 0;
  callbacks_.horizontal_advance(Userdata(), NativeObject(), g, z, &v);
  if (!std::isfinite(v)) {
    return 0;
  }
  return v;
}

void CallbackTypeface::GetHorizontalAdvances(GlyphID* g, uint32_t n, float* v,
                                             float z) const {
  if (n) std::fill(v, v + n, 0);
  callbacks_.horizontal_advances(Userdata(), NativeObject(), g, n, z, v);
  for (uint32_t i = 0; i < n; ++i)
    if (!std::isfinite(v[i])) {
      v[i] = 0;
    }
}

void CallbackTypeface::GetWidthBound(float* v, GlyphID g, float z) const {
  textra_rect_t r{};
  callbacks_.width_bound(Userdata(), NativeObject(), g, z, &r);
  if (!std::isfinite(r.left) || !std::isfinite(r.top) ||
      !std::isfinite(r.right) || !std::isfinite(r.bottom)) {
    r = {};
  }
  v[0] = r.left;
  v[1] = r.top;
  v[2] = r.right;
  v[3] = r.bottom;
}

void CallbackTypeface::GetWidthBounds(float* v, GlyphID* g, uint32_t n,
                                      float z) {
  textra_rect_t r{};
  callbacks_.width_bounds(Userdata(), NativeObject(), g, n, z, &r);
  if (!std::isfinite(r.left) || !std::isfinite(r.top) ||
      !std::isfinite(r.right) || !std::isfinite(r.bottom)) {
    r = {};
  }
  v[0] = r.left;
  v[1] = r.top;
  v[2] = r.right;
  v[3] = r.bottom;
}

const void* CallbackTypeface::GetFontData() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  if (!data_loaded_) {
    const void* value = nullptr;
    if (callbacks_.font_data(Userdata(), NativeObject(), &value) ==
        TEXTRA_STATUS_OK) {
      data_ = value;
      data_loaded_ = true;
    }
  }
  return data_;
}

size_t CallbackTypeface::GetFontDataSize() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  if (!data_size_loaded_) {
    uint64_t value = 0;
    if (callbacks_.font_data_size(Userdata(), NativeObject(), &value) ==
            TEXTRA_STATUS_OK &&
        value <= std::numeric_limits<size_t>::max() && value <= UINT32_MAX) {
      data_size_ = static_cast<size_t>(value);
      data_size_loaded_ = true;
    }
  }
  return data_size_;
}

int CallbackTypeface::GetFontIndex() const {
  int32_t v = 0;
  callbacks_.font_index(Userdata(), NativeObject(), &v);
  return v;
}

uint16_t CallbackTypeface::UnicharToGlyph(Unichar c, uint32_t s) const {
  uint16_t v = 0;
  callbacks_.unichar_to_glyph(Userdata(), NativeObject(), c, s, &v);
  return v;
}

void CallbackTypeface::UnicharsToGlyphs(const Unichar* c, uint32_t n,
                                        GlyphID* v) const {
  if (n) std::fill(v, v + n, 0);
  callbacks_.unichars_to_glyphs(Userdata(), NativeObject(), c, n, v);
}

uint32_t CallbackTypeface::GetUnitsPerEm() const {
  uint32_t v = 0;
  callbacks_.units_per_em(Userdata(), NativeObject(), &v);
  return v;
}

const void* CallbackTypeface::Native() const {
  const void* p = nullptr;
  callbacks_.native_font(Userdata(), NativeObject(), &p);
  return p;
}

void CallbackTypeface::OnCreateFontInfo(FontInfo* out, float z) const {
  textra_backend_font_metrics_t v{sizeof(v), 0, 0, 0, 0, 0, z};
  if (callbacks_.font_info(Userdata(), NativeObject(), z, &v)) {
    return;
  }
  if (!std::isfinite(v.ascent) || !std::isfinite(v.descent) ||
      !std::isfinite(v.top) || !std::isfinite(v.bottom) ||
      !std::isfinite(v.leading) || !std::isfinite(v.font_size) ||
      v.font_size <= 0 || v.ascent > v.descent) {
    return;
  }
  out->SetTop(v.top);
  out->SetAscent(v.ascent);
  out->SetDescent(v.descent);
  out->SetBottom(v.bottom);
  out->SetLeading(v.leading);
  out->SetFontSize(v.font_size);
}

void CallbackTypeface::OnCreateFontEmHeight(FontEmHeight* out, float z) const {
  if (!callbacks_.em_height) {
    ITypefaceHelper::OnCreateFontEmHeight(out, z);
    return;
  }
  textra_backend_em_height_t v{sizeof(v), 0, 0};
  if (callbacks_.em_height(Userdata(), NativeObject(), z, &v) ||
      !std::isfinite(v.ascent) || !std::isfinite(v.descent)) {
    return;
  }
  out->SetAscent(v.ascent);
  out->SetDescent(v.descent);
}

namespace {
std::shared_ptr<CallbackTypeface> Wrap(bool native_tokens,
                                       const textra_backend_typeface_api_t& api,
                                       void* userdata, void* native,
                                       bool consume_failure = true) {
  if (!native) return nullptr;
  auto id = next_id.fetch_add(1);
  if (id > UINT32_MAX) {
    if (consume_failure) api.destroy(userdata, native);
    return nullptr;
  }
  return std::make_shared<CallbackTypeface>(
      api, userdata, native, static_cast<uint32_t>(id), native_tokens);
}
}  // namespace

std::shared_ptr<CallbackTypeface> CallbackFontManager::AdoptTypeface(
    void* face, textra_status_t status) const {
  if (status) {
    if (face) typeface_callbacks_.destroy(userdata_, face);
    return nullptr;
  }
  return Wrap(native_tokens_, typeface_callbacks_, userdata_, face);
}
std::shared_ptr<CallbackTypeface> CreateTypeface(
    const textra_typeface_options_t& options) {
  textra_backend_typeface_api_t api{};
  CopyCallbacks(api, options.callbacks);
  return Wrap(NativeFontTokens(options.backend), api, options.userdata,
              const_cast<void*>(options.native_font), false);
}
}  // namespace backend
}  // namespace tttext
}  // namespace ttoffice

textra_status_t TEXTRA_CALL textra_typeface_create(
    const textra_typeface_options_t* options, textra_typeface_t** out) {
  *out = nullptr;
  auto object = std::make_unique<textra_typeface>();
  object->backend =
      static_cast<uint32_t>(tt::backend::ResolveBackend(options->backend));
  if (options->callbacks) {
    auto resolved = *options;
    resolved.backend = object->backend;
    object->impl = tt::PlatformHelper::CreateTypefaceHelper(resolved);
    if (!object->impl) return TEXTRA_STATUS_UNSUPPORTED;
    object->native =
        static_cast<tt::backend::CallbackTypeface*>(object->impl.get())
            ->PlatformFont();
  } else {
    if (options->flags) return TEXTRA_STATUS_UNSUPPORTED;
    object->manager = tt::PlatformHelper::CreateFontManager(
        static_cast<tt::RenderingBackend>(object->backend));
    if (!object->manager) return TEXTRA_STATUS_UNSUPPORTED;
    object->impl = tt::PlatformHelper::CreateTypefaceHelper(
        static_cast<tt::RenderingBackend>(object->backend),
        options->native_font, object->manager);
    if (!object->impl) return TEXTRA_STATUS_UNSUPPORTED;
    object->native =
        object->backend == TEXTRA_RENDERING_BACKEND_ANDROID
            ? reinterpret_cast<void*>(
                  static_cast<uintptr_t>(object->impl->GetUniqueId()))
            : object->manager->getPlatformFontFromTypeface(object->impl);
  }
  *out = object.release();
  return TEXTRA_STATUS_OK;
}
