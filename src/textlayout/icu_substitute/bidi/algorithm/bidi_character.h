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

#ifndef BIDI_CHARACTER_H
#define BIDI_CHARACTER_H

#include <vector>

class Character {
 public:
  static const int type_lst_size_ = 549;
  static const int direct_lst_size_ = 2047;
  static const uint32_t TypeLst[549];
  static const uint32_t DirectLst[2047];
  static const int MIN_HIGH_SURROGATE = 0xD800;  // char '\uD800'
  static const int MIN_LOW_SURROGATE = 0xDC00;
  static const int MAX_HIGH_SURROGATE = 0xDBFF;
  static const int MAX_LOW_SURROGATE = 0xDFFF;
  static const int MIN_SURROGATE = MIN_HIGH_SURROGATE;
  static const int MAX_SURROGATE = MAX_LOW_SURROGATE;
  static const int MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;

  /*
   * General character types
   */

  /**
   * General category "Cn" in the Unicode specification.
   * @since   1.1
   */
  static const short UNASSIGNED = 0;

  /**
   * General category "Lu" in the Unicode specification.
   * @since   1.1
   */
  static const short UPPERCASE_LETTER = 1;

  /**
   * General category "Ll" in the Unicode specification.
   * @since   1.1
   */
  static const short LOWERCASE_LETTER = 2;

  /**
   * General category "Lt" in the Unicode specification.
   * @since   1.1
   */
  static const short TITLECASE_LETTER = 3;

  /**
   * General category "Lm" in the Unicode specification.
   * @since   1.1
   */
  static const short MODIFIER_LETTER = 4;

  /**
   * General category "Lo" in the Unicode specification.
   * @since   1.1
   */
  static const short OTHER_LETTER = 5;

  /**
   * General category "Mn" in the Unicode specification.
   * @since   1.1
   */
  static const short NON_SPACING_MARK = 6;

  /**
   * General category "Me" in the Unicode specification.
   * @since   1.1
   */
  static const short ENCLOSING_MARK = 7;

  /**
   * General category "Mc" in the Unicode specification.
   * @since   1.1
   */
  static const short COMBINING_SPACING_MARK = 8;

  /**
   * General category "Nd" in the Unicode specification.
   * @since   1.1
   */
  static const short DECIMAL_DIGIT_NUMBER = 9;

  /**
   * General category "Nl" in the Unicode specification.
   * @since   1.1
   */
  static const short LETTER_NUMBER = 10;

  /**
   * General category "No" in the Unicode specification.
   * @since   1.1
   */
  static const short OTHER_NUMBER = 11;

  /**
   * General category "Zs" in the Unicode specification.
   * @since   1.1
   */
  static const short SPACE_SEPARATOR = 12;

  /**
   * General category "Zl" in the Unicode specification.
   * @since   1.1
   */
  static const short LINE_SEPARATOR = 13;

  /**
   * General category "Zp" in the Unicode specification.
   * @since   1.1
   */
  static const short PARAGRAPH_SEPARATOR = 14;

  /**
   * General category "Cc" in the Unicode specification.
   * @since   1.1
   */
  static const short CONTROL = 15;

  /**
   * General category "Cf" in the Unicode specification.
   * @since   1.1
   */
  static const short FORMAT = 16;

  /**
   * General category "Co" in the Unicode specification.
   * @since   1.1
   */
  static const short PRIVATE_USE = 18;

  /**
   * General category "Cs" in the Unicode specification.
   * @since   1.1
   */
  static const short SURROGATE = 19;

  /**
   * General category "Pd" in the Unicode specification.
   * @since   1.1
   */
  static const short DASH_PUNCTUATION = 20;

  /**
   * General category "Ps" in the Unicode specification.
   * @since   1.1
   */
  static const short START_PUNCTUATION = 21;

  /**
   * General category "Pe" in the Unicode specification.
   * @since   1.1
   */
  static const short END_PUNCTUATION = 22;

  /**
   * General category "Pc" in the Unicode specification.
   * @since   1.1
   */
  static const short CONNECTOR_PUNCTUATION = 23;

  /**
   * General category "Po" in the Unicode specification.
   * @since   1.1
   */
  static const short OTHER_PUNCTUATION = 24;

  /**
   * General category "Sm" in the Unicode specification.
   * @since   1.1
   */
  static const short MATH_SYMBOL = 25;

  /**
   * General category "Sc" in the Unicode specification.
   * @since   1.1
   */
  static const short CURRENCY_SYMBOL = 26;

  /**
   * General category "Sk" in the Unicode specification.
   * @since   1.1
   */
  static const short MODIFIER_SYMBOL = 27;

  /**
   * General category "So" in the Unicode specification.
   * @since   1.1
   */
  static const short OTHER_SYMBOL = 28;

  /**
   * General category "Pi" in the Unicode specification.
   * @since   1.4
   */
  static const short INITIAL_QUOTE_PUNCTUATION = 29;

  /**
   * General category "Pf" in the Unicode specification.
   * @since   1.4
   */
  static const short FINAL_QUOTE_PUNCTUATION = 30;

  /**
   * Error flag. Use int (code point) to avoid confusion with U+FFFF.
   */
  static const int ERROR = 0xFFFFFFFF;

  /**
   * Undefined bidirectional character type. Undefined {@code char}
   * values have undefined directionality in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_UNDEFINED = -1;

  /**
   * Strong bidirectional character type "L" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_LEFT_TO_RIGHT = 0;

  /**
   * Strong bidirectional character type "R" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_RIGHT_TO_LEFT = 1;

  /**
   * Strong bidirectional character type "AL" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC = 2;

  /**
   * Weak bidirectional character type "EN" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_EUROPEAN_NUMBER = 3;

  /**
   * Weak bidirectional character type "ES" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR = 4;

  /**
   * Weak bidirectional character type "ET" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = 5;

  /**
   * Weak bidirectional character type "AN" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_ARABIC_NUMBER = 6;

  /**
   * Weak bidirectional character type "CS" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_COMMON_NUMBER_SEPARATOR = 7;

  /**
   * Weak bidirectional character type "NSM" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_NONSPACING_MARK = 8;

  /**
   * Weak bidirectional character type "BN" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_BOUNDARY_NEUTRAL = 9;

  /**
   * Neutral bidirectional character type "B" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_PARAGRAPH_SEPARATOR = 10;

  /**
   * Neutral bidirectional character type "S" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_SEGMENT_SEPARATOR = 11;

  /**
   * Neutral bidirectional character type "WS" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_WHITESPACE = 12;

  /**
   * Neutral bidirectional character type "ON" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_OTHER_NEUTRALS = 13;

  /**
   * Strong bidirectional character type "LRE" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING = 14;

  /**
   * Strong bidirectional character type "LRO" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE = 15;

  /**
   * Strong bidirectional character type "RLE" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING = 16;

  /**
   * Strong bidirectional character type "RLO" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE = 17;

  /**
   * Weak bidirectional character type "PDF" in the Unicode specification.
   * @since 1.4
   */
  static const short DIRECTIONALITY_POP_DIRECTIONAL_FORMAT = 18;

  /**
   * The minimum value of a
   * <a href="http://www.unicode.org/glossary/#code_point">
   * Unicode code point</a>, constant {@code U+0000}.
   *
   * @since 1.5
   */
  static const int MIN_CODE_POINT = 0x000000;

  /**
   * The maximum value of a
   * <a href="http://www.unicode.org/glossary/#code_point">
   * Unicode code point</a>, constant {@code U+10FFFF}.
   *
   * @since 1.5
   */
  static const int MAX_CODE_POINT = 0X10FFFF;

  static bool isLowSurrogate(char32_t ch) {
    return ch >= MIN_LOW_SURROGATE && ch < (MAX_LOW_SURROGATE + 1);
  }

  static int toCodePoint(char32_t high, char32_t low) {
    // Optimized form of:
    // return ((high - MIN_HIGH_SURROGATE) << 10)
    //         + (low - MIN_LOW_SURROGATE)
    //         + MIN_SUPPLEMENTARY_CODE_POINT;
    return ((high << 10) + low) +
           (MIN_SUPPLEMENTARY_CODE_POINT - (MIN_HIGH_SURROGATE << 10) -
            MIN_LOW_SURROGATE);
  }

  static bool isHighSurrogate(char32_t ch) {
    // Help VM constant-fold; MAX_HIGH_SURROGATE + 1 == MIN_LOW_SURROGATE
    return ch >= MIN_HIGH_SURROGATE && ch < (MAX_HIGH_SURROGATE + 1);
  }

  static short getDirectionality(int codePoint);

  static int getType(char32_t c);

  static int charCount(int codePoint) {
    return codePoint >= MIN_SUPPLEMENTARY_CODE_POINT ? 2 : 1;
  }

  static int decode_codepoint(uint32_t num);
  static uint8_t decode_val(uint32_t num);
};

#endif  // BIDI_CHARACTER_H
