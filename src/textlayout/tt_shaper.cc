// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/tt_shaper.h"

#include "src/ports/rendering_backend_internal.h"

#ifdef ENABLE_SKSHAPER
#include "shaper/skshaper/shaper_skshaper.h"
#endif
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
#include "shaper/java/java_shaper.h"
#endif
#if defined(ENABLE_CTSHAPER)
#include "src/ports/shaper/coretext/shaper_core_text.h"
#include "src/ports/shaper/coretext/shaper_core_text_self_rendering.h"
#endif
#if defined(TEXTRA_ENABLE_OHOS_BACKEND)
#include "src/ports/shaper/ark_graphics/shaper_ark_graphics.h"
#endif
#ifdef TEXTRA_ENABLE_LVGL_BACKEND
#include "src/ports/shaper/lvgl/lvgl_shaper.h"
#endif
#include <textra/macro.h>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/tttext_context_impl.h"
#include "src/textlayout/utils/log_util.h"
namespace ttoffice {
namespace tttext {
std::unique_ptr<TTShaper> TTShaper::CreateShaper(
    FontmgrCollection* font_collection, RenderingBackend backend) {
  if (!font_collection || !internal::IsBackendCompiled(backend)) return nullptr;
  auto create = [&]() -> std::unique_ptr<TTShaper> {
    switch (backend) {
#ifdef TEXTRA_ENABLE_ANDROID_BACKEND
      case RenderingBackend::kAndroid:
        return std::make_unique<JavaShaper>(*font_collection);
#endif
#ifdef ENABLE_CTSHAPER
      case RenderingBackend::kIOS:
      case RenderingBackend::kMacOS:
        return std::make_unique<ShaperCoreText>(*font_collection);
      case RenderingBackend::kIOSSelfRendering:
      case RenderingBackend::kMacOSSelfRendering:
      case RenderingBackend::kSkityCoreText:
        return std::make_unique<ShaperCoreTextSelfRendering>(*font_collection);
#endif
#ifdef TEXTRA_ENABLE_OHOS_BACKEND
      case RenderingBackend::kOHOS:
        return std::make_unique<ShaperArkGraphics>(*font_collection);
#endif
#ifdef TEXTRA_ENABLE_LVGL_BACKEND
      case RenderingBackend::kLVGL:
        return std::make_unique<LVGLShaper>(*font_collection);
#endif
#ifdef ENABLE_SKSHAPER
      case RenderingBackend::kSkia:
      case RenderingBackend::kSkity:
        if (ShaperSkShaper::Preload())
          return std::make_unique<ShaperSkShaper>(*font_collection);
        break;
#endif
      default:
        break;
    }
    return nullptr;
  };
  auto shaper = create();
  if (shaper) shaper->backend_ = backend;
  return shaper;
}

bool TTShaper::Preload(RenderingBackend backend) {
  if (!internal::IsBackendCompiled(backend)) return false;
#ifdef ENABLE_SKSHAPER
  if (backend == RenderingBackend::kSkia || backend == RenderingBackend::kSkity)
    return ShaperSkShaper::Preload();
#endif
  return true;
}

void TTShaper::ClearFontCache() {
#if defined(ENABLE_CTSHAPER)
  ShaperCoreTextSelfRendering::ClearSafeFontCache();
#endif
}

TTShaper::TTShaper(FontmgrCollection font_collection) noexcept
    : font_collection_(font_collection), context_(nullptr) {}
TTShaper::~TTShaper() = default;
void TTShaper::ProcessBidirection(const char32_t* text, uint32_t length,
                                  WriteDirection write_direction,
                                  uint32_t* visual_map, uint32_t* logical_map,
                                  uint8_t* dir_vec) {
  for (size_t i = 0; i < length; i++) {
    visual_map[i] = static_cast<uint32_t>(i);
    logical_map[i] = static_cast<uint32_t>(i);
    dir_vec[i] = 0;
  }
}
ShapeResultRef TTShaper::ShapeText(const char32_t* text, uint32_t length,
                                   const ShapeStyle* shape_style,
                                   bool rtl) const {
  const ShapeKey key(text, length, shape_style, rtl, backend_,
                     font_collection_.typeface_cache_,
                     context_ && context_->IsAndroidMetricsRound());
  const auto disable_shape_cache =
      context_ != nullptr && context_->GetImpl().IsShapeCacheDisabled();
  ShapeCache::Epoch cache_epoch = 0;
  auto result = disable_shape_cache
                    ? nullptr
                    : ShapeCache::GetInstance().Find(key, &cache_epoch);
  if (result == nullptr) {
    result = std::make_shared<ShapeResult>(length, rtl);
    OnShapeText(key, result.get());
    TTASSERT(result->GlyphCount() > 0);

    for (auto k = 0u; k < length; k++) {
      if (text[k] < 32) {
        result->advances_[result->CharToGlyph(k)][0] = 0;
        result->advances_[result->CharToGlyph(k)][1] = 0;
      }
    }
    if (!disable_shape_cache) {
      ShapeCache::GetInstance().AddToCache(key, result, cache_epoch);
    }
  }
  TTASSERT(result != nullptr);
  return result;
}
void ShapeResult::AppendPlatformShapingResult(
    const PlatformShapingResultReader& reader) {
  ReserveGlyphCount(reader.GlyphCount());
  auto index = 0u;
  auto glyph_count = reader.GlyphCount();
  while (index < glyph_count) {
    auto idx = index;
    indices_[idx] = reader.ReadIndices(index);
    glyphs_[idx] = reader.ReadGlyphID(index);
    font_[idx] = reader.ReadFontId(index);
    advances_[idx][0] = reader.ReadAdvanceX(index);
    advances_[idx][1] = reader.ReadAdvanceY(index);
    position_[idx][0] = reader.ReadPositionX(index);
    position_[idx][1] = reader.ReadPositionY(index);
    index++;
  }

  c2glyph_indices_.resize(reader.TextCount(), -1u);

  for (auto k = 0u; k < glyph_count; k++) {
    auto c_idx = indices_[k];
    TTASSERT(0 <= c_idx && c_idx <= c2glyph_indices_.size());
    if (c2glyph_indices_[c_idx] == -1u) {
      c2glyph_indices_[c_idx] = k;
    }
  }
  for (auto k = 0u; k < reader.TextCount(); k++) {
    if (c2glyph_indices_[k] == -1u) {
      TTASSERT(k > 0);
      c2glyph_indices_[k] = c2glyph_indices_[k - 1];
    }
  }
}
float ShapeResult::MeasureWidth(uint32_t start_char, uint32_t char_count,
                                float letter_spacing) const {
  if (char_count == 0) {
    return 0;
  }

  TTASSERT(start_char < CharCount());
  TTASSERT(start_char + char_count <= CharCount());

  const uint32_t glyph_start = CharToGlyph(start_char);
  const uint32_t glyph_end = CharToGlyph(start_char + char_count);
  TTASSERT(glyph_start <= glyph_end);
  TTASSERT(glyph_end <= GlyphCount());

  float width = 0;
  uint32_t prev_char_idx = static_cast<uint32_t>(-1);
  for (uint32_t glyph_id = glyph_start; glyph_id < glyph_end; ++glyph_id) {
    const auto adv = Advances(glyph_id);
    if (FloatsLarger(adv[0], 0)) {
      width += adv[0];
      const uint32_t char_idx = GlyphToChar(glyph_id);
      if (char_idx != prev_char_idx) {
        width += letter_spacing;
      }
    }
    prev_char_idx = GlyphToChar(glyph_id);
  }
  return width;
}
}  // namespace tttext
}  // namespace ttoffice
