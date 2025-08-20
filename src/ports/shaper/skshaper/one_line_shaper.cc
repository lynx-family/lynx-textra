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

#include "src/ports/shaper/skshaper/one_line_shaper.h"

#include <textra/font_info.h>

#include <algorithm>
#include <limits>
#include <unordered_set>

#include "src/ports/shaper/skshaper/run.h"
#include "src/textlayout/tt_shaper.h"
#include "src/textlayout/utils/hasher.h"
#include "src/textlayout/utils/log_util.h"

namespace ttoffice {
namespace tttext {
OneLineShaper::OneLineShaper(FontmgrCollection& font_collections)
    : fFontCollection_(font_collections), fUnresolvedGlyphs(0) {
  shaper_ = SkShaper::MakeShapeDontWrapOrReorder();
}
void OneLineShaper::commitRunBuffer(const RunInfo&) {
  fCurrentRun->commit();

  auto oldUnresolvedCount = fUnresolvedBlocks.size();
  /*
      SkDebugf("Run [%d:%d)\n", fCurrentRun->fTextRange.start,
     fCurrentRun->fTextRange.end); for (size_t i = 0; i < fCurrentRun->size();
     ++i) { SkDebugf("[%d] %d %d %f\n", i, fCurrentRun->fGlyphs[i],
     fCurrentRun->fClusterIndexes[i], fCurrentRun->fPositions[i].fX);
      }
  */
  // Find all unresolved blocks
  sortOutGlyphs([&](GlyphRange block) {
    if (block.Empty()) {
      return;
    }
    // LogUtil::E("Add UnresolvedWithRun");
    addUnresolvedWithRun(block);
  });

  // Fill all the gaps between unresolved blocks with resolved ones
  if (oldUnresolvedCount == fUnresolvedBlocks.size()) {
    // No unresolved blocks added - we resolved the block with one run entirely
    addFullyResolved();
    return;
  } else if (oldUnresolvedCount == fUnresolvedBlocks.size() - 1) {
    auto& unresolved = fUnresolvedBlocks.back();
    if (fCurrentRun->textRange() == unresolved.fText) {
      // Nothing was resolved; preserve the initial run if it makes sense
      auto& front = fUnresolvedBlocks.front();
      if (front.fRun != nullptr) {
        unresolved.fRun = front.fRun;
        unresolved.fGlyphs = front.fGlyphs;
      }
      return;
    }
  }

  fillGaps(oldUnresolvedCount);
}

#ifdef SK_DEBUG
// void OneLineShaper::printState() {
// #ifndef RK_ENABLE_TRIM
//   SkDebugf("Resolved: %d\n", fResolvedBlocks.size());
//   for (auto& resolved : fResolvedBlocks) {
//     if (resolved.fRun == nullptr) {
//       SkDebugf("[%d:%d) unresolved\n", resolved.fText.start,
//                resolved.fText.end);
//       continue;
//     }
//     SkString name("???");
//     if (resolved.fRun->fFont.getTypeface() != nullptr) {
//       resolved.fRun->fFont.getTypeface()->getFamilyName(&name);
//     }
//     SkDebugf("[%d:%d) ", resolved.fGlyphs.start, resolved.fGlyphs.end);
//     SkDebugf("[%d:%d) with %s\n", resolved.fText.start, resolved.fText.end,
//              name.c_str());
//   }
//
//   auto size = fUnresolvedBlocks.size();
//   SkDebugf("Unresolved: %d\n", size);
//   for (const auto& unresolved : fUnresolvedBlocks) {
//     SkDebugf("[%d:%d)\n", unresolved.fText.start, unresolved.fText.end);
//   }
// #endif
// }
#endif

#define EMPTY_TEXT \
  TextRange { tttext::Range::EmptyIndex(), tttext::Range::EmptyIndex() }

void OneLineShaper::fillGaps(size_t startingCount) {
  // Fill out gaps between all unresolved blocks
  TextRange resolvedTextLimits = fCurrentRun->fTextRange;
  if (!fCurrentRun->leftToRight()) {
    resolvedTextLimits.Swap();
    //    std::swap(resolvedTextLimits.GetStart(), resolvedTextLimits.GetEnd());
  }
  TextIndex resolvedTextStart = resolvedTextLimits.GetStart();
  GlyphIndex resolvedGlyphsStart = 0;

  auto begin = fUnresolvedBlocks.begin();
  auto end = fUnresolvedBlocks.end();
  begin += startingCount;  // Skip the old ones, do the new ones
  TextRange prevText = EMPTY_TEXT;
  for (; begin != end; ++begin) {
    auto& unresolved = *begin;

    if (unresolved.fText == prevText) {
      // Clean up repetitive blocks that appear inside the same grapheme block
      unresolved.fText = EMPTY_TEXT;
      continue;
    } else {
      prevText = unresolved.fText;
    }

    TextRange resolvedText(resolvedTextStart, fCurrentRun->leftToRight()
                                                  ? unresolved.fText.GetStart()
                                                  : unresolved.fText.GetEnd());
    if (resolvedText.Width() > 0) {
      if (!fCurrentRun->leftToRight()) {
        resolvedText.Swap();
      }

      GlyphRange resolvedGlyphs(resolvedGlyphsStart,
                                unresolved.fGlyphs.GetStart());
      RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs);

      if (resolvedGlyphs.Width() == 0) {
        // Extend the unresolved block with an empty resolved
        if (unresolved.fText.GetEnd() <= resolved.fText.GetStart()) {
          unresolved.fText.SetEnd(resolved.fText.GetEnd());
        }
        if (unresolved.fText.GetStart() >= resolved.fText.GetEnd()) {
          unresolved.fText.SetStart(resolved.fText.GetStart());
        }
      } else {
        fResolvedBlocks.emplace_back(resolved);
      }
    }
    resolvedGlyphsStart = unresolved.fGlyphs.GetEnd();
    resolvedTextStart = fCurrentRun->leftToRight()
                            ? unresolved.fText.GetEnd()
                            : unresolved.fText.GetStart();
  }

  TextRange resolvedText(resolvedTextStart, resolvedTextLimits.GetEnd());
  if (resolvedText.Width() > 0) {
    if (!fCurrentRun->leftToRight()) {
      resolvedText.Swap();
    }

    GlyphRange resolvedGlyphs(resolvedGlyphsStart,
                              static_cast<unsigned int>(fCurrentRun->size()));
    RunBlock resolved(fCurrentRun, resolvedText, resolvedGlyphs);
    fResolvedBlocks.emplace_back(resolved);
  }
}

void OneLineShaper::finish(TextRange blockText) {
  // Add all unresolved blocks to resolved blocks
  while (!fUnresolvedBlocks.empty()) {
    auto unresolved = fUnresolvedBlocks.front();
    fUnresolvedBlocks.pop_front();
    if (unresolved.fText.GetLength() == 0) {
      continue;
    }
    fResolvedBlocks.emplace_back(unresolved);
    fUnresolvedGlyphs += unresolved.fGlyphs.GetLength();
  }

  // Sort all pieces by text
  std::sort(fResolvedBlocks.begin(), fResolvedBlocks.end(),
            [](const RunBlock& a, const RunBlock& b) {
              return a.fText.GetStart() < b.fText.GetStart();
            });

  // Go through all of them
  size_t lastTextEnd = blockText.GetStart();
  for (auto& block : fResolvedBlocks) {
    if (block.fText.GetEnd() <= blockText.GetStart()) {
      continue;
    }

    auto run = block.fRun;
    auto glyphs = block.fGlyphs;
    auto text = block.fText;
    if (lastTextEnd != text.GetStart()) {
      LogUtil::D("Text ranges mismatch: ...:%d] - [%d:%d] (%d-%d)\n",
                 lastTextEnd, text.GetStart(), text.GetEnd(), glyphs.GetStart(),
                 glyphs.GetEnd());
      TTASSERT(false);
    }
    lastTextEnd = text.GetEnd();

    //    if (block.isFullyResolved()) {
    //      // Just move the entire run
    //      //            block.fRun->fIndex = this->fParagraph->fRuns.size();
    //      //            this->fParagraph->fRuns.emplace_back(*block.fRun);
    //      block.fRun.reset();
    //      continue;
    //    } else if (run == nullptr) {
    //      continue;
    //    }
    //
    //    auto runAdvance = SkVector::Make(
    //        run->posX(glyphs.end) - run->posX(glyphs.start),
    //        run->fAdvance.fY);
    //    const SkShaper::RunHandler::RunInfo info = {
    //        run->fFont, run->fBidiLevel, runAdvance, glyphs.width(),
    //        SkShaper::RunHandler::Range(text.start - run->fClusterStart,
    //                                    text.width())};
    //    //        this->fParagraph->fRuns.emplace_back(
    //    //                    this->fParagraph,
    //    //                    info,
    //    //                    run->fClusterStart,
    //    //                    GetEvenLeadingEnabled(this->fParagraph),
    //    //                    height,
    //    //                    this->fParagraph->fRuns.count(),
    //    //                    advanceX
    //    //                );
    //    //        auto piece = &this->fParagraph->fRuns.back();
    //
    //    //        // TODO: Optimize copying
    //    //        auto zero = run->fPositions[glyphs.start];
    //    //        for (size_t i = glyphs.start; i <= glyphs.end; ++i) {
    //    //
    //    //            auto index = i - glyphs.start;
    //    //            if (i < glyphs.end) {
    //    //                piece->fGlyphs[index] = run->fGlyphs[i];
    //    //                piece->fBounds[index] = run->fBounds[i];
    //    //            }
    //    //            piece->fClusterIndexes[index] = run->fClusterIndexes[i];
    //    //            piece->fPositions[index] = run->fPositions[i] - zero;
    //    //            piece->addX(index, advanceX);
    //    //        }
    //
    //    // Carve out the line text out of the entire run text
    //    fAdvance.fX += runAdvance.fX;
    //    fAdvance.fY = std::max(fAdvance.fY, runAdvance.fY);
  }
  if (lastTextEnd != blockText.GetEnd()) {
    LogUtil::D("Last range mismatch: %d - %d\n", lastTextEnd,
               blockText.GetEnd());
    TTASSERT(false);
  }
}

void OneLineShaper::addFullyResolved() {
  if (this->fCurrentRun->size() == 0) {
    return;
  }
  RunBlock resolved(
      fCurrentRun, this->fCurrentRun->fTextRange,
      GlyphRange(0, static_cast<unsigned int>(this->fCurrentRun->size())));
  fResolvedBlocks.emplace_back(resolved);
}

// Make it [left:right) regardless of a text direction
TextRange OneLineShaper::normalizeTextRange(GlyphRange glyphRange) {
  if (fCurrentRun->leftToRight()) {
    return TextRange(clusterIndex(glyphRange.GetStart()),
                     clusterIndex(glyphRange.GetEnd()));
  } else {
    return TextRange(clusterIndex(glyphRange.GetEnd() - 1),
                     glyphRange.GetStart() > 0
                         ? clusterIndex(glyphRange.GetStart() - 1)
                         : fCurrentRun->fTextRange.GetEnd());
  }
}

void OneLineShaper::addUnresolvedWithRun(GlyphRange glyphRange) {
  auto text_range = normalizeTextRange(glyphRange);
  RunBlock unresolved(fCurrentRun, text_range, glyphRange);
  if (unresolved.fGlyphs.GetLength() == fCurrentRun->size()) {
    TTASSERT(unresolved.fText.GetLength() ==
             fCurrentRun->fTextRange.GetLength());
  } else if (!fUnresolvedBlocks.empty()) {
    auto& lastUnresolved = fUnresolvedBlocks.back();
    if (lastUnresolved.fRun != nullptr &&
        lastUnresolved.fRun == unresolved.fRun) {
      if (lastUnresolved.fText.GetEnd() == unresolved.fText.GetStart()) {
        // Two pieces next to each other - can join them
        lastUnresolved.fText.SetEnd(unresolved.fText.GetEnd());
        lastUnresolved.fGlyphs.SetEnd(glyphRange.GetEnd());
        return;
      } else if (lastUnresolved.fText == unresolved.fText) {
        // Nothing was resolved; ignore it
        return;
      } else if (lastUnresolved.fText.Contain(unresolved.fText)) {
        // We get here for the very first unresolved piece
        return;
      } else if (!tttext::Range::Intersect(lastUnresolved.fText,
                                           unresolved.fText)
                      .Empty()) {
        // Few pieces of the same unresolved text block can ignore the second
        // one
        lastUnresolved.fGlyphs.SetStart(
            std::min(lastUnresolved.fGlyphs.GetStart(), glyphRange.GetStart()));
        lastUnresolved.fGlyphs.SetEnd(
            std::max(lastUnresolved.fGlyphs.GetEnd(), glyphRange.GetEnd()));
        lastUnresolved.fText = lastUnresolved.fGlyphs;
        return;
      }
    }
  }
  fUnresolvedBlocks.emplace_back(unresolved);
}

// Glue whitespaces to the next/prev unresolved blocks
// (so we don't have chinese text with english whitespaces broken into millions
// of tiny runs)
#ifndef SK_PARAGRAPH_GRAPHEME_EDGES
void OneLineShaper::sortOutGlyphs(
    std::function<void(GlyphRange)>&& sortOutUnresolvedBLock) {
  auto text = fCurrentRun->fOwner;

  GlyphRange block(tttext::Range::EmptyIndex(), tttext::Range::EmptyIndex());
  bool graphemeResolved = false;
  TextIndex EMPTY_INDEX = std::numeric_limits<TextIndex>::max();
  TextIndex graphemeStart = EMPTY_INDEX;
  for (size_t i = 0; i < fCurrentRun->size(); ++i) {
    ClusterIndex ci = clusterIndex(static_cast<unsigned int>(i));
    // Removing all pretty optimizations for whitespaces
    // because they get in a way of grapheme rounding
    // Inspect the glyph
    auto glyph = fCurrentRun->fGlyphs[i];

    GraphemeIndex gi = ci;
    if ((fCurrentRun->leftToRight() ? gi > graphemeStart
                                    : gi < graphemeStart) ||
        graphemeStart == EMPTY_INDEX) {
      // This is the Flutter change
      // Do not count control codepoints as unresolved
      Unichar codepoint = text[ci];
      bool isControl8 = (codepoint == '\r') || (codepoint == '\n');
      // We only count glyph resolved if all the glyphs in its grapheme are
      // resolved
      graphemeResolved = glyph != 0 || isControl8;
      graphemeStart = gi;
    } else if (glyph == 0) {
      // Found unresolved glyph - the entire grapheme is unresolved now
      graphemeResolved = false;
    }

    if (!graphemeResolved) {  // Unresolved glyph and not control codepoint
      if (block.GetStart() == EMPTY_INDEX) {
        // Start new unresolved block
        block.SetStart(static_cast<unsigned int>(i));
        block.SetEnd(EMPTY_INDEX);
      } else {
        // Keep skipping unresolved block
      }
    } else {  // Resolved glyph or control codepoint
      if (block.GetStart() == EMPTY_INDEX) {
        // Keep skipping resolved code points
      } else {
        // This is the end of unresolved block
        block.SetEnd(static_cast<unsigned int>(i));
        sortOutUnresolvedBLock(block);
        block = tttext::Range(EMPTY_INDEX, EMPTY_INDEX);
      }
    }
  }

  // One last block could have been left
  if (block.GetStart() != EMPTY_INDEX) {
    block.SetEnd(static_cast<unsigned int>(fCurrentRun->size()));
    sortOutUnresolvedBLock(block);
  }
}
#else
void OneLineShaper::sortOutGlyphs(
    std::function<void(GlyphRange)>&& sortOutUnresolvedBLock) {
  auto text = fCurrentRun->fOwner->text();

  TextIndex whitespacesStart = EMPTY_INDEX;
  GlyphRange block = EMPTY_RANGE;
  for (size_t i = 0; i < fCurrentRun->size(); ++i) {
    const char* cluster = text.begin() + clusterIndex(i);
    SkUnichar codepoint = nextUtf8Unit(&cluster, text.end());
    bool isControl8 = fParagraph->getUnicode()->isControl(codepoint);
    // TODO(hfuttyh): This is a temp change to match space handiling in LibTxt
    // (all spaces are resolved with the main font)
#ifdef SK_PARAGRAPH_LIBTXT_SPACES_RESOLUTION
    bool isWhitespace8 =
        false;  // fParagraph->getUnicode()->isWhitespace(codepoint);
#else
    bool isWhitespace8 = fParagraph->getUnicode()->isWhitespace(codepoint);
#endif
    // Inspect the glyph
    auto glyph = fCurrentRun->fGlyphs[i];
    if (glyph == 0 &&
        !isControl8) {  // Unresolved glyph and not control codepoint
      if (block.start == EMPTY_INDEX) {
        // Start new unresolved block
        // (all leading whitespaces glued to the resolved part if it's not
        // empty)
        block.start = whitespacesStart == 0 ? 0 : i;
        block.end = EMPTY_INDEX;
      } else {
        // Keep skipping unresolved block
      }
    } else {  // Resolved glyph or control codepoint
      if (block.start == EMPTY_INDEX) {
        // Keep skipping resolved code points
      } else if (isWhitespace8) {
        // Glue whitespaces after to the unresolved block
      } else {
        // This is the end of unresolved block (all trailing whitespaces glued
        // to the resolved part)
        block.end = whitespacesStart == EMPTY_INDEX ? i : whitespacesStart;
        sortOutUnresolvedBLock(block);
        block = EMPTY_RANGE;
        whitespacesStart = EMPTY_INDEX;
      }
    }

    // Keep updated the start of the latest whitespaces patch
    if (isWhitespace8) {
      if (whitespacesStart == EMPTY_INDEX) {
        whitespacesStart = i;
      }
    } else {
      whitespacesStart = EMPTY_INDEX;
    }
  }

  // One last block could have been left
  if (block.start != EMPTY_INDEX) {
    block.end = fCurrentRun->size();
    sortOutUnresolvedBLock(block);
  }
}
#endif

void OneLineShaper::matchResolvedFonts(const ShapeStyle& textStyle,
                                       const TypefaceVisitor& visitor) {
  std::vector<std::shared_ptr<ITypefaceHelper>> typefaces;
  auto fd = textStyle.GetFontDescriptor();
  auto font_style = fd.font_style_;

  if (fd.platform_font_ != 0) {
    auto font_mgr = fFontCollection_.GetDefaultFontManager();
    if (font_mgr != nullptr) {
      font_mgr->createTypefaceFromPlatformFont(
          reinterpret_cast<const void*>(fd.platform_font_));
    } else {
      typefaces.emplace_back(
          reinterpret_cast<ITypefaceHelper*>(fd.platform_font_)
              ->shared_from_this());
    }
  } else {
    auto found_typefaces = fFontCollection_.findTypefaces(fd);
    typefaces.insert(typefaces.end(), found_typefaces.begin(),
                     found_typefaces.end());
  }

  for (const auto& typeface : typefaces) {
    if (visitor(typeface) == Resolved::Everything) {
      // Resolved everything
      return;
    }
  }

  if (fFontCollection_.fontFallbackEnabled()) {
    // Give fallback a clue
    // Some unresolved subblocks might be resolved with different fallback fonts
    std::vector<RunBlock> hopelessBlocks;
    while (!fUnresolvedBlocks.empty()) {
      auto unresolvedRange = fUnresolvedBlocks.front().fText;
      // We have the global cache for all already found typefaces for SkUnichar
      // but we still need to keep track of all SkUnichars used in this
      // unresolved block
      auto idx = unresolvedRange.GetStart();
      std::unordered_set<Unichar> alreadyTried;
      Unichar unicode = content_[idx++];
      while (true) {
        std::shared_ptr<ITypefaceHelper> typeface;

        // First try to find in in a cache
        FontKey fontKey(unicode, font_style, "");
        auto found = fFallbackFonts.find(fontKey);
        if (found != fFallbackFonts.end()) {
          typeface = found->second;
        } else {
          typeface = fFontCollection_.defaultFallback(unicode, font_style, "");

          if (typeface != nullptr) {
            fFallbackFonts[fontKey] = typeface;
          }
        }

        if (typeface != nullptr) {
          auto resolved = visitor(typeface);
          if (resolved == Resolved::Everything) {
            // Resolved everything, no need to try another font
            goto exit;
          }

          if (resolved == Resolved::Something) {
            // Resolved something, no need to try another codepoint
            break;
          }
        }

        if (idx == unresolvedRange.GetEnd()) {
          // Not a single codepoint could be resolved but we finished the block
          hopelessBlocks.push_back(fUnresolvedBlocks.front());
          fUnresolvedBlocks.pop_front();
          break;
        }

        // We can stop here or we can switch to another DIFFERENT codepoint
        while (idx != unresolvedRange.GetEnd()) {
          unicode = content_[idx++];
          auto found = alreadyTried.find(unicode);
          if (found == alreadyTried.end()) {
            alreadyTried.emplace(unicode);
            break;
          }
        }
      }
    }

  exit:
    for (auto& block : hopelessBlocks) {
      fUnresolvedBlocks.emplace_front(block);
    }
  }
}

bool OneLineShaper::shape(const char32_t* content, uint32_t len,
                          const ShapeStyle& style, bool rtl) {
  content_ = content;
  len_ = len;
  fUnresolvedBlocks.clear();
  fResolvedBlocks.clear();
  fCurrentRun = nullptr;
  // The text can be broken into many shaping sequences
  // (by placeholders, possibly, by hard line breaks or tabs, too)
  auto limitlessWidth = std::numeric_limits<SkScalar>::max();
  // float advanceX = 0;

  // Start from the beginning (hoping that it's a simple case one block - one
  // run)
  TextRange fRange(0, len_);
  fCurrentText = TextRange{0, len};
  fUnresolvedBlocks.emplace_back(RunBlock(fRange));

  matchResolvedFonts(style, [&](std::shared_ptr<ITypefaceHelper> typeface) {
    // Create one more font to try
    Font font(typeface, style.GetFontSize());
    //    font.setEdging(SkFont::Edging::kAntiAlias);
    //    font.setHinting(SkFontHinting::kSlight);
    font.SetSubpixel(true);

    // Apply fake bold and/or italic settings to the font if the
    // typeface's attributes do not match the intended font style.
    auto wanted_weight = style.GetFontDescriptor().font_style_.GetWeight();
    const bool fakeBold =
        wanted_weight >= FontStyle::Weight::kSemiBold_Weight &&
        static_cast<int32_t>(wanted_weight) -
                static_cast<int32_t>(typeface->FontStyle().GetWeight()) >=
            200;
    auto need_italic = style.GetFontDescriptor().font_style_.GetSlant() ==
                       FontStyle::Slant::kItalic_Slant;
    const bool fakeItalic = need_italic && typeface->FontStyle().GetSlant() !=
                                               FontStyle::Slant::kItalic_Slant;
    font.SetFakeBold(fakeBold);
    font.SetSkewX(fakeItalic ? -0.25 : 0);

    // Walk through all the currently unresolved blocks
    // (ignoring those that appear later)
    auto resolvedCount = fResolvedBlocks.size();
    auto unresolvedCount = fUnresolvedBlocks.size();
    while (unresolvedCount-- > 0) {
      auto unresolvedRange = fUnresolvedBlocks.front().fText;
      if (unresolvedRange == EMPTY_TEXT) {
        // Duplicate blocks should be ignored
        fUnresolvedBlocks.pop_front();
        continue;
      }

      SkShaper::TrivialFontRunIterator fontIter(font,
                                                unresolvedRange.GetLength());
      fCurrentText = unresolvedRange;
      shaper_->shape(content_ + unresolvedRange.GetStart(),
                     unresolvedRange.GetLength(), &fontIter, !rtl,
                     limitlessWidth, this);

      // Take off the queue the block we tried to resolved -
      // whatever happened, we have now smaller pieces of it to deal with
      fUnresolvedBlocks.pop_front();
    }

    if (fUnresolvedBlocks.empty()) {
      return Resolved::Everything;
    } else if (resolvedCount < fResolvedBlocks.size()) {
      return Resolved::Something;
    } else {
      return Resolved::Nothing;
    }
  });

  this->finish(fRange);
  return true;
}
SkShaper::RunHandler::Buffer OneLineShaper::runBuffer(
    const SkShaper::RunHandler::RunInfo& info) {
  fCurrentRun = std::make_shared<Run>(content_, info, fCurrentText.GetStart());
  return fCurrentRun->newRunBuffer();
}
ClusterIndex OneLineShaper::clusterIndex(GlyphIndex glyph) {
  return fCurrentText.GetStart() + fCurrentRun->fClusterIndexes[glyph];
}

bool OneLineShaper::FontKey::operator==(
    const OneLineShaper::FontKey& other) const {
  return fUnicode == other.fUnicode && fFontStyle == other.fFontStyle &&
         fLocale == other.fLocale;
}

size_t OneLineShaper::FontKey::operator()(
    const OneLineShaper::FontKey& key) const {
  return Hasher()
      .update(key.fUnicode)
      .update(static_cast<uint32_t>(key.fFontStyle.Value()))
      .updateString(key.fLocale)
      .hash();
}

OneLineShaper::RunBlock::RunBlock(std::shared_ptr<Run> run)
    : fRun(std::move(run)),
      fText(fRun->fTextRange),
      fGlyphs(GlyphRange(0, static_cast<unsigned int>(fRun->size()))) {}
bool OneLineShaper::RunBlock::isFullyResolved() const {
  return fRun != nullptr && fGlyphs.GetLength() == fRun->size();
}
}  // namespace tttext
}  // namespace ttoffice
