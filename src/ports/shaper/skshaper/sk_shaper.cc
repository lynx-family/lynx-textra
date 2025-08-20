/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/shaper/skshaper/sk_shaper.h"

#include "src/ports/shaper/skshaper/harfbuzz_lib.h"

namespace ttoffice {
namespace tttext {
// std::unique_ptr<SkShaper> SkShaper::Make(sk_sp<SkFontMgr> fontmgr) {
// #ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
//   std::unique_ptr<SkShaper> shaper =
//       SkShaper::MakeShapeDontWrapOrReorder(std::move(fontmgr));
//   if (shaper) {
//     return shaper;
//   }
// #endif
//   return SkShaper::MakePrimitive();
// }

std::unique_ptr<SkShaper> SkShaper::MakeShapeDontWrapOrReorder() {
  return HarfbuzzLib::HasHarfbuzzLib()
             ? MakeShapeDontWrapOrReorderForHB()
             : MakeShapeDontWrapOrReorderForPlatform();
}

void SkShaper::PurgeCaches() {
#ifdef SK_SHAPER_HARFBUZZ_AVAILABLE
  if (HarfbuzzLib::HasHarfbuzzLib()) {
    PurgeHarfBuzzCache();
  }
#endif
}

// std::unique_ptr<SkShaper::BiDiRunIterator>
// SkShaper::MakeBiDiRunIterator(const char* utf8, size_t utf8Bytes, uint8_t
// bidiLevel) { #ifdef SK_UNICODE_AVAILABLE
//     auto unicode = SkUnicode::Make();
//     if (!unicode) {
//         return nullptr;
//     }
//     std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
//         SkShaper::MakeSkUnicodeBidiRunIterator(unicode.get(),
//                                                utf8,
//                                                utf8Bytes,
//                                                bidiLevel);
//     if (bidi) {
//         return bidi;
//     }
// #endif
//     return std::make_unique<SkShaper::TrivialBiDiRunIterator>(bidiLevel,
//     utf8Bytes);
// }
//
// std::unique_ptr<SkShaper::ScriptRunIterator>
// SkShaper::MakeScriptRunIterator(const char* utf8, size_t utf8Bytes,
// SkFourByteTag scriptTag) { #if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) &&
// defined(SK_UNICODE_AVAILABLE)
//     auto unicode = SkUnicode::Make();
//     if (!unicode) {
//         return nullptr;
//     }
//     std::unique_ptr<SkShaper::ScriptRunIterator> script =
//         SkShaper::MakeSkUnicodeHbScriptRunIterator(unicode.get(), utf8,
//         utf8Bytes);
//     if (script) {
//         return script;
//     }
// #endif
//     return std::make_unique<SkShaper::TrivialScriptRunIterator>(scriptTag,
//     utf8Bytes);
// }

SkShaper::SkShaper() {}
SkShaper::~SkShaper() {}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
// static inline uint32_t utf8_next(const char** ptr, const char* end) {
//   uint32_t char_len = 0;
//   uint32_t val = ttoffice::U8String::U8CharToU32(*ptr, char_len);
//   *ptr += char_len;
//   return val == -1 ? 0xFFFD : val;
// }

// class FontMgrRunIterator final : public SkShaper::FontRunIterator {
//  public:
//   FontMgrRunIterator(const char32_t* utf8, size_t utf8Bytes, const SkFont&
//   font,
//                      sk_sp<SkFontMgr> fallbackMgr, const char* requestName,
//                      SkFontStyle requestStyle,
//                      const SkShaper::LanguageRunIterator* lang)
//       : fCurrent(utf8),
//         fBegin(utf8),
//         fEnd(fCurrent + utf8Bytes),
//         fFallbackMgr(std::move(fallbackMgr)),
//         fFont(font),
//         fFallbackFont(fFont),
//         fCurrentFont(nullptr),
//         fRequestName(requestName),
//         fRequestStyle(requestStyle),
//         fLanguage(lang) {
//     fFont.setTypeface(font.refTypefaceOrDefault());
//     fFallbackFont.setTypeface(nullptr);
//   }
//   FontMgrRunIterator(const char32_t* utf8, size_t utf8Bytes, const SkFont&
//   font,
//                      sk_sp<SkFontMgr> fallbackMgr)
//       : FontMgrRunIterator(utf8, utf8Bytes, font, std::move(fallbackMgr),
//                            nullptr, font.refTypefaceOrDefault()->fontStyle(),
//                            nullptr) {}
//
//   void consume() override {
//     TTASSERT(fCurrent < fEnd);
//     TTASSERT(!fLanguage ||
//              this->endOfCurrentRun() <= fLanguage->endOfCurrentRun());
//     auto u = *fCurrent++;  // utf8_next(&fCurrent, fEnd);
//     // If the starting typeface can handle this character, use it.
//     if (fFont.unicharToGlyph(u)) {
//       fCurrentFont = &fFont;
//       // If the current fallback can handle this character, use it.
//     } else if (fFallbackFont.getTypeface() &&
//     fFallbackFont.unicharToGlyph(u)) {
//       fCurrentFont = &fFallbackFont;
//       // If not, try to find a fallback typeface
//     } else {
//       const char* language = fLanguage ? fLanguage->currentLanguage() :
//       nullptr; int languageCount = fLanguage ? 1 : 0; sk_sp<SkTypeface>
//       candidate(fFallbackMgr->matchFamilyStyleCharacter(
//           fRequestName, fRequestStyle, &language, languageCount, u));
//       if (candidate) {
//         fFallbackFont.setTypeface(std::move(candidate));
//         fCurrentFont = &fFallbackFont;
//       } else {
//         fCurrentFont = &fFont;
//       }
//     }
//
//     while (fCurrent < fEnd) {
//       const char32_t* prev = fCurrent;
//       u = *fCurrent++;  // utf8_next(&fCurrent, fEnd);
//
//       // End run if not using initial typeface and initial typeface has this
//       // character.
//       if (fCurrentFont->getTypeface() != fFont.getTypeface() &&
//           fFont.unicharToGlyph(u)) {
//         fCurrent = prev;
//         return;
//       }
//
//       // End run if current typeface does not have this character and some
//       other
//       // font does.
//       if (!fCurrentFont->unicharToGlyph(u)) {
//         const char* language =
//             fLanguage ? fLanguage->currentLanguage() : nullptr;
//         int languageCount = fLanguage ? 1 : 0;
//         sk_sp<SkTypeface> candidate(fFallbackMgr->matchFamilyStyleCharacter(
//             fRequestName, fRequestStyle, &language, languageCount, u));
//         if (candidate) {
//           fCurrent = prev;
//           return;
//         }
//       }
//     }
//   }
//   size_t endOfCurrentRun() const override { return fCurrent - fBegin; }
//   bool atEnd() const override { return fCurrent == fEnd; }
//
//   const ITypefaceHelper* currentFont() const override { return *fCurrentFont;
//   }
//
//  private:
//   char32_t const* fCurrent;
//   char32_t const* const fBegin;
//   char32_t const* const fEnd;
//   sk_sp<SkFontMgr> const fFallbackMgr;
//   SkFont fFont;
//   SkFont fFallbackFont;
//   SkFont* fCurrentFont;
//   char const* const fRequestName;
//   SkFontStyle const fRequestStyle;
//   SkShaper::LanguageRunIterator const* const fLanguage;
// };

// std::unique_ptr<SkShaper::FontRunIterator> SkShaper::MakeFontMgrRunIterator(
//     const char32_t* utf8, size_t utf8Bytes, const SkFont& font,
//     sk_sp<SkFontMgr> fallback) {
//   return std::make_unique<FontMgrRunIterator>(utf8, utf8Bytes, font,
//                                               std::move(fallback));
// }

// std::unique_ptr<SkShaper::FontRunIterator> SkShaper::MakeFontMgrRunIterator(
//     const char32_t* utf8, size_t utf8Bytes, const SkFont& font,
//     sk_sp<SkFontMgr> fallback, const char* requestName,
//     SkFontStyle requestStyle, const SkShaper::LanguageRunIterator* language)
//     {
//   return std::make_unique<FontMgrRunIterator>(utf8, utf8Bytes, font,
//                                               std::move(fallback),
//                                               requestName, requestStyle,
//                                               language);
// }

// std::unique_ptr<SkShaper::LanguageRunIterator>
// SkShaper::MakeStdLanguageRunIterator(const char32_t* utf8, size_t utf8Bytes)
// {
//   return std::make_unique<TrivialLanguageRunIterator>(
//       std::locale().name().c_str(), utf8Bytes);
// }
}  // namespace tttext
}  // namespace ttoffice
