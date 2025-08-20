// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_SKSHAPER_RUN_H_
#define SRC_PORTS_SHAPER_SKSHAPER_RUN_H_

#include <textra/font_info.h>
#include <textra/i_typeface_helper.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "src/ports/shaper/skshaper/font.h"
#include "src/ports/shaper/skshaper/sk_shaper.h"
#include "src/textlayout/utils/tt_range.h"

namespace ttoffice {
namespace tttext {

using ClusterIndex = uint32_t;
using GraphemeIndex = uint32_t;
using GlyphIndex = uint32_t;
using GlyphRange = Range;
using TextRange = Range;
using TextIndex = uint32_t;
using SkScalar = float;

class Run {
 public:
  Run(const char32_t* content_, const SkShaper::RunHandler::RunInfo& info,
      size_t firstChar);
  Run(const Run&) = default;
  Run& operator=(const Run&) = delete;
  Run(Run&&) = default;
  Run& operator=(Run&&) = delete;
  ~Run() = default;

  SkShaper::RunHandler::Buffer newRunBuffer();

  size_t size() const { return fGlyphs.size(); }
  SkScalar ascent() const { return fFontMetrics.GetAscent(); }
  SkScalar descent() const { return fFontMetrics.GetDescent(); }
  SkScalar leading() const { return 0; }
  SkScalar correctAscent() const { return fCorrectAscent; }
  SkScalar correctDescent() const { return fCorrectDescent; }
  SkScalar correctLeading() const { return fCorrectLeading; }
  const Font& font() const { return fFont; }
  bool leftToRight() const { return fBidiLevel % 2 == 0; }
  //  TextDirection getTextDirection() const {
  //    return leftToRight() ? TextDirection::kLtr : TextDirection::kRtl;
  //  }
  //  size_t index() const { return fIndex; }
  size_t clusterIndex(size_t pos) const { return fClusterIndexes[pos]; }

  TextRange textRange() const { return fTextRange; }

  void commit();

  //  SkRect getBounds(size_t pos) const { return fBounds[pos]; }

 public:
  std::vector<GlyphID>& GetGlyphs() { return fGlyphs; }
  std::vector<float>& GetAdvances() { return advances_; }
  std::vector<uint32_t> GetIndices() { return fClusterIndexes; }

 private:
  friend class OneLineShaper;

  const char32_t* fOwner;
  TextRange fTextRange;

  Font fFont;
  std::vector<GlyphID> fGlyphs;
  std::vector<float> advances_;
  //  std::vector<TTPoint> fPositions;
  std::vector<uint32_t> fClusterIndexes;
  FontInfo fFontMetrics;
  SkScalar fCorrectAscent;
  SkScalar fCorrectDescent;
  SkScalar fCorrectLeading;

  uint8_t fBidiLevel;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_SKSHAPER_RUN_H_
