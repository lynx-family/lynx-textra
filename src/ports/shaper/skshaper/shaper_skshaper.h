// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_SKSHAPER_SHAPER_SKSHAPER_H_
#define SRC_PORTS_SHAPER_SKSHAPER_SHAPER_SKSHAPER_H_

#include <textra/fontmgr_collection.h>
#include <textra/i_typeface_helper.h>

#include <map>
#include <memory>
#include <string>

#include "src/ports/shaper/skshaper/one_line_shaper.h"
#include "src/textlayout/tt_shaper.h"
namespace ttoffice {
namespace tttext {

class OneLineShaper;

using TTFont = std::shared_ptr<ITypefaceHelper>;

class ShaperSkShaper : public TTShaper {
 public:
  explicit ShaperSkShaper(FontmgrCollection& font_collections);
  ~ShaperSkShaper() override;

 public:
  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override;

 protected:
  std::unique_ptr<OneLineShaper> shaper_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_SKSHAPER_SHAPER_SKSHAPER_H_
