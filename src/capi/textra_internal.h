// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef SRC_CAPI_TEXTRA_INTERNAL_H_
#define SRC_CAPI_TEXTRA_INTERNAL_H_

#include <textra/capi/textra.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>
#include <textra/layout_drawer.h>
#include <textra/paragraph.h>
#include <textra/platform_helper.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/capi/textra_backend_adapters.h"
#include "src/capi/textra_platform_internal.h"

namespace textra_capi {
namespace tt = ttoffice::tttext;
struct RefCounted {
  std::atomic<uint32_t> references{1};
  virtual ~RefCounted() = default;
};
template <class T>
void Retain(T* p) {
  if (p) p->references.fetch_add(1, std::memory_order_relaxed);
}
template <class T>
void Release(T* p) {
  if (p && p->references.fetch_sub(1, std::memory_order_acq_rel) == 1) delete p;
}
template <class T>
class Ref {
 public:
  Ref() = default;
  static Ref Adopt(T* p) {
    Ref result;
    result.p_ = p;
    return result;
  }
  explicit Ref(T* p) : p_(p) { Retain(p_); }
  Ref(const Ref& r) : Ref(r.p_) {}
  Ref(Ref&& r) noexcept : p_(std::exchange(r.p_, nullptr)) {}
  ~Ref() { Release(p_); }
  Ref& operator=(Ref r) noexcept {
    std::swap(p_, r.p_);
    return *this;
  }
  T* get() const { return p_; }
  T* operator->() const { return p_; }
  explicit operator bool() const { return p_ != nullptr; }

 private:
  T* p_ = nullptr;
};
inline uint32_t StructSize(const void* p) {
  uint32_t size = 0;
  if (p) std::memcpy(&size, p, sizeof(size));
  return size;
}
template <class T>
bool ValidStruct(const T* p) {
  return p && StructSize(p) >= sizeof(uint32_t);
}
template <class T, class M>
M Read(const T* p, size_t offset, M fallback) {
  if (!ValidStruct(p) || StructSize(p) < offset + sizeof(M)) return fallback;
  M value;
  std::memcpy(&value, reinterpret_cast<const char*>(p) + offset, sizeof(M));
  return value;
}
template <class T, class M>
void Write(T* p, size_t offset, const M& value) {
  if (StructSize(p) >= offset + sizeof(M))
    std::memcpy(reinterpret_cast<char*>(p) + offset, &value, sizeof(M));
}
#define TEXTRA_READ(p, field, fallback)                                       \
  ::textra_capi::Read(p, offsetof(std::remove_pointer_t<decltype(p)>, field), \
                      fallback)
#define TEXTRA_WRITE(p, field, value)                                          \
  ::textra_capi::Write(p, offsetof(std::remove_pointer_t<decltype(p)>, field), \
                       value)
inline bool Bool(textra_bool_t b) { return b <= 1; }
inline bool Finite(float f) { return std::isfinite(f); }
inline bool Nonnegative(float f) { return Finite(f) && f >= 0; }
textra_status_t CopyString(const std::string& value, char* buffer,
                           uint32_t capacity, uint32_t* size);

struct Operation {
  textra_status_t error = TEXTRA_STATUS_OK;
  textra_canvas_t* canvas = nullptr;
};
extern thread_local Operation* operation;
struct OperationScope {
  Operation local;
  Operation* previous;
  explicit OperationScope(textra_canvas_t* canvas = nullptr)
      : previous(operation) {
    local.canvas = canvas;
    operation = &local;
  }
  ~OperationScope() { operation = previous; }
};
inline void Fail(textra_status_t status) {
  if (operation && operation->error == TEXTRA_STATUS_OK)
    operation->error = status;
}
struct Active {
  bool& value;
  explicit Active(bool& flag) : value(flag) { value = true; }
  ~Active() { value = false; }
};
}  // namespace textra_capi

struct textra_canvas : textra_capi::RefCounted {
  std::unique_ptr<textra_capi::tt::ICanvasHelper> impl;
  uint32_t backend = 0;
  bool active = false;
};
struct textra_typeface : textra_capi::RefCounted {
  std::shared_ptr<textra_capi::tt::IFontManager> manager;
  std::shared_ptr<textra_capi::tt::ITypefaceHelper> impl;
  uint32_t backend = 0;
  void* native = nullptr;
};

struct textra_font_descriptor : textra_capi::RefCounted {
  textra_capi::tt::FontDescriptor impl;
  textra_capi::Ref<textra_typeface> typeface;
};
struct textra_resolved_gradient : textra_capi::RefCounted {
  std::shared_ptr<const textra_capi::tt::ResolvedGradient> impl;
};
struct textra_style : textra_capi::RefCounted {
  textra_capi::tt::Style impl;
  textra_capi::Ref<textra_typeface> typeface;
};
namespace textra_capi {
class Delegate final : public tt::RunDelegate {
 public:
  textra_run_callbacks_t callbacks{};
  void* userdata = nullptr;
  bool adopted = false;
  textra_run_metrics_t metrics{sizeof(textra_run_metrics_t), 0, 0, 0};
  ~Delegate() override;
  textra_status_t Measure();
  float GetAscent() const override { return metrics.ascent; }
  float GetDescent() const override { return metrics.descent; }
  float GetAdvance() const override { return metrics.advance; }
  void Layout() override { Fail(Measure()); }
  void Draw(tt::ICanvasHelper*, float x, float y) override;
};
}  // namespace textra_capi
struct textra_run_delegate : textra_capi::RefCounted {
  std::shared_ptr<textra_capi::Delegate> impl;
};
struct textra_paragraph_style : textra_capi::RefCounted {
  textra_capi::tt::ParagraphStyle impl;
  textra_capi::Ref<textra_typeface> typeface;
  textra_capi::Ref<textra_run_delegate> ellipsis;
  bool break_allowed = false;
};
struct textra_text_layout : textra_capi::RefCounted {
  textra_capi::tt::FontmgrCollection fonts;
  std::unique_ptr<textra_capi::tt::TextLayout> impl;
  bool active = false;
  uint32_t backend = 0;
};
namespace textra_capi {
struct StyleSpan {
  uint32_t start, end;
  tt::Style style;
};
void Overlay(tt::Style& destination, const tt::Style& source);
tt::Style EffectiveStyle(const textra_paragraph*, uint32_t index);
}  // namespace textra_capi
struct textra_paragraph : textra_capi::RefCounted {
  std::unique_ptr<textra_capi::tt::Paragraph> impl;
  textra_capi::Ref<textra_paragraph_style> style;
  std::vector<textra_capi::Ref<textra_typeface>> typefaces;
  std::vector<std::shared_ptr<textra_capi::Delegate>> delegates;
  textra_capi::Ref<textra_text_layout> single_engine;
  uint32_t locks = 0;
  std::vector<textra_capi::StyleSpan> spans;
  std::vector<std::vector<textra_capi::StyleSpan>> saved_spans;
  bool valid_layout = false;
  const void* metrics_region = nullptr;
  std::shared_ptr<uint32_t> single_results;
  ~textra_paragraph();
  bool has_text = false;
};
struct textra_shaper_cache {
  std::shared_ptr<textra_capi::tt::ShaperCache> impl;
};
struct textra_text_context : textra_capi::RefCounted {
  textra_capi::tt::TTTextContext impl;
  std::array<bool, 7> features{{true, false, true, false, false, false, false}};
  textra_capi::Ref<textra_paragraph> paragraph;
  textra_capi::Ref<textra_text_layout> engine;
  std::shared_ptr<uint32_t> results = std::make_shared<uint32_t>(0);
  uint32_t position = 0;
  bool active = false;
  bool used = false;
  bool locked = false;
  bool failed = false;
  bool bottom_explicit = false;
  std::weak_ptr<int> previous_region;
  ~textra_text_context();
  void Unbind();
};
namespace textra_capi {
struct Dependency {
  Ref<textra_paragraph> paragraph;
  Ref<textra_text_layout> engine;
  std::shared_ptr<uint32_t> results;
  Ref<textra_text_context> context;
  Dependency(textra_paragraph* p, textra_text_layout* e, textra_text_context* c)
      : paragraph(p), engine(e), results(c->results), context(c) {
    ++p->locks;
    ++*results;
  }
  ~Dependency() {
    --paragraph->locks;
    --*results;
  }
};
class Region;
}  // namespace textra_capi
struct textra_layout_region : textra_capi::RefCounted {
  std::shared_ptr<int> identity = std::make_shared<int>(0);
  textra_layout_page_callbacks_t callbacks{};
  void* userdata = nullptr;
  bool adopted = false;
  bool active = false;
  float width = 0, height = 0, pitch = 0;
  uint32_t width_mode = 0, height_mode = 0;
  std::vector<std::unique_ptr<textra_capi::Dependency>> dependencies;
  std::unique_ptr<textra_capi::Region> impl;
  ~textra_layout_region();
};
namespace textra_capi {
class PageListener final : public tt::LayoutPageListener {
 public:
  textra_layout_region* owner;
  explicit PageListener(textra_layout_region* o) : owner(o) {}
  void OnLineLayouted(tt::LayoutRegion*, uint32_t, bool, float,
                      tt::LayoutResult*) override;
};
class Region final : public tt::LayoutRegion {
 public:
  textra_layout_region* owner;
  PageListener listener;
  explicit Region(textra_layout_region* o)
      : tt::LayoutRegion(
            o->width, o->height, static_cast<tt::LayoutMode>(o->width_mode),
            static_cast<tt::LayoutMode>(o->height_mode), &listener),
        owner(o),
        listener(o) {
    SetLinePitch(o->pitch);
  }
  std::vector<std::array<float, 2>> GetRangeList(float*, float, float,
                                                 float) override;
};
class DrawerListener final : public tt::LayoutDrawerListener {
 public:
  textra_layout_drawer* owner = nullptr;
  uint32_t FetchThemeColor(tt::ThemeCategory, const tt::TTColor&) override;
};
}  // namespace textra_capi
struct textra_layout_drawer : textra_capi::RefCounted {
  textra_capi::Ref<textra_canvas> canvas;
  textra_layout_drawer_callbacks_t callbacks{};
  void* userdata = nullptr;
  bool adopted = false;
  bool active = false;
  textra_capi::DrawerListener listener;
  ~textra_layout_drawer();
};
namespace textra_capi {
textra_status_t Editable(textra_paragraph* p, bool general = false);
void Invalidate(textra_paragraph* p);
std::unique_ptr<textra_paragraph_style> Clone(const textra_paragraph_style*);
std::unique_ptr<textra_style> Clone(const textra_style*);
textra_status_t CheckFonts(const textra_paragraph*, const textra_text_layout*);
textra_status_t Draw(textra_layout_drawer*, const textra_layout_region*,
                     const textra_paragraph*, uint32_t line,
                     const textra_text_range_t*, float x, float y);
}  // namespace textra_capi
#endif  // SRC_CAPI_TEXTRA_INTERNAL_H_
