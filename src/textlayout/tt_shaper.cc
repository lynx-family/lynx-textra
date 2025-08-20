// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/textlayout/tt_shaper.h"

#ifdef ENABLE_SKSHAPER
#include "shaper/skshaper/shaper_skshaper.h"
#endif
#ifdef ENABLE_JAVASHAPER
#include "shaper/java/java_shaper.h"
#endif
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
#include "src/ports/shaper/coretext/shaper_core_text.h"
#endif
#if defined(ENABLE_OHOS)
#include "src/ports/shaper/ark_graphics/shaper_ark_graphics.h"
#endif
#include <textra/macro.h>

#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/run/base_run.h"
#include "src/textlayout/shape_cache.h"
#include "src/textlayout/style/style_manager.h"
#include "src/textlayout/utils/log_util.h"
#ifdef USE_ICU
#include <textra/icu_wrapper.h>
#endif
namespace ttoffice {
namespace tttext {
std::unique_ptr<TTShaper> TTShaper::CreateShaper(
    FontmgrCollection* font_collection, ShaperType type) {
  switch (type) {
    case kSystem: {
#ifdef ENABLE_JAVASHAPER
      return std::make_unique<JavaShaper>(*font_collection);
#endif
#ifdef ENABLE_CTSHAPER
      return std::make_unique<ShaperCoreText>(*font_collection);
#endif
#ifdef ENABLE_OHOS
      return std::make_unique<ShaperArkGraphics>(*font_collection);
#endif
      break;
    }
    case kSelfRendering: {
#ifdef ENABLE_SKSHAPER
      return std::make_unique<ShaperSkShaper>(*font_collection);
      break;
#endif
#ifdef ENABLE_CTSHAPER
      return std::make_unique<ShaperCoreText>(*font_collection);
      break;
#endif
      break;
    }
    default:
      break;
  }
  TTASSERT(false);
  return nullptr;
}
TTShaper::TTShaper(FontmgrCollection font_collection) noexcept
    : font_collection_(font_collection) {}
TTShaper::~TTShaper() = default;
void TTShaper::ProcessBidirection(const char32_t* text, uint32_t length,
                                  WriteDirection write_direction,
                                  uint32_t* visual_map, uint32_t* logical_map,
                                  uint8_t* dir_vec) {
#ifdef USE_ICU
  ICUWrapper::BidiInit(text, length, write_direction, dir_vec, visual_map,
                       logical_map);
#else
  for (size_t i = 0; i < length; i++) {
    visual_map[i] = static_cast<uint32_t>(i);
    logical_map[i] = static_cast<uint32_t>(i);
    dir_vec[i] = 0;
  }
#endif
}
ShapeResultRef TTShaper::ShapeText(const char32_t* text, uint32_t length,
                                   const ShapeStyle* shape_style,
                                   bool rtl) const {
  const ShapeKey key(text, length, shape_style, rtl);
  auto result = ShapeCache::GetInstance().Find(key);
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
    ShapeCache::GetInstance().AddToCache(key, result);
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
  uint32_t k = 0;
  float width = 0;
  uint32_t prev_glyph_id = -1;
  while (k < char_count) {
    TTASSERT(k + start_char < CharCount());
    auto glyph_id = CharToGlyph(k + start_char);
    TTASSERT(glyph_id < GlyphCount());
    k++;
    if (glyph_id == prev_glyph_id) {
      continue;
    }
    auto adv = Advances(glyph_id);
    if (FloatsLarger(adv[0], 0)) {
      width += adv[0] + letter_spacing;
    }
    prev_glyph_id = glyph_id;
  }
  return width;
}
}  // namespace tttext
}  // namespace ttoffice
