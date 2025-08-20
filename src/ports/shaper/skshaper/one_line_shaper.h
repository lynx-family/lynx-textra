// Copyright 2019 Google LLC.
/**
Copyright (c) 2011 Google Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

* Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef SRC_PORTS_SHAPER_SKSHAPER_ONE_LINE_SHAPER_H_
#define SRC_PORTS_SHAPER_SKSHAPER_ONE_LINE_SHAPER_H_

#include <textra/fontmgr_collection.h>

#include <deque>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ports/shaper/skshaper/font.h"
#include "src/ports/shaper/skshaper/sk_shaper.h"
#include "src/textlayout/paragraph_impl.h"
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

class Run;
class ShaperSkShaper;
class OneLineShaper : public SkShaper::RunHandler {
  friend ShaperSkShaper;

 public:
  explicit OneLineShaper(FontmgrCollection& font_collections);

  bool shape(const char32_t* content, uint32_t len, const ShapeStyle& style,
             bool rtl);

  size_t unresolvedGlyphs() { return fUnresolvedGlyphs; }

  struct RunBlock {
    RunBlock() : fRun(nullptr) {}

    // First unresolved block
    explicit RunBlock(TextRange text) : fRun(nullptr), fText(text) {}

    RunBlock(std::shared_ptr<Run> run, TextRange text, GlyphRange glyphs)
        : fRun(std::move(run)), fText(text), fGlyphs(glyphs) {}

    // Entire run comes as one block fully resolved
    explicit RunBlock(std::shared_ptr<Run> run);

    std::shared_ptr<Run> fRun;
    TextRange fText;
    GlyphRange fGlyphs;
    bool isFullyResolved() const;
  };

  enum Resolved { Nothing, Something, Everything };

 private:
  using TypefaceVisitor =
      std::function<Resolved(std::shared_ptr<ITypefaceHelper> typeface)>;
  void matchResolvedFonts(const ShapeStyle& textStyle,
                          const TypefaceVisitor& visitor);
  void finish(TextRange text);

  void beginLine() override {}
  void runInfo(const RunInfo&) override {}
  void commitRunInfo() override {}
  void commitLine() override {}

  Buffer runBuffer(const RunInfo& info) override;

  void commitRunBuffer(const RunInfo&) override;

  ClusterIndex clusterIndex(GlyphIndex glyph);
  void addFullyResolved();
  TextRange normalizeTextRange(GlyphRange glyphRange);
  void addUnresolvedWithRun(GlyphRange glyphRange);
  void sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock);
  void fillGaps(size_t);

  const char32_t* content_;
  uint32_t len_;
  FontmgrCollection& fFontCollection_;
  TextRange fCurrentText;
  //  SkScalar fHeight;
  //  SkVector fAdvance;
  size_t fUnresolvedGlyphs;

  // TODO(hfuttyh): Something that is not thead-safe since we don't need it
  std::shared_ptr<Run> fCurrentRun;
  std::deque<RunBlock> fUnresolvedBlocks;
  std::vector<RunBlock> fResolvedBlocks;

  std::unique_ptr<SkShaper> shaper_;

  // Keeping all resolved typefaces
  struct FontKey {
    FontKey() {}

    FontKey(Unichar unicode, FontStyle fontStyle, std::string locale)
        : fUnicode(unicode), fFontStyle(fontStyle), fLocale(locale) {}
    Unichar fUnicode;
    FontStyle fFontStyle;
    std::string fLocale;

    bool operator==(const FontKey& other) const;

    size_t operator()(const FontKey& key) const;
  };
  std::unordered_map<FontKey, std::shared_ptr<ITypefaceHelper>, FontKey>
      fFallbackFonts;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // SRC_PORTS_SHAPER_SKSHAPER_ONE_LINE_SHAPER_H_
