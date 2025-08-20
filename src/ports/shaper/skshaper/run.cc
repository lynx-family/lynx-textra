// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/ports/shaper/skshaper/run.h"

#include "src/ports/shaper/skshaper/sk_shaper.h"

namespace ttoffice {
namespace tttext {

Run::Run(const char32_t* owner, const SkShaper::RunHandler::RunInfo& info,
         size_t firstChar)
    : fOwner(owner),
      fTextRange(static_cast<uint32_t>(firstChar + info.utf8Range.begin()),
                 static_cast<uint32_t>(firstChar + info.utf8Range.end())),
      fFont(info.fFont) {
  fBidiLevel = info.fBidiLevel;
  //  fAdvance = info.fAdvance;
  //  fOffset = SkVector::Make(offsetX, 0);
  fGlyphs.resize(info.glyphCount);
  //  fBounds.resize(info.glyphCount);
  advances_.resize(info.glyphCount);
  //  fPositions.resize(info.glyphCount + 1);
  fClusterIndexes.resize(info.glyphCount + 1);
  //  fShifts.push_back_n(info.glyphCount + 1, 0.0);
  fFont.GetFontInfo(&fFontMetrics);

  // To make edge cases easier:
  //  fPositions[info.glyphCount] = fOffset + fAdvance;
  fClusterIndexes[info.glyphCount] = static_cast<uint32_t>(
      this->leftToRight() ? info.utf8Range.end() : info.utf8Range.begin());
}

SkShaper::RunHandler::Buffer Run::newRunBuffer() {
  return {fGlyphs.data(), advances_.data(), fClusterIndexes.data()};
}

void Run::commit() {
  //  fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
}
}  // namespace tttext
}  // namespace ttoffice
