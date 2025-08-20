/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <hb-ot.h>
#include <hb.h>

#include <locale>
#include <mutex>
#include <vector>

#include "src/ports/shaper/skshaper/font.h"
#include "src/ports/shaper/skshaper/harfbuzz_lib.h"
#include "src/ports/shaper/skshaper/sk_shaper.h"
#include "src/textlayout/utils/lru_cache.h"
#include "src/textlayout/utils/mutex.h"
#include "src/textlayout/utils/tt_rectf.h"

// HB_FEATURE_GLOBAL_START and HB_FEATURE_GLOBAL_END were not added until
// HarfBuzz 2.0 They would have always worked, they just hadn't been named yet.
#if !defined(HB_FEATURE_GLOBAL_START)
#define HB_FEATURE_GLOBAL_START 0
#endif
#if !defined(HB_FEATURE_GLOBAL_END)
#define HB_FEATURE_GLOBAL_END ((unsigned int)-1)
#endif

namespace ttoffice {
namespace tttext {
namespace {

#define HB_FUNC(func_name)                                  \
  template <typename... Args>                               \
  auto tt_##func_name(Args&&... args)                       \
      -> decltype(func_name(std::forward<Args>(args)...)) { \
    return HarfbuzzLib::GetHarfbuzzLib()->f_##func_name(    \
        std::forward<Args>(args)...);                       \
  }

void tt_hb_blob_destroy(hb_blob_t* blob) {
  HarfbuzzLib::GetHarfbuzzLib()->f_hb_blob_destroy(blob);
}

void tt_hb_face_destroy(hb_face_t* face) {
  HarfbuzzLib::GetHarfbuzzLib()->f_hb_face_destroy(face);
}

void tt_hb_font_destroy(hb_font_t* font) {
  HarfbuzzLib::GetHarfbuzzLib()->f_hb_font_destroy(font);
}

void tt_hb_buffer_destroy(hb_buffer_t* buffer) {
  HarfbuzzLib::GetHarfbuzzLib()->f_hb_buffer_destroy(buffer);
}

void tt_hb_buffer_clear_contents(hb_buffer_t* buffer) {
  HarfbuzzLib::GetHarfbuzzLib()->f_hb_buffer_clear_contents(buffer);
}

HB_EMIT_FUNCS_TYPE1
#undef HB_FUNC

template <typename T, T* P>
struct SkFunctionWrapper {
  template <typename... Args>
  auto operator()(Args&&... args) const
      -> decltype(P(std::forward<Args>(args)...)) {
    return P(std::forward<Args>(args)...);
  }
};

/** \class SkAutoTCallVProc

Call a function when this goes out of scope. The template uses two
    parameters, the object, and a function that is to be called in the
destructor. If release() is called, the object reference is set to null. If the
object reference is null when the destructor is called, we do not call the
        function.
        */
template <typename T, void (*P)(T*)>
class SkAutoTCallVProc
    : public std::unique_ptr<
          T, SkFunctionWrapper<std::remove_pointer_t<decltype(P)>, P>> {
  using inherited =
      std::unique_ptr<T,
                      SkFunctionWrapper<std::remove_pointer_t<decltype(P)>, P>>;

 public:
  using inherited::inherited;
  SkAutoTCallVProc(const SkAutoTCallVProc&) = delete;
  SkAutoTCallVProc(SkAutoTCallVProc&& that) : inherited(std::move(that)) {}

  operator T*() const { return this->get(); }
};

template <typename T, typename P, P* p>
using resource = std::unique_ptr<T, SkFunctionWrapper<P, p>>;
using HBBlob =
    resource<hb_blob_t, decltype(tt_hb_blob_destroy), tt_hb_blob_destroy>;
using HBFace =
    resource<hb_face_t, decltype(tt_hb_face_destroy), tt_hb_face_destroy>;
using HBFont =
    resource<hb_font_t, decltype(tt_hb_font_destroy), tt_hb_font_destroy>;
using HBBuffer =
    resource<hb_buffer_t, decltype(tt_hb_buffer_destroy), tt_hb_buffer_destroy>;
using SkScalar = float;

// using SkUnicodeBidi = std::unique_ptr<SkBidiIterator>;
// using SkUnicodeBreak = std::unique_ptr<SkBreakIterator>;
// using SkUnicodeScript = std::unique_ptr<SkScriptIterator>;

#define SK_MaxS32FitsInFloat 2147483520
#define SK_MinS32FitsInFloat -SK_MaxS32FitsInFloat

/**
 *  Return the closest int for the given float. Returns SK_MaxS32FitsInFloat for
 * NaN.
 */
inline int sk_float_saturate2int(float x) {
  x = x < SK_MaxS32FitsInFloat ? x : SK_MaxS32FitsInFloat;
  x = x > SK_MinS32FitsInFloat ? x : SK_MinS32FitsInFloat;
  return static_cast<int>(x);
}
#define sk_float_floor(x) floorf(x)
#define sk_float_ceil(x) ceilf(x)
#define sk_float_floor2int(x) sk_float_saturate2int(sk_float_floor(x))
#define sk_float_round2int(x) sk_float_saturate2int(sk_float_floor((x) + 0.5f))
#define sk_float_ceil2int(x) sk_float_saturate2int(sk_float_ceil(x))
#define SkScalarRoundToInt(x) sk_float_round2int(x)
#define SkScalarFloorToInt(x) sk_float_floor2int(x)
#define SkScalarCeilToInt(x) sk_float_ceil2int(x)

#define SkIntToScalar(x) static_cast<float>(x)

hb_position_t skhb_position(SkScalar value) {
  // Treat HarfBuzz hb_position_t as 16.16 fixed-point.
  constexpr int kHbPosition1 = 1 << 16;
  return SkScalarRoundToInt(value * kHbPosition1);
}

hb_bool_t skhb_glyph(hb_font_t* hb_font, void* font_data,
                     hb_codepoint_t unicode, hb_codepoint_t variation_selector,
                     hb_codepoint_t* glyph, void* user_data) {
  Font& font = *reinterpret_cast<Font*>(font_data);
  *glyph = font.UnicharToGlyph(unicode);
  return *glyph != 0;
}

hb_bool_t skhb_nominal_glyph(hb_font_t* hb_font, void* font_data,
                             hb_codepoint_t unicode, hb_codepoint_t* glyph,
                             void* user_data) {
  return skhb_glyph(hb_font, font_data, unicode, 0, glyph, user_data);
}

unsigned skhb_nominal_glyphs(hb_font_t* hb_font, void* font_data,
                             unsigned int count, const hb_codepoint_t* unicodes,
                             unsigned int unicode_stride,
                             hb_codepoint_t* glyphs, unsigned int glyph_stride,
                             void* user_data) {
  Font& font = *reinterpret_cast<Font*>(font_data);

  // Batch call textToGlyphs since entry cost is not cheap.
  // Copy requred because textToGlyphs is dense and hb is strided.
  std::vector<Unichar> unicode(count);
  for (unsigned i = 0; i < count; i++) {
    unicode[i] = *unicodes;
    unicodes = reinterpret_cast<const hb_codepoint_t*>(
        reinterpret_cast<const char*>(unicodes) + unicode_stride);
  }
  std::vector<GlyphID> glyph(count);
  font.UnicharsToGlyphs(unicode.data(), count, glyph.data());

  // Copy the results back to the sparse array.
  unsigned int done;
  for (done = 0; done < count && glyph[done] != 0; done++) {
    *glyphs = glyph[done];
    glyphs = reinterpret_cast<hb_codepoint_t*>(reinterpret_cast<char*>(glyphs) +
                                               glyph_stride);
  }
  // return 'done' to allow HarfBuzz to synthesize with NFC and spaces, return
  // 'count' to avoid
  return done;
}

hb_position_t skhb_glyph_h_advance(hb_font_t* hb_font, void* font_data,
                                   hb_codepoint_t hbGlyph, void* user_data) {
  Font& font = *reinterpret_cast<Font*>(font_data);

  SkScalar advance;
  GlyphID skGlyph = static_cast<GlyphID>(hbGlyph);

  advance = font.GetHorizontalAdvance(skGlyph);
  if (!font.IsSubpixel()) {
    advance = SkScalarRoundToInt(advance);
  }
  return skhb_position(advance);
}

void skhb_glyph_h_advances(hb_font_t* hb_font, void* font_data, unsigned count,
                           const hb_codepoint_t* glyphs,
                           unsigned int glyph_stride, hb_position_t* advances,
                           unsigned int advance_stride, void* user_data) {
  Font& font = *reinterpret_cast<Font*>(font_data);

  // Batch call getWidths since entry cost is not cheap.
  // Copy requred because getWidths is dense and hb is strided.
  std::vector<GlyphID> glyph(count);
  std::vector<float> advance(count);
  for (unsigned i = 0; i < count; i++) {
    advance[i] = font.GetHorizontalAdvance(*glyphs);
    glyphs = reinterpret_cast<const hb_codepoint_t*>(
        reinterpret_cast<const char*>(glyphs) + glyph_stride);
  }
  //  font.getWidths(glyph.get(), count, advance.get());

  if (!font.IsSubpixel()) {
    for (unsigned i = 0; i < count; i++) {
      advance[i] = SkScalarRoundToInt(advance[i]);
    }
  }

  // Copy the results back to the sparse array.
  for (unsigned i = 0; i < count; i++) {
    *advances = skhb_position(advance[i]);
    advances = reinterpret_cast<hb_position_t*>(
        reinterpret_cast<char*>(advances) + advance_stride);
  }
}

// HarfBuzz callback to retrieve glyph extents, mainly used by HarfBuzz for
// fallback mark positioning, i.e. the situation when the font does not have
// mark anchors or other mark positioning rules, but instead HarfBuzz is
// supposed to heuristically place combining marks around base glyphs. HarfBuzz
// does this by measuring "ink boxes" of glyphs, and placing them according to
// Unicode mark classes. Above, below, centered or left or right, etc.
hb_bool_t skhb_glyph_extents(hb_font_t* hb_font, void* font_data,
                             hb_codepoint_t hbGlyph,
                             hb_glyph_extents_t* extents, void* user_data) {
  Font& font = *reinterpret_cast<Font*>(font_data);
  TTASSERT(extents);

  RectF sk_bounds;
  GlyphID skGlyph = static_cast<GlyphID>(hbGlyph);

  float rect_ltwh[4];
  font.GetWidthBound(rect_ltwh, skGlyph);
  if (!font.IsSubpixel()) {
    sk_bounds = RectF::MakeLTWH(SkIntToScalar(SkScalarFloorToInt(rect_ltwh[0])),
                                SkIntToScalar(SkScalarFloorToInt(rect_ltwh[1])),
                                SkIntToScalar(SkScalarCeilToInt(rect_ltwh[2])),
                                SkIntToScalar(SkScalarCeilToInt(rect_ltwh[3])));
  } else {
    sk_bounds =
        RectF::MakeLTWH(rect_ltwh[0], rect_ltwh[1], rect_ltwh[2], rect_ltwh[3]);
  }

  // Skia is y-down but HarfBuzz is y-up.
  extents->x_bearing = skhb_position(sk_bounds.GetLeft());
  extents->y_bearing = skhb_position(-sk_bounds.GetTop());
  extents->width = skhb_position(sk_bounds.GetWidth());
  extents->height = skhb_position(-sk_bounds.GetHeight());
  return true;
}

#define SK_HB_VERSION_CHECK(x, y, z)                         \
  (HB_VERSION_MAJOR > (x)) ||                                \
      (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR > (y)) || \
      (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR == (y) && \
       HB_VERSION_MICRO >= (z))

hb_font_funcs_t* skhb_get_font_funcs() {
  static hb_font_funcs_t* const funcs = [] {
    // HarfBuzz will use the default (parent) implementation if they aren't set.
    hb_font_funcs_t* const funcs = tt_hb_font_funcs_create();
    tt_hb_font_funcs_set_variation_glyph_func(funcs, skhb_glyph, nullptr,
                                              nullptr);
    tt_hb_font_funcs_set_nominal_glyph_func(funcs, skhb_nominal_glyph, nullptr,
                                            nullptr);
#if SK_HB_VERSION_CHECK(2, 0, 0)
    tt_hb_font_funcs_set_nominal_glyphs_func(funcs, skhb_nominal_glyphs,
                                             nullptr, nullptr);
#else
    sk_ignore_unused_variable(skhb_nominal_glyphs);
#endif
    tt_hb_font_funcs_set_glyph_h_advance_func(funcs, skhb_glyph_h_advance,
                                              nullptr, nullptr);
#if SK_HB_VERSION_CHECK(1, 8, 6)
    tt_hb_font_funcs_set_glyph_h_advances_func(funcs, skhb_glyph_h_advances,
                                               nullptr, nullptr);
#else
    sk_ignore_unused_variable(skhb_glyph_h_advances);
#endif
    tt_hb_font_funcs_set_glyph_extents_func(funcs, skhb_glyph_extents, nullptr,
                                            nullptr);
    tt_hb_font_funcs_make_immutable(funcs);
    return funcs;
  }();
  TTASSERT(funcs);
  return funcs;
}

// hb_blob_t* skhb_get_table(hb_face_t* face, hb_tag_t tag, void* user_data) {
//   ITypefaceHelper& typeface = *reinterpret_cast<ITypefaceHelper*>(user_data);
//
//   auto data = typeface.copyTableData(tag);
//   if (!data) {
//     return nullptr;
//   }
//   SkData* rawData = data.release();
//   return hb_blob_create(reinterpret_cast<char*>(rawData->writable_data()),
//                         rawData->size(), HB_MEMORY_MODE_READONLY, rawData,
//                         [](void* ctx) { SkSafeUnref(((SkData*)ctx)); });
// }

HBBlob stream_to_blob(const TypefaceRef typeface) {
  size_t size = typeface->GetFontDataSize();
  HBBlob blob;
  if (const void* base = typeface->GetFontData()) {
    blob.reset(tt_hb_blob_create((const char*)base, static_cast<uint32_t>(size),
                                 HB_MEMORY_MODE_READONLY, nullptr, nullptr));
  } else {
    TTASSERT(false);
    // SkDebugf("Extra SkStreamAsset copy\n");
    //    void* ptr = size ? sk_malloc_throw(size) : nullptr;
    //    asset->read(ptr, size);
    //    blob.reset(hb_blob_create((char*)ptr, SkToUInt(size),
    //                              HB_MEMORY_MODE_READONLY, ptr, sk_free));
  }
  TTASSERT(blob);
  tt_hb_blob_make_immutable(blob.get());
  return blob;
}

// SkDEBUGCODE(static hb_user_data_key_t gDataIdKey;)

HBFace create_hb_face(const TypefaceRef typeface) {
  int index = 0;
  HBFace face;
  if (typeface->GetFontData() != nullptr) {
    HBBlob blob(stream_to_blob(typeface));
    face.reset(tt_hb_face_create(blob.get(), (unsigned)index));
  }
  //  std::unique_ptr<SkStreamAsset> typefaceAsset =
  //  typeface.openStream(&index); if (typefaceAsset &&
  //  typefaceAsset->getMemoryBase()) {
  //    HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
  //    face.reset(hb_face_create(blob.get(), (unsigned)index));
  //  } else {
  //    face.reset(hb_face_create_for_tables(
  //        skhb_get_table, static_cast<void*>(typeface.get()),
  //        [](void* user_data) {
  ////          reinterpret_cast<ITypefaceHelper*>(user_data);
  //        }));
  //  }
  TTASSERT(face);
  if (!face) {
    return nullptr;
  }
  tt_hb_face_set_index(face.get(), (unsigned)index);
  tt_hb_face_set_upem(face.get(), typeface->GetUnitsPerEm());

  //  SkDEBUGCODE(hb_face_set_user_data(face.get(), &gDataIdKey,
  //                                    const_cast<SkTypeface*>(&typeface),
  //                                    nullptr, false);)

  return face;
}

HBFont create_hb_font(const Font& font, const HBFace& face) {
  //  SkDEBUGCODE(void* dataId = hb_face_get_user_data(face.get(), &gDataIdKey);
  //              TTASSERT(dataId == font.getTypeface());)

  HBFont otFont(tt_hb_font_create(face.get()));
  TTASSERT(otFont);
  if (!otFont) {
    return nullptr;
  }
  tt_hb_ot_font_set_funcs(otFont.get());
  // Creating a sub font means that non-available functions
  // are found from the parent.
  HBFont skFont(tt_hb_font_create_sub_font(otFont.get()));
  tt_hb_font_set_funcs(
      skFont.get(), skhb_get_font_funcs(),
      reinterpret_cast<void*>(new Font(font)),
      [](void* user_data) { delete reinterpret_cast<Font*>(user_data); });
  int scale = skhb_position(font.GetSize());
  tt_hb_font_set_scale(skFont.get(), scale, scale);

  return skFont;
}

using SkVector = PointF;

struct ShapedGlyph {
  GlyphID fID;
  uint32_t fCluster;
  PointF fOffset;
  SkVector fAdvance;
  bool fHasVisual;
  bool fGraphemeBreakBefore;
  bool fUnsafeToBreak;
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

class ShaperHarfBuzz : public SkShaper {
 public:
  ShaperHarfBuzz(
      //                   std::unique_ptr<SkUnicode>,
      //                   SkUnicodeBreak line,
      //                   SkUnicodeBreak grapheme,
      HBBuffer);

 protected:
  //    std::unique_ptr<SkUnicode> fUnicode;
  //    SkUnicodeBreak fLineBreakIterator;
  //    SkUnicodeBreak fGraphemeBreakIterator;

  ShapedRun shape(const char32_t* const utf8, const size_t utf8Bytes,
                  const char32_t* const utf8Start,
                  const char32_t* const utf8End, bool bidi,
                  const char* language, const SkFourByteTag& script,
                  const FontRunIterator& font, const Feature* const features,
                  const size_t featuresSize) const;

 private:
  //  const sk_sp<SkFontMgr> fFontMgr;
  HBBuffer fBuffer;
  hb_language_t fUndefinedLanguage;

  void shape(const char32_t* utf8, size_t utf8Bytes, FontRunIterator*,
             bool leftToRight, SkScalar width, RunHandler*) const override;

  virtual void wrap(const char32_t* const utf8, size_t utf8Bytes, bool,
                    const char*, const SkFourByteTag, FontRunIterator*,
                    const Feature*, size_t featuresSize, SkScalar width,
                    RunHandler*) const = 0;
};

class ShapeDontWrapOrReorder : public ShaperHarfBuzz {
 public:
  using ShaperHarfBuzz::ShaperHarfBuzz;

 private:
  void wrap(const char32_t* const utf8, size_t utf8Bytes, bool, const char*,
            const SkFourByteTag, FontRunIterator*, const Feature*,
            size_t featuresSize, float width, RunHandler*) const override;
};

ShaperHarfBuzz::ShaperHarfBuzz(
    //    std::unique_ptr<SkUnicode> unicode,
    //    SkUnicodeBreak lineIter, SkUnicodeBreak graphIter,
    HBBuffer buffer)
    :  //      fUnicode(std::move(unicode))
       //    , fLineBreakIterator(std::move(lineIter))
       //    , fGraphemeBreakIterator(std::move(graphIter))
       //    ,
       //      fFontMgr(std::move(fontmgr)),
      fBuffer(std::move(buffer)),
      fUndefinedLanguage(tt_hb_language_from_string("und", -1)) {}

void ShaperHarfBuzz::shape(const char32_t* utf8, size_t utf8Bytes,
                           FontRunIterator* font, bool leftToRight,
                           SkScalar width, RunHandler* handler) const {
  //  SkBidiIterator::Level defaultLevel =
  //      leftToRight ? SkBidiIterator::kLTR : SkBidiIterator::kRTL;
  //  std::unique_ptr<BiDiRunIterator> bidi(MakeSkUnicodeBidiRunIterator(
  //      fUnicode.get(), utf8, utf8Bytes, defaultLevel));

  // ltr 0, rtl 1
  auto bidi = !leftToRight;
  //  auto bidi = MakeBiDiRunIterator(utf8, utf8Bytes, !leftToRight);
  //  if (!bidi) {
  //    return;
  //  }

  auto language = std::locale().name();
  //  std::unique_ptr<LanguageRunIterator> language(
  //      MakeStdLanguageRunIterator(utf8, utf8Bytes));
  //  if (!language) {
  //    return;
  //  }

  auto script = 0;  // SkSetFourByteTag(0, 0, 0, 0);
  //  std::unique_ptr<ScriptRunIterator> script(
  //      MakeSkUnicodeHbScriptRunIterator(fUnicode.get(), utf8, utf8Bytes));
  //  if (!script) {
  //    return;
  //  }

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

using SkFontID = uint32_t;
using Mutex = std::mutex;
class HBLockedFaceCache {
 public:
  HBLockedFaceCache(android::LruCache<SkFontID, HBFace>* lruCache, Mutex* mutex)
      : fLRUCache(lruCache), fMutex(mutex) {
    fMutex->lock();
  }
  HBLockedFaceCache(const HBLockedFaceCache&) = delete;
  HBLockedFaceCache& operator=(const HBLockedFaceCache&) = delete;
  HBLockedFaceCache(HBLockedFaceCache&&) = delete;
  HBLockedFaceCache& operator=(HBLockedFaceCache&&) = delete;

  ~HBLockedFaceCache() { fMutex->unlock(); }

  HBFace* find(SkFontID fontId) { return fLRUCache->get(fontId); }
  HBFace* insert(SkFontID fontId, HBFace hbFace) {
    return fLRUCache->put(fontId, std::move(hbFace));
  }
  void reset() { fLRUCache->clear(); }

 private:
  android::LruCache<SkFontID, HBFace>* fLRUCache;
  Mutex* fMutex;
};
static HBLockedFaceCache get_hbFace_cache() {
  static Mutex gHBFaceCacheMutex;
  static android::LruCache<SkFontID, HBFace> gHBFaceCache(100);
  return HBLockedFaceCache(&gHBFaceCache, &gHBFaceCacheMutex);
}

ShapedRun ShaperHarfBuzz::shape(
    const char32_t* const utf8, const size_t utf8Bytes,
    const char32_t* const utf8Start, const char32_t* const utf8End, bool bidi,
    const char* language, const SkFourByteTag& script,
    const FontRunIterator& font, const Feature* const features,
    const size_t featuresSize) const {
  size_t utf8runLength = utf8End - utf8Start;
  ShapedRun run(RunHandler::Range(utf8Start - utf8, utf8runLength),
                font.currentFont(), bidi, nullptr, 0);

  hb_buffer_t* buffer = fBuffer.get();
  SkAutoTCallVProc<hb_buffer_t, tt_hb_buffer_clear_contents> autoClearBuffer(
      buffer);
  tt_hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
  tt_hb_buffer_set_cluster_level(buffer,
                                 HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  // Documentation for HB_BUFFER_FLAG_BOT/EOT at
  // 763e5466c0a03a7c27020e1e2598e488612529a7. Currently BOT forces a dotted
  // circle when first codepoint is a mark; EOT has no effect. Avoid adding
  // dotted circle, re-evaluate if BOT/EOT change. See https://skbug.com/9618.
  // hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

  // Add precontext.
  tt_hb_buffer_add_utf32(
      buffer, reinterpret_cast<uint32_t*>(const_cast<char32_t*>(utf8)),
      static_cast<uint32_t>(utf8Start - utf8),
      static_cast<uint32_t>(utf8Start - utf8), 0);

  // Populate the hb_buffer directly with utf8 cluster indexes.
  const char32_t* utf8Current = utf8Start;
  while (utf8Current < utf8End) {
    unsigned int cluster = static_cast<unsigned int>(utf8Current - utf8);
    hb_codepoint_t u = *utf8Current++;  // utf8_next(&utf8Current, utf8End);
    tt_hb_buffer_add(buffer, u, cluster);
  }

  // Add postcontext.
  tt_hb_buffer_add_utf32(
      buffer, reinterpret_cast<uint32_t*>(const_cast<char32_t*>(utf8Current)),
      static_cast<uint32_t>(utf8 + utf8Bytes - utf8Current), 0, 0);

  hb_direction_t direction = is_LTR(bidi) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
  tt_hb_buffer_set_direction(buffer, direction);
  tt_hb_buffer_set_script(buffer,
                          tt_hb_script_from_iso15924_tag((hb_tag_t)script));
  // Buffers with HB_LANGUAGE_INVALID race since hb_language_get_default is not
  // thread safe. The user must provide a language, but may provide data
  // hb_language_from_string cannot use. Use "und" for the undefined language in
  // this case (RFC5646 4.1 5).
  hb_language_t hbLanguage = tt_hb_language_from_string(language, -1);
  if (hbLanguage == HB_LANGUAGE_INVALID) {
    hbLanguage = fUndefinedLanguage;
  }
  tt_hb_buffer_set_language(buffer, hbLanguage);
  tt_hb_buffer_guess_segment_properties(buffer);

  // TODO(hfuttyh): better cache HBFace (data) / hbfont (typeface)
  // An HBFace is expensive (it sanitizes the bits).
  // An HBFont is fairly inexpensive.
  // An HBFace is actually tied to the data, not the typeface.
  // The size of 100 here is completely arbitrary and used to match libtxt.
  HBFont hbFont;
  {
    HBLockedFaceCache cache = get_hbFace_cache();
    SkFontID dataId = font.currentFont().GetTypeface()->GetUniqueId();
    HBFace* hbFaceCached = cache.find(dataId);
    if (!hbFaceCached) {
      HBFace hbFace(create_hb_face(font.currentFont().GetTypeface()));
      hbFaceCached = cache.insert(dataId, std::move(hbFace));
    }
    hbFont = create_hb_font(font.currentFont(), *hbFaceCached);
  }
  if (!hbFont) {
    return run;
  }

  std::vector<hb_feature_t> hbFeatures(32);
  //  for (const auto& feature : SkSpan(features, featuresSize)) {
  //    if (feature.end < SkTo<size_t>(utf8Start - utf8) ||
  //        SkTo<size_t>(utf8End - utf8) <= feature.start) {
  //      continue;
  //    }
  //    if (feature.start <= SkTo<size_t>(utf8Start - utf8) &&
  //        SkTo<size_t>(utf8End - utf8) <= feature.end) {
  //      hbFeatures.push_back({(hb_tag_t)feature.tag, feature.value,
  //                            HB_FEATURE_GLOBAL_START,
  //                            HB_FEATURE_GLOBAL_END});
  //    } else {
  //      hbFeatures.push_back({(hb_tag_t)feature.tag, feature.value,
  //                            SkTo<unsigned>(feature.start),
  //                            SkTo<unsigned>(feature.end)});
  //    }
  //  }

  tt_hb_shape(hbFont.get(), buffer, hbFeatures.data(),
              static_cast<uint32_t>(hbFeatures.size()));
  unsigned len = tt_hb_buffer_get_length(buffer);
  if (len == 0) {
    return run;
  }

  if (direction == HB_DIRECTION_RTL) {
    // Put the clusters back in logical order.
    // Note that the advances remain ltr.
    tt_hb_buffer_reverse(buffer);
  }
  hb_glyph_info_t* info = tt_hb_buffer_get_glyph_infos(buffer, nullptr);
  hb_glyph_position_t* pos = tt_hb_buffer_get_glyph_positions(buffer, nullptr);

  run = ShapedRun(RunHandler::Range(utf8Start - utf8, utf8runLength),
                  font.currentFont(), bidi,
                  std::unique_ptr<ShapedGlyph[]>(new ShapedGlyph[len]), len);

  // Undo skhb_position with (1.0/(1<<16)) and scale as needed.
  double SkScalarFromHBPosX = +(1.52587890625e-5) * run.fFont.GetScaleX();
  double SkScalarFromHBPosY =
      -(1.52587890625e-5);  // HarfBuzz y-up, Skia y-down
  SkVector runAdvance = {0, 0};
  for (unsigned i = 0; i < len; i++) {
    ShapedGlyph& glyph = run.fGlyphs[i];
    glyph.fID = info[i].codepoint;
    glyph.fCluster = info[i].cluster;
    glyph.fOffset.SetX(pos[i].x_offset * SkScalarFromHBPosX);
    glyph.fOffset.SetY(pos[i].y_offset * SkScalarFromHBPosY);
    glyph.fAdvance.SetX(pos[i].x_advance * SkScalarFromHBPosX);
    glyph.fAdvance.SetY(pos[i].y_advance * SkScalarFromHBPosY);

#if 0
        SkRect bounds;
        SkScalar advance;
        SkPaint p;
        run.fFont.getWidthsBounds(&glyph.fID, 1, &advance, &bounds, &p);
        glyph.fHasVisual = !bounds.isEmpty();
        // !font->currentTypeface()->glyphBoundsAreZero(glyph.fID);
#else
    // yulitao: It takes no effect whether true or false, but performance
    // improves.
    glyph.fHasVisual = true;
#endif

#if SK_HB_VERSION_CHECK(1, 5, 0)
    glyph.fUnsafeToBreak = info[i].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK;
#else
    glyph.fUnsafeToBreak = false;
#endif
    //    glyph.fMustLineBreakBefore = false;

    runAdvance += glyph.fAdvance;
  }
  run.fAdvance = runAdvance;

  return run;
}
}  // namespace
std::unique_ptr<SkShaper> SkShaper::MakeShapeDontWrapOrReorderForHB() {
  HBBuffer buffer(tt_hb_buffer_create());
  if (!buffer) {
    //    SkDEBUGF("Could not create hb_buffer");
    return nullptr;
  }

  //    auto unicode = SkUnicode::Make();
  //    if (!unicode) {
  //        return nullptr;
  //    }

  return std::make_unique<ShapeDontWrapOrReorder>(
      /*std::move(unicode), nullptr, nullptr, */ std::move(buffer));
}

void SkShaper::PurgeHarfBuzzCache() {
  HBLockedFaceCache cache = get_hbFace_cache();
  cache.reset();
}
}  // namespace tttext
}  // namespace ttoffice
