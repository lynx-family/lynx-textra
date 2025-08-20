// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_ICU_WRAPPER_H_
#define PUBLIC_TEXTRA_ICU_WRAPPER_H_

#include <sys/cdefs.h>
#include <textra/layout_definition.h>
#include <textra/macro.h>

#include <vector>

#include "unicode/ubidi.h"
#include "unicode/uchar.h"
#include "unicode/uscript.h"

#define ICU_WRAPPER_GET_GC_MASK(c) \
  U_MASK(tttext::ICUWrapper::GetInstance().icu_charType(c))

struct UBreakIterator;
struct UBiDi;
namespace ttoffice {
namespace tttext {
/**
 * @brief This class provides a singleton interface to ICU (International
 * Components for Unicode) functionality, including:
 * - Bidirectional(bidi) text processing (for right-to-left languages like
 * Arabic)
 * - Text boundary analysis (character, word, and line boundaries)
 * - Character property detection
 *
 * The class handles dynamic loading of ICU libraries and uses RAII to manage
 * the lifecycle of ICU resources.
 */
class L_EXPORT ICUWrapper {
 public:
  ICUWrapper(const ICUWrapper&) = delete;
  void operator==(const ICUWrapper&) = delete;
  ~ICUWrapper();

 private:
  ICUWrapper();

 public:
  L_EXPORT static ICUWrapper& GetInstance();

 public:
  /**
   * @brief Bidirectional(bidi) text processing interface
   */

  /**
   * @brief Processes bidi text and generates bidi levels and mapping
   * information.
   *
   * @param[in] u32_content Pointer to UTF-32 encoded text content.
   * @param[in] length Length of the text content in characters.
   * @param[in] direction Writing direction of the paragraph.
   * @param[out] bidi_levels Bidi embedding levels for each character.
   * @param[out] visual_map Logical-to-visual character position mapping.
   * @param[out] logical_map Visual-to-logical character position mapping.
   */
  L_EXPORT static void BidiInit(const char32_t* u32_content,
                                const uint32_t& length,
                                WriteDirection direction, uint8_t* bidi_levels,
                                uint32_t* visual_map, uint32_t* logical_map);

  /**
   * @see uscript_getScript()
   */
  L_EXPORT static UScriptCode uscriptGetScript(uint32_t codepoint,
                                               UErrorCode* pErrorCode);

  /**
   * @see ubidi_openSized()
   */
  L_EXPORT static UBiDi* ubidiOpenSized(int32_t maxLength, int32_t maxRunCount,
                                        UErrorCode* pErrorCode);

  /**
   * @see ubidi_setPara()
   */
  L_EXPORT static void ubidiSetPara(UBiDi* pBiDi, const UChar* text,
                                    int32_t length, UBiDiLevel paraLevel,
                                    UBiDiLevel* embeddingLevels,
                                    UErrorCode* pErrorCode);

  /**
   * @see ubidi_getDirection()
   */
  L_EXPORT static UBiDiDirection ubidiGetDirection(const UBiDi* pBiDi);

  /**
   * @see ubidi_close()
   */
  L_EXPORT static void ubidiClose(UBiDi* pBiDi);

  /**
   * @brief Text boundary analysis interface
   */

  /**
   * @brief Analyzes text and determines boundary types (character, word, line,
   * etc) for each character. The boundary type is used for text layout
   * operations like line breaking.
   *
   * @param[in] u32_content Pointer to UTF-32 encoded text content.
   * @param[in] char_count Number of characters in the text.
   * @param[out] boundary Pre-allocated vector of size char_count.
   */
  void icu_boundary_breaker(const char32_t* u32_content, uint32_t char_count,
                            std::vector<BoundaryType>& boundary);

  /**
   * @brief Utility functions for character properties and analysis
   */

  /**
   * @brief Returns the Unicode general category value for a code point.
   * @see UCharCategory
   */
  int8_t icu_charType(uint32_t c);

  /**
   * @brief Tests if a character has a specific binary Unicode property.
   */
  L_EXPORT bool icu_hasBinaryProperty(UChar32 c, UProperty which);

  // private:
  //  // Bidi
  //  UBiDi *bidi_openSized(int32_t maxLength, int32_t maxRunCount,
  //                        UErrorCode *pErrorCode);
  //  void bidi_close(UBiDi *para);
  //
  //  void bidi_setPara(UBiDi *pBiDi, const UChar *text, int32_t length,
  //                    UBiDiLevel paraLevel, UBiDiLevel *embeddingLevels,
  //                    UErrorCode *pErrorCode);
  //  const UBiDiLevel *bidi_getLevels(UBiDi *content, UErrorCode *pErrorCode);
  //  void bidi_getVisualMap(UBiDi *pBiDi, int32_t *indexMap,
  //                         UErrorCode *pErrorCode);
  //  void bidi_getLogicalMap(UBiDi *pBiDi, int32_t *indexMap,
  //                          UErrorCode *pErrorCode);
  //  // Breaker
  //  UBreakIterator *brk_open(UBreakIteratorType type, const char *locale,
  //                           const UChar *text, int32_t textLength,
  //                           UErrorCode *status);
  //  void brk_setText(UBreakIterator *bi, const UChar *text, int32_t
  //  textLength,
  //                   UErrorCode *status);
  //  int32_t brk_first(UBreakIterator *bi);
  //  int32_t brk_next(UBreakIterator *bi);
  //  void brk_close(UBreakIterator *bi);
  //
  //  // breaker initialization
  //  void icu_breaker_init();
  //  // char breaker
  //  void icu_break_char(const UChar *text, uint32_t textLength, uint8_t *);
  //  // word breaker
  //  void icu_break_word(const UChar *text, uint32_t textLength, uint8_t *);
  //  // line breaker
  //  void icu_break_line(const UChar *text, uint32_t textLength, uint8_t *);

 private:
  UBreakIterator* char_brk_iter_ = nullptr;
  UBreakIterator* word_brk_iter_ = nullptr;
  UBreakIterator* line_brk_iter_ = nullptr;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_ICU_WRAPPER_H_
