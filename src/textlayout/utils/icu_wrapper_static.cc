// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/icu_wrapper.h>

#include <algorithm>
#include <vector>
#ifdef __ANDROID__
#error "android should not use static icu"
#endif
#include <textra/macro.h>
#ifndef USE_LIBLINEBREAK
#include "src/textlayout/utils/u_8_string.h"
#ifdef USE_ICU
#include "unicode/putil.h"
#include "unicode/ubidi.h"
#include "unicode/ubrk.h"
#include "unicode/uchar.h"
#include "unicode/ucnv.h"
#include "unicode/umachine.h"
#include "unicode/utypes.h"
#endif
#else
#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/graphemebreak.h"
#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/linebreak.h"
#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/wordbreak.h"
#endif

namespace ttoffice {
namespace tttext {
#ifdef USE_ICU
#ifndef USE_LIBLINEBREAK
UBreakIterator* brk_open(UBreakIteratorType type, const char* locale,
                         const UChar* text, int32_t textLength,
                         UErrorCode* status) {
  return ubrk_open(type, locale, text, textLength, status);
}
void brk_close(UBreakIterator* bi) { ubrk_close(bi); }
int32_t brk_first(UBreakIterator* bi) { return ubrk_first(bi); }
int32_t brk_next(UBreakIterator* bi) { return ubrk_next(bi); }
void brk_setText(UBreakIterator* bi, const UChar* text, int32_t textLength,
                 UErrorCode* status) {
  ubrk_setText(bi, text, textLength, status);
}
void icu_break_char(UBreakIterator* breaker, const UChar* text,
                    uint32_t textLength, uint8_t* res) {
  UErrorCode status = U_ZERO_ERROR;
  brk_setText(breaker, text, textLength, &status);
  auto curr = brk_first(breaker);
  while (curr != UBRK_DONE) {
    curr = brk_next(breaker);
    if (curr == UBRK_DONE) {
      break;
    }
    res[curr - 1] = true;
  }
}
void icu_break_word(UBreakIterator* breaker, const UChar* text,
                    uint32_t textLength, uint8_t* res) {
  UErrorCode status = U_ZERO_ERROR;
  brk_setText(breaker, text, textLength, &status);
  auto curr = brk_first(breaker);
  while (curr != UBRK_DONE) {
    curr = brk_next(breaker);
    if (curr == UBRK_DONE) {
      break;
    }
    res[curr - 1] = true;
  }
}
void icu_break_line(UBreakIterator* breaker, const UChar* text,
                    uint32_t textLength, uint8_t* res) {
  UErrorCode status = U_ZERO_ERROR;
  brk_setText(breaker, text, textLength, &status);
  auto curr = brk_first(breaker);
  while (curr != UBRK_DONE) {
    curr = brk_next(breaker);
    if (curr == UBRK_DONE) {
      break;
    }
    res[curr - 1] = true;
  }
}
#endif
UBiDi* bidi_openSized(int32_t maxLength, int32_t maxRunCount,
                      UErrorCode* pErrorCode) {
  return ubidi_openSized(maxLength, maxRunCount, pErrorCode);
}

void bidi_close(UBiDi* para) { ubidi_close(para); }

void bidi_setPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                  UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                  UErrorCode* pErrorCode) {
  ubidi_setPara(pBiDi, text, length, paraLevel, embeddingLevels, pErrorCode);
}

const UBiDiLevel* bidi_getLevels(UBiDi* content, UErrorCode* pErrorCode) {
  return ubidi_getLevels(content, pErrorCode);
}

void bidi_getVisualMap(UBiDi* pBiDi, int32_t* indexMap,
                       UErrorCode* pErrorCode) {
  ubidi_getVisualMap(pBiDi, indexMap, pErrorCode);
}

void bidi_getLogicalMap(UBiDi* pBiDi, int32_t* indexMap,
                        UErrorCode* pErrorCode) {
  ubidi_getLogicalMap(pBiDi, indexMap, pErrorCode);
}
#endif

ICUWrapper::ICUWrapper() {
#ifdef USE_ICU
  // u_setDataDirectory(ICU_DATA_FOLDER);
  UErrorCode status = U_ZERO_ERROR;
  if (!char_brk_iter_) {
    char_brk_iter_ = brk_open(UBRK_CHARACTER, "zh", NULL, 0, &status);
  }
  if (!word_brk_iter_) {
    word_brk_iter_ = brk_open(UBRK_WORD, "zh", NULL, 0, &status);
  }
  if (!line_brk_iter_) {
    line_brk_iter_ = brk_open(UBRK_LINE, "zh", NULL, 0, &status);
  }
  // if (!bidi_handler_) {
  //   bidi_handler_ = bidi_openSized(0, 0, &status);
  // }
#endif
}

ICUWrapper::~ICUWrapper() {
#ifdef USE_ICU
  if (char_brk_iter_) {
    brk_close(char_brk_iter_);
  }
  if (word_brk_iter_) {
    brk_close(word_brk_iter_);
  }
  if (line_brk_iter_) {
    brk_close(line_brk_iter_);
  }
  // if (bidi_handler_) {
  //   bidi_close(bidi_handler_);
  // }
#endif
}

ICUWrapper& ICUWrapper::GetInstance() {
  static ICUWrapper wrapper;
  return wrapper;
}

void ICUWrapper::BidiInit(const char32_t* u32_content, const uint32_t& length,
                          WriteDirection direction, uint8_t* bidi_levels,
                          uint32_t* visual_map, uint32_t* logical_map) {
#ifdef USE_ICU
  int para_l = UBIDI_DEFAULT_LTR;
  if (direction == WriteDirection::kLTR || direction == WriteDirection::kTTB) {
    para_l = UBIDI_LTR;
  } else if (direction == WriteDirection::kRTL ||
             direction == WriteDirection::kBTT) {
    para_l = UBIDI_RTL;
  }
  UErrorCode ErrorCode = U_ZERO_ERROR;
  UErrorCode* pErrorCode = &ErrorCode;
  auto u16_string = base::U32StringToU16(u32_content, length);
  auto u16_length = u16_string.length();
  if (u16_length == 0) return;
  auto* para = bidi_openSized(static_cast<int32_t>(u16_length), 0, pErrorCode);
  bidi_setPara(para, reinterpret_cast<const UChar*>(u16_string.c_str()),
               static_cast<int32_t>(u16_length), para_l, NULL, pErrorCode);
  const auto* bidiLevels = bidi_getLevels(para, pErrorCode);
  auto* vmap = new int32_t[u16_length];
  bidi_getVisualMap(para, vmap, pErrorCode);
  auto* lmap = new int32_t[u16_length];
  bidi_getLogicalMap(para, lmap, pErrorCode);
  size_t u16_idx = 0;
  size_t u32_idx = 0;
  while ((size_t)u16_idx < u16_length) {
    bidi_levels[u32_idx] = bidiLevels[u16_idx];
    visual_map[u32_idx] = vmap[u16_idx];
    logical_map[u32_idx] = lmap[u16_idx];
    u32_idx++;
    if (base::IsSurrogate(u16_string[u16_idx])) {
      u16_idx += 2;
    } else {
      u16_idx += 1;
    }
  }
  TTASSERT(u32_idx == length);
  delete[] vmap;
  delete[] lmap;
  bidi_close(para);
#else
  TTASSERT(false);
#endif
}
int8_t ICUWrapper::icu_charType(uint32_t c) {
#ifdef USE_ICU
  return u_charType(c);
#else
  TTASSERT(false);
  return 0;
#endif
}
void ICUWrapper::icu_boundary_breaker(const char32_t* u32_content,
                                      uint32_t char_count,
                                      std::vector<BoundaryType>& boundary) {
#ifdef USE_ICU
  // 1. string32 to string16, and record the position of char32 and correspond
  // char16
  auto u16str = base::U32StringToU16(u32_content, char_count);
  auto* uchar_str = reinterpret_cast<UChar*>(u16str.data());
  std::vector<int> u16_to_u32_map;
  u16_to_u32_map.reserve(char_count);
  int u32_str_indx = 0;
  for (auto i = 0u; i < u16str.length(); i++) {
    auto& ch = u16str[i];
    if (!base::IsSurrogate(ch)) {
      u16_to_u32_map.push_back(u32_str_indx);
    } else {
      u16_to_u32_map.push_back(u32_str_indx);
      i++;
      u16_to_u32_map.push_back(u32_str_indx);
    }
    u32_str_indx++;
  }
  // 2. char break
  std::vector<uint8_t> break_result(u16_to_u32_map.size(), 0);
  icu_break_char(char_brk_iter_, uchar_str,
                 static_cast<int32_t>(u16str.length()), break_result.data());
  for (auto i = 0u; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kGraphme;
    }
  }
  // 3. word break
  std::fill(break_result.begin(), break_result.end(), 0);
  icu_break_word(word_brk_iter_, uchar_str,
                 static_cast<uint32_t>(u16str.length()), break_result.data());
  for (auto i = 0u; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kWord;
    }
  }
  // 4. line break
  std::fill(break_result.begin(), break_result.end(), 0);
  icu_break_line(line_brk_iter_, uchar_str,
                 static_cast<uint32_t>(u16str.length()), break_result.data());
  for (auto i = 0u; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kLineBreakable;
    }
  }
#elif defined USE_LIBLINEBREAK
  std::vector<char> brks(char_count, 0);
  init_graphemebreak();
  set_graphemebreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                           char_count, "zh", brks.data());
  for (auto k = 0u; k < char_count; k++) {
    if (brks[k] == GRAPHEMEBREAK_BREAK) {
      boundary[k] = textlayout::BoundaryType::kGraphme;
    }
    TTASSERT(brks[k] != GRAPHEMEBREAK_INSIDEACHAR);
  }

  init_wordbreak();
  set_wordbreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                       char_count, "zh", brks.data());
  for (auto k = 0u; k < char_count; k++) {
    if (brks[k] == WORDBREAK_BREAK) {
      TTASSERT(boundary[k] == textlayout::BoundaryType::kGraphme);
      boundary[k] = textlayout::BoundaryType::kWord;
    }
    TTASSERT(brks[k] != WORDBREAK_INSIDEACHAR);
  }

  init_linebreak();
  set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                       char_count, "zh", brks.data());
  for (auto k = 0u; k < char_count; k++) {
    if (brks[k] == LINEBREAK_ALLOWBREAK || brks[k] == LINEBREAK_MUSTBREAK) {
      // for example, in string 'you're', it's allow break after character ''',
      // but it's not a word boundary
      TTASSERT(boundary[k] == textlayout::BoundaryType::kWord ||
               u32_content[k] == U'\u2019');
      boundary[k] = textlayout::BoundaryType::kLineBreakable;
    }
    TTASSERT(brks[k] != LINEBREAK_INSIDEACHAR);
  }
  boundary.back() = textlayout::BoundaryType::kLineBreakable;
#else
#endif
}
}  // namespace tttext
}  // namespace ttoffice
