// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_TEXTLAYOUT_SHAPER_CACHE_H_
#define SRC_TEXTLAYOUT_SHAPER_CACHE_H_

#include "src/textlayout/shape_cache.h"

#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
#include "src/ports/shaper/coretext/font_cache.h"
#endif

namespace ttoffice {
namespace tttext {

struct ShaperCache {
  explicit ShaperCache(bool thread_safe)
      : shape_cache_(thread_safe)
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
        ,
        system_font_cache_(thread_safe),
        font_cache_(thread_safe)
#endif
  {
  }

  void ClearShapeCache() { shape_cache_.Clear(); }

  void ClearFontCache() {
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
    system_font_cache_.Clear();
    font_cache_.Clear();
#endif
  }

  ShapeCache shape_cache_;
#if defined(ENABLE_CTSHAPER) || defined(ENABLE_CTSHAPER_SKITY)
  // The system and self-rendering shapers use different font cascades.
  CoreTextFontCache system_font_cache_;
  CoreTextFontCache font_cache_;
#endif
};

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_TEXTLAYOUT_SHAPER_CACHE_H_
