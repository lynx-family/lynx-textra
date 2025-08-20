// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/icu_wrapper.h>

#ifdef __ANDROID__
#include <dirent.h>

#include <cctype>
#else
#include <unistd.h>
#endif

#include <dlfcn.h>
#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "src/textlayout/utils/log_util.h"
#include "unicode/putil.h"
#include "unicode/ubidi.h"
#include "unicode/ubrk.h"
#include "unicode/uchar.h"
#include "unicode/ucnv.h"
#include "unicode/umachine.h"
#include "unicode/uscript.h"
#include "unicode/utypes.h"
#ifndef USE_LIBLINEBREAK
#include "src/textlayout/utils/u_8_string.h"
#else
#include <textra/macro.h>

#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/graphemebreak.h"
#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/linebreak.h"
#include "src/textlayout/icu_substitute/linebreak/libunibreak/src/wordbreak.h"
#endif

// #if defined(__APPLE__) || defined(__MACH__)
// #define ICU_I18N_LIB "/usr/local/Cellar/icu4c/70.1/lib/libicui18n.dylib"
// #define ICU_UC_LIB "/usr/local/Cellar/icu4c/70.1/lib/libicuuc.dylib"
// #define ICU_CURRENT_PATH "/usr/local/Cellar/icu4c"
// #elif defined(__ANDROID__)
// #define ICU_I18N_LIB "libicui18n.so"
// #define ICU_UC_LIB "libicuuc.so"
// #define ICU_DATA_FOLDER "/system/usr/icu"
// #elif defined(__linux__)
// #define ICU_I18N_LIB "libicui18n.so"
// #define ICU_UC_LIB "libicuuc.so"
// #define ICU_CURRENT_PATH "/usr/lib/x86_64-linux-gnu/icu/current"
// #endif

#define ICUDATA_VERSION_MIN_LENGTH 2
#define ICUDATA_VERSION_MAX_LENGTH 30
#define ICUDATA_VERSION_MIN 44

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static char g_icudata_version[ICUDATA_VERSION_MAX_LENGTH + 1];

static bool valid_library = false;

static void* handle_common = nullptr;
static void* handle_i18n = nullptr;

namespace ttoffice {
namespace tttext {
#ifdef __ANDROID__
static int __icu_dat_file_filter(const dirent* dirp) {
  const char* name = dirp->d_name;

  // Is the name the right length to match 'icudt(\d\d\d)l.dat'?
  const size_t len = strlen(name);
  if (len < 10 + ICUDATA_VERSION_MIN_LENGTH ||
      len > 10 + ICUDATA_VERSION_MAX_LENGTH) {
    return 0;
  }

  // Check that the version is a valid decimal number.
  for (size_t i = 5; i < len - 5; i++) {
    if (!isdigit(name[i])) {
      return 0;
    }
  }

  return !strncmp(name, "icudt", 5) && !strncmp(&name[len - 5], "l.dat", 5);
}
#endif

static void init_icudata_version() {
  int max_version = -1;
#ifdef __ANDROID__
  dirent** namelist = nullptr;
  int n =
      scandir(ICU_DATA_FOLDER, &namelist, &__icu_dat_file_filter, alphasort);
  if (nullptr == namelist) {
    // LogUtil::E("Cannot locate ICU data file at %s", ICU_DATA_FOLDER);
    return;
  }
  while (n--) {
    // We prefer the latest version available.
    int version = atoi(&namelist[n]->d_name[strlen("icudt")]);
    if (version != 0 && version > max_version) {
      max_version = version;
    }
    free(namelist[n]);
  }
  free(namelist);
#else
  //  char buf[ICUDATA_VERSION_MAX_LENGTH + 2];
  //  readlink(ICU_CURRENT_PATH, buf, ICUDATA_VERSION_MAX_LENGTH + 1);
  //  buf[ICUDATA_VERSION_MAX_LENGTH + 1] = '\0';
  //  char *dot = strchr(buf, '.');
  //  if (nullptr != dot) {
  //    *dot = '\0';
  //    int version = atoi(buf);
  //    version = 70;
  //    if (version != 0 && version > max_version) {
  //      max_version = version;
  //    }
  //  }
  max_version = 70;
#endif
  if (max_version < ICUDATA_VERSION_MIN) {
    // LogUtil::E("Cannot find valid version of ICU data file.");
    return;
  }

  snprintf(g_icudata_version, sizeof(g_icudata_version), "_%d", max_version);

  handle_i18n = dlopen(ICU_I18N_LIB, RTLD_LAZY | RTLD_LOCAL);
  if (handle_i18n == nullptr) {
    LogUtil::E("Could not open libicui18n: %s", dlerror());
    return;
  }

  handle_common = dlopen(ICU_UC_LIB, RTLD_LAZY | RTLD_LOCAL);
  if (handle_common == nullptr) {
    LogUtil::E("Could not open libicuuc: %s", dlerror());
    return;
  }

  valid_library = true;
}

static void* do_dlsym(void** handle, const char* name) {
  pthread_once(&once_control, &init_icudata_version);

  std::vector<char> versioned_symbol_name(strlen(name) +
                                          sizeof(g_icudata_version));
  snprintf(versioned_symbol_name.data(), versioned_symbol_name.size(), "%s%s",
           name, g_icudata_version);

  return dlsym(*handle, versioned_symbol_name.data());
}

UBreakIterator* brk_open(UBreakIteratorType type, const char* locale,
                         const UChar* text, int32_t textLength,
                         UErrorCode* status) {
  typedef decltype(&ubrk_open) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubrk_open"));
  return ptr(type, locale, text, textLength, status);
}

void brk_close(UBreakIterator* bi) {
  typedef decltype(&ubrk_close) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubrk_close"));
  ptr(bi);
}

int32_t brk_first(UBreakIterator* bi) {
  typedef decltype(&ubrk_first) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubrk_first"));
  return ptr(bi);
}

int32_t brk_next(UBreakIterator* bi) {
  typedef decltype(&ubrk_next) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubrk_next"));
  return ptr(bi);
}

void brk_setText(UBreakIterator* bi, const UChar* text, int32_t textLength,
                 UErrorCode* status) {
  typedef decltype(&ubrk_setText) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubrk_setText"));
  return ptr(bi, text, textLength, status);
}

UScriptCode uscript_getScript(uint32_t codepoint, UErrorCode* pErrorCode) {
  typedef decltype(&uscript_getScript) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "uscript_getScript"));
  return ptr(codepoint, pErrorCode);
}

UScriptCode ICUWrapper::uscriptGetScript(uint32_t codepoint,
                                         UErrorCode* pErrorCode) {
  return uscript_getScript(codepoint, pErrorCode);
}

UBiDi* ubidi_openSized(int32_t maxLength, int32_t maxRunCount,
                       UErrorCode* pErrorCode) {
  typedef decltype(&ubidi_openSized) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_openSized"));
  return ptr(maxLength, maxRunCount, pErrorCode);
}

UBiDi* ICUWrapper::ubidiOpenSized(int32_t maxLength, int32_t maxRunCount,
                                  UErrorCode* pErrorCode) {
  return tttext::ubidi_openSized(maxLength, maxRunCount, pErrorCode);
}

void ubidi_setPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                   UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                   UErrorCode* pErrorCode) {
  typedef decltype(&ubidi_setPara) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_setPara"));
  ptr(pBiDi, text, length, paraLevel, embeddingLevels, pErrorCode);
}

void ICUWrapper::ubidiSetPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                              UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                              UErrorCode* pErrorCode) {
  return tttext::ubidi_setPara(pBiDi, text, length, paraLevel, embeddingLevels,
                               pErrorCode);
}

UBiDiDirection ubidi_getDirection(const UBiDi* pBiDi) {
  typedef decltype(&ubidi_getDirection) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_getDirection"));
  return ptr(pBiDi);
}

UBiDiDirection ICUWrapper::ubidiGetDirection(const UBiDi* pBiDi) {
  return tttext::ubidi_getDirection(pBiDi);
}

void ubidi_close(UBiDi* pBiDi) {
  typedef decltype(&ubidi_close) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_close"));
  ptr(pBiDi);
}

void ICUWrapper::ubidiClose(UBiDi* pBiDi) { return tttext::ubidi_close(pBiDi); }

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

UBiDi* bidi_openSized(int32_t maxLength, int32_t maxRunCount,
                      UErrorCode* pErrorCode) {
  typedef decltype(&ubidi_openSized) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_openSized"));
  return ptr(maxLength, maxRunCount, pErrorCode);
}

void bidi_close(UBiDi* para) {
  typedef decltype(&ubidi_close) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_close"));
  // ubidi_close(para);
  return ptr(para);
}

void bidi_setPara(UBiDi* pBiDi, const UChar* text, int32_t length,
                  UBiDiLevel paraLevel, UBiDiLevel* embeddingLevels,
                  UErrorCode* pErrorCode) {
  // ubidi_setPara(pBiDi, text, length, paraLevel, embeddingLevels, pErrorCode);
  typedef decltype(&ubidi_setPara) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_setPara"));
  return ptr(pBiDi, text, length, paraLevel, embeddingLevels, pErrorCode);
}

const UBiDiLevel* bidi_getLevels(UBiDi* content, UErrorCode* pErrorCode) {
  // return ubidi_getLevels(content, pErrorCode);
  typedef decltype(&ubidi_getLevels) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_getLevels"));
  return ptr(content, pErrorCode);
}

void bidi_getVisualMap(UBiDi* pBiDi, int32_t* indexMap,
                       UErrorCode* pErrorCode) {
  // ubidi_getVisualMap(pBiDi,indexMap,pErrorCode);
  typedef decltype(&ubidi_getVisualMap) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "ubidi_getVisualMap"));
  return ptr(pBiDi, indexMap, pErrorCode);
}

void bidi_getLogicalMap(UBiDi* pBiDi, int32_t* indexMap,
                        UErrorCode* pErrorCode) {
  // ubidi_getLogicalMap(pBiDi,indexMap,pErrorCode);
  typedef decltype(&ubidi_getLogicalMap) FuncPtr;
  static auto ptr = reinterpret_cast<FuncPtr>(
      do_dlsym(&handle_common, "ubidi_getLogicalMap"));
  return ptr(pBiDi, indexMap, pErrorCode);
}

ICUWrapper::ICUWrapper() {
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
}

ICUWrapper::~ICUWrapper() {
  if (char_brk_iter_) {
    brk_close(char_brk_iter_);
  }
  if (word_brk_iter_) {
    brk_close(word_brk_iter_);
  }
  if (line_brk_iter_) {
    brk_close(line_brk_iter_);
  }
}

ICUWrapper& ICUWrapper::GetInstance() {
  static ICUWrapper wrapper;
  return wrapper;
}

void ICUWrapper::BidiInit(const char32_t* u32_content, const uint32_t& length,
                          WriteDirection direction, uint8_t* bidi_levels,
                          uint32_t* visual_map, uint32_t* logical_map) {
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
  auto* para = bidi_openSized(u16_length, 0, pErrorCode);
  bidi_setPara(para, reinterpret_cast<const UChar*>(u16_string.c_str()),
               u16_length, para_l, NULL, pErrorCode);
  const auto* bidiLevels = bidi_getLevels(para, pErrorCode);
  auto* vmap = new int32_t[u16_length];
  bidi_getVisualMap(para, vmap, pErrorCode);
  auto* lmap = new int32_t[u16_length];
  bidi_getLogicalMap(para, lmap, pErrorCode);
  auto u16_idx = 0u;
  auto u32_idx = 0;
  while (u16_idx < u16_length) {
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
  delete[] vmap;
  delete[] lmap;
  bidi_close(para);
}

int8_t ICUWrapper::icu_charType(uint32_t c) {
  typedef decltype(&u_charType) FuncPtr;
  static auto ptr =
      reinterpret_cast<FuncPtr>(do_dlsym(&handle_common, "u_charType"));
  return ptr(c);
}

void ICUWrapper::icu_boundary_breaker(const char32_t* u32_content,
                                      uint32_t char_count,
                                      std::vector<BoundaryType>& boundary) {
#ifndef USE_LIBLINEBREAK
  // 1. string32 to string16, and record the position of char32 and correspond
  // char16
  auto u16str = base::U32StringToU16(u32_content, char_count);
  std::vector<int> u16_to_u32_map;
  u16_to_u32_map.reserve(char_count);
  int u32_str_indx = 0;
  for (size_t i = 0; i < u16str.length(); i++) {
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
  icu_break_char(char_brk_iter_, u16str.data(), u16str.length(),
                 break_result.data());
  for (size_t i = 0; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kGraphme;
    }
  }
  // 3. word break
  std::fill(break_result.begin(), break_result.end(), 0);
  icu_break_word(word_brk_iter_, u16str.data(), u16str.length(),
                 break_result.data());
  for (size_t i = 0; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kWord;
    }
  }
  // 4. line break
  std::fill(break_result.begin(), break_result.end(), 0);
  icu_break_line(line_brk_iter_, u16str.data(), u16str.length(),
                 break_result.data());
  for (size_t i = 0; i < break_result.size(); i++) {
    if (break_result[i]) {
      boundary[u16_to_u32_map[i]] = BoundaryType::kLineBreakable;
    }
  }
#else
  std::vector<char> brks(char_count, 0);
  init_graphemebreak();
  set_graphemebreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                           char_count, "zh", brks.data());
  for (auto k = 0; k < char_count; k++) {
    if (brks[k] == GRAPHEMEBREAK_BREAK) {
      boundary[k] = textlayout::BoundaryType::kGraphme;
    }
    TTASSERT(brks[k] != GRAPHEMEBREAK_INSIDEACHAR);
  }

  init_wordbreak();
  set_wordbreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                       char_count, "zh", brks.data());
  for (auto k = 0; k < char_count; k++) {
    if (brks[k] == WORDBREAK_BREAK) {
      TTASSERT(boundary[k] == textlayout::BoundaryType::kGraphme);
      boundary[k] = textlayout::BoundaryType::kWord;
    }
    TTASSERT(brks[k] != WORDBREAK_INSIDEACHAR);
  }

  init_linebreak();
  set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(u32_content),
                       char_count, "zh", brks.data());
  for (auto k = 0; k < char_count; k++) {
    if (brks[k] == LINEBREAK_ALLOWBREAK || brks[k] == LINEBREAK_MUSTBREAK) {
      // for example, in string 'you're', it's allow break after character ''',
      // but it's not a word boundary
      TTASSERT(boundary_[k] == BoundaryType::kWord ||
               u32_content[k] == U'\u2019');
      boundary[k] = textlayout::BoundaryType::kLine;
    }
    TTASSERT(brks[k] != LINEBREAK_INSIDEACHAR);
  }
  boundary.back() = textlayout::BoundaryType::kLine;
#endif
}

bool ICUWrapper::icu_hasBinaryProperty(UChar32 c, UProperty which) {
  typedef decltype(&u_hasBinaryProperty) FuncPtr;
  static auto ptr = reinterpret_cast<FuncPtr>(
      do_dlsym(&handle_common, "u_hasBinaryProperty"));
  return ptr(c, which);
}
}  // namespace tttext
}  // namespace ttoffice
