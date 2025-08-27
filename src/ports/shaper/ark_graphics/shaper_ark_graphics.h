// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_ARK_GRAPHICS_SHAPER_ARK_GRAPHICS_H_
#define SRC_PORTS_SHAPER_ARK_GRAPHICS_SHAPER_ARK_GRAPHICS_H_
#include <native_drawing/drawing_font_collection.h>

#include "src/textlayout/tt_shaper.h"

namespace ttoffice {
namespace tttext {
class ShaperArkGraphics : public TTShaper {
 public:
  ShaperArkGraphics() = delete;
  explicit ShaperArkGraphics(
      const FontmgrCollection& font_collections) noexcept;
  ~ShaperArkGraphics() override;

 public:
  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;
  void ShapingTextWithHighAPILevel(const ShapeKey& key,
                                   ShapeResult* result) const;
  void ShapingTextWithLowAPILevel(const ShapeKey& key,
                                  ShapeResult* result) const;

 private:
  OH_Drawing_FontCollection* shared_font_collection_;
  OH_Drawing_TypographyStyle* typography_style_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_ARK_GRAPHICS_SHAPER_ARK_GRAPHICS_H_
