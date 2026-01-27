// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
**********************************************************************
*   Copyright (C) 1999-2015, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*/
// Modifications 2021 The Lynx Authors.
// Modifications licensed under the same terms as the original ICU license.

#ifndef SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_WRITER_H_
#define SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_WRITER_H_
#include <string>

#include "src/textlayout/icu_substitute/bidi/algorithm/ucharacter_enums.h"

class Bidi;
class BidiWriter {
 public:
  /** Bidi control code points */
  static const int LRM_CHAR = 0x200e;
  static const int RLM_CHAR = 0x200f;
  static const int MASK_R_AL = (1 << UCharacterEnums::RIGHT_TO_LEFT |
                                1 << UCharacterEnums::RIGHT_TO_LEFT_ARABIC);

  static bool IsCombining(int type) {
    return ((1 << type & (1 << UCharacterEnums::NON_SPACING_MARK |
                          1 << UCharacterEnums::COMBINING_SPACING_MARK |
                          1 << UCharacterEnums::ENCLOSING_MARK)) != 0);
  }

  static std::u32string doWriteForward(const std::u32string& src, int options);
  static std::u32string doWriteForward(const char32_t* text, int start,
                                       int limit, int options);

  static std::u32string writeReverse(const std::u32string& src, int options);
  static std::u32string doWriteReverse(const char32_t* text, int start,
                                       int limit, int options);

  static std::u32string writeReordered(Bidi* bidi, int options);
};
#endif  // SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_WRITER_H_
