// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <locale>
#include <vector>

#include "src/ports/shaper/skshaper/font.h"
#include "src/ports/shaper/skshaper/sk_shaper.h"

namespace ttoffice {
namespace tttext {
namespace {

using SkScalar = float;

using SkVector = PointF;

struct ShapedGlyph {
  GlyphID fID;
  uint32_t fCluster;
  PointF fOffset;
  SkVector fAdvance;
};

struct ShapedRun {
  ShapedRun(SkShaper::RunHandler::Range utf8Range, const Font& font, bool level,
            std::unique_ptr<ShapedGlyph[]> glyphs, size_t numGlyphs,
            SkVector advance = {0, 0})
      : fUtf8Range(utf8Range),
        fFont(font),
        fLevel(level),
        fGlyphs(std::move(glyphs)),
        fNumGlyphs(numGlyphs),
        fAdvance(advance) {}

  SkShaper::RunHandler::Range fUtf8Range;
  Font fFont;
  bool fLevel;
  std::unique_ptr<ShapedGlyph[]> fGlyphs;
  size_t fNumGlyphs;
  SkVector fAdvance;
};

constexpr bool is_LTR(bool level) { return (level & 1) == 0; }

void append(SkShaper::RunHandler* handler,
            const SkShaper::RunHandler::RunInfo& runInfo, const ShapedRun& run,
            size_t startGlyphIndex, size_t endGlyphIndex) {
  TTASSERT(startGlyphIndex <= endGlyphIndex);
  const size_t glyphLen = endGlyphIndex - startGlyphIndex;

  const auto buffer = handler->runBuffer(runInfo);
  TTASSERT(buffer.glyphs);
  TTASSERT(buffer.advances);

  SkVector advance = {0, 0};
  for (size_t i = 0; i < glyphLen; i++) {
    // Glyphs are in logical order, but output ltr since PDF readers seem to
    // expect that.
    const ShapedGlyph& glyph =
        run.fGlyphs[is_LTR(run.fLevel) ? startGlyphIndex + i
                                       : endGlyphIndex - 1 - i];
    buffer.glyphs[i] = glyph.fID;
    buffer.advances[i] = glyph.fAdvance.GetX();
    //    if (buffer.offsets) {
    //      buffer.positions[i] = advance + buffer.point;
    //      buffer.offsets[i] = glyph.fOffset;
    //    } else {
    //      buffer.positions[i] = advance + buffer.point + glyph.fOffset;
    //    }
    if (buffer.clusters) {
      buffer.clusters[i] = glyph.fCluster;
    }
    advance += glyph.fAdvance;
  }
  handler->commitRunBuffer(runInfo);
}

class ShaperPlatform : public SkShaper {
 public:
  ShaperPlatform();

 protected:
  ShapedRun shape(const char32_t* const utf8, const size_t utf8Bytes,
                  const char32_t* const utf8Start,
                  const char32_t* const utf8End, bool bidi,
                  const char* language, const SkFourByteTag& script,
                  const FontRunIterator& font, const Feature* const features,
                  const size_t featuresSize) const;

 private:
  void shape(const char32_t* utf8, size_t utf8Bytes, FontRunIterator*,
             bool leftToRight, SkScalar width, RunHandler*) const override;

  virtual void wrap(const char32_t* const utf8, size_t utf8Bytes, bool,
                    const char*, const SkFourByteTag, FontRunIterator*,
                    const Feature*, size_t featuresSize, SkScalar width,
                    RunHandler*) const = 0;
};

class ShapeDontWrapOrReorder : public ShaperPlatform {
 public:
  using ShaperPlatform::ShaperPlatform;

 private:
  void wrap(const char32_t* const utf8, size_t utf8Bytes, bool, const char*,
            const SkFourByteTag, FontRunIterator*, const Feature*,
            size_t featuresSize, float width, RunHandler*) const override;
};

ShaperPlatform::ShaperPlatform() {}

void ShaperPlatform::shape(const char32_t* utf8, size_t utf8Bytes,
                           FontRunIterator* font, bool leftToRight,
                           SkScalar width, RunHandler* handler) const {
  auto bidi = !leftToRight;
  auto language = std::locale().name();
  auto script = 0;
  this->wrap(utf8, utf8Bytes, bidi, language.c_str(), script, font, nullptr, 0,
             width, handler);
}

void ShapeDontWrapOrReorder::wrap(const char32_t* const utf8, size_t utf8Bytes,
                                  bool bidi, const char* language,
                                  const SkFourByteTag script,
                                  FontRunIterator* font,
                                  const Feature* features, size_t featuresSize,
                                  SkScalar width, RunHandler* handler) const {
  //  sk_ignore_unused_variable(width);
  std::vector<ShapedRun> runs;

  const char32_t* utf8Start = nullptr;
  const char32_t* utf8End = utf8;
  while (!font->atEnd()) {
    font->consume();

    utf8Start = utf8End;
    utf8End = utf8 + font->endOfCurrentRun();

    runs.emplace_back(shape(utf8, utf8Bytes, utf8Start, utf8End, bidi, language,
                            script, *font, features, featuresSize));
  }

  handler->beginLine();
  for (const auto& run : runs) {
    const RunHandler::RunInfo info = {run.fFont, run.fLevel, run.fAdvance,
                                      run.fNumGlyphs, run.fUtf8Range};
    handler->runInfo(info);
  }
  handler->commitRunInfo();
  for (const auto& run : runs) {
    const RunHandler::RunInfo info = {run.fFont, run.fLevel, run.fAdvance,
                                      run.fNumGlyphs, run.fUtf8Range};
    append(handler, info, run, 0, run.fNumGlyphs);
  }
  handler->commitLine();
}

ShapedRun ShaperPlatform::shape(
    const char32_t* const utf8, const size_t utf8Bytes,
    const char32_t* const utf8Start, const char32_t* const utf8End, bool bidi,
    const char* language, const SkFourByteTag& script,
    const FontRunIterator& fontRunIterator, const Feature* const features,
    const size_t featuresSize) const {
  const Font& font = fontRunIterator.currentFont();

  size_t numGlyphs = utf8End - utf8Start;
  std::vector<Unichar> unichars(numGlyphs);
  std::unique_ptr<GlyphID[]> glyphIDs(new GlyphID[numGlyphs]);
  std::unique_ptr<SkScalar[]> advances(new SkScalar[numGlyphs]);

  std::copy(utf8Start, utf8End, unichars.begin());

  font.UnicharsToGlyphs(unichars.data(), numGlyphs, glyphIDs.get());

  font.GetHorizontalAdvances(glyphIDs.get(), numGlyphs, advances.get());

  ShapedRun run(RunHandler::Range(utf8Start - utf8, numGlyphs), font, bidi,
                std::unique_ptr<ShapedGlyph[]>(new ShapedGlyph[numGlyphs]),
                numGlyphs);

  SkScalar totalAdvanceX = 0;
  for (size_t i = 0; i < numGlyphs; ++i) {
    ShapedGlyph& shapedGlyph = run.fGlyphs[i];
    shapedGlyph.fID = glyphIDs[i];
    shapedGlyph.fAdvance.SetX(advances[i]);
    // offset default to 0, 0
    shapedGlyph.fCluster = utf8Start - utf8 + i;

    totalAdvanceX += advances[i];
  }

  run.fAdvance.SetX(totalAdvanceX);
  return run;
}
}  // namespace
std::unique_ptr<SkShaper> SkShaper::MakeShapeDontWrapOrReorderForPlatform() {
  return std::make_unique<ShapeDontWrapOrReorder>();
}
}  // namespace tttext
}  // namespace ttoffice
