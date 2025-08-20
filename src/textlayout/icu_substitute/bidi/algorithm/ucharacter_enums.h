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

#ifndef BIDI_UCHARACTERENUMS_H
#define BIDI_UCHARACTERENUMS_H

class UCharacterEnums {
 public:
  UCharacterEnums(){};

  ~UCharacterEnums(){};

  // ECharacterCategory

  static const short UNASSIGNED = 0;
  static const short GENERAL_OTHER_TYPES = 0;
  static const short UPPERCASE_LETTER = 1;
  static const short LOWERCASE_LETTER = 2;
  static const short TITLECASE_LETTER = 3;
  static const short MODIFIER_LETTER = 4;
  static const short OTHER_LETTER = 5;
  static const short NON_SPACING_MARK = 6;
  static const short ENCLOSING_MARK = 7;
  static const short COMBINING_SPACING_MARK = 8;
  static const short DECIMAL_DIGIT_NUMBER = 9;
  static const short LETTER_NUMBER = 10;
  static const short OTHER_NUMBER = 11;
  static const short SPACE_SEPARATOR = 12;
  static const short LINE_SEPARATOR = 13;
  static const short PARAGRAPH_SEPARATOR = 14;
  static const short CONTROL = 15;
  static const short FORMAT = 16;
  static const short PRIVATE_USE = 17;
  static const short SURROGATE = 18;
  static const short DASH_PUNCTUATION = 19;
  static const short START_PUNCTUATION = 20;
  static const short END_PUNCTUATION = 21;
  static const short CONNECTOR_PUNCTUATION = 22;
  static const short OTHER_PUNCTUATION = 23;
  static const short MATH_SYMBOL = 24;
  static const short CURRENCY_SYMBOL = 25;
  static const short MODIFIER_SYMBOL = 26;
  static const short OTHER_SYMBOL = 27;
  static const short INITIAL_PUNCTUATION = 28;
  static const short INITIAL_QUOTE_PUNCTUATION = 28;
  static const short FINAL_PUNCTUATION = 29;
  static const short FINAL_QUOTE_PUNCTUATION = 29;
  static const short CHAR_CATEGORY_COUNT = 30;

  // ECharacterDirection

  static const int LEFT_TO_RIGHT = 0;
  static const short DIRECTIONALITY_LEFT_TO_RIGHT = (short)LEFT_TO_RIGHT;
  static const int RIGHT_TO_LEFT = 1;
  static const short DIRECTIONALITY_RIGHT_TO_LEFT = (short)RIGHT_TO_LEFT;
  static const int EUROPEAN_NUMBER = 2;
  static const short DIRECTIONALITY_EUROPEAN_NUMBER = (short)EUROPEAN_NUMBER;
  static const int EUROPEAN_NUMBER_SEPARATOR = 3;
  static const short DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR =
      (short)EUROPEAN_NUMBER_SEPARATOR;
  static const int EUROPEAN_NUMBER_TERMINATOR = 4;
  static const short DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR =
      (short)EUROPEAN_NUMBER_TERMINATOR;
  static const int ARABIC_NUMBER = 5;
  static const short DIRECTIONALITY_ARABIC_NUMBER = (short)ARABIC_NUMBER;
  static const int COMMON_NUMBER_SEPARATOR = 6;
  static const short DIRECTIONALITY_COMMON_NUMBER_SEPARATOR =
      (short)COMMON_NUMBER_SEPARATOR;

  static const int BLOCK_SEPARATOR = 7;

  static const short DIRECTIONALITY_PARAGRAPH_SEPARATOR =
      (short)BLOCK_SEPARATOR;

  static const int SEGMENT_SEPARATOR = 8;

  static const short DIRECTIONALITY_SEGMENT_SEPARATOR =
      (short)SEGMENT_SEPARATOR;

  static const int WHITE_SPACE_NEUTRAL = 9;

  static const short DIRECTIONALITY_WHITESPACE = (short)WHITE_SPACE_NEUTRAL;

  static const int OTHER_NEUTRAL = 10;

  static const short DIRECTIONALITY_OTHER_NEUTRALS = (short)OTHER_NEUTRAL;

  static const int LEFT_TO_RIGHT_EMBEDDING = 11;

  static const short DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING =
      (short)LEFT_TO_RIGHT_EMBEDDING;

  static const int LEFT_TO_RIGHT_OVERRIDE = 12;
  static const short DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE =
      (short)LEFT_TO_RIGHT_OVERRIDE;
  static const int RIGHT_TO_LEFT_ARABIC = 13;

  static const short DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC =
      (short)RIGHT_TO_LEFT_ARABIC;

  static const int RIGHT_TO_LEFT_EMBEDDING = 14;

  static const short DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING =
      (short)RIGHT_TO_LEFT_EMBEDDING;

  static const int RIGHT_TO_LEFT_OVERRIDE = 15;

  static const short DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE =
      (short)RIGHT_TO_LEFT_OVERRIDE;

  static const int POP_DIRECTIONAL_FORMAT = 16;

  static const short DIRECTIONALITY_POP_DIRECTIONAL_FORMAT =
      (short)POP_DIRECTIONAL_FORMAT;

  static const int DIR_NON_SPACING_MARK = 17;
  static const short DIRECTIONALITY_NONSPACING_MARK =
      (short)DIR_NON_SPACING_MARK;
  static const int BOUNDARY_NEUTRAL = 18;
  static const short DIRECTIONALITY_BOUNDARY_NEUTRAL = (short)BOUNDARY_NEUTRAL;
  static const short FIRST_STRONG_ISOLATE = 19;
  static const short LEFT_TO_RIGHT_ISOLATE = 20;
  static const short RIGHT_TO_LEFT_ISOLATE = 21;
  static const short POP_DIRECTIONAL_ISOLATE = 22;
  static const int CHAR_DIRECTION_COUNT = 23;
  static const short DIRECTIONALITY_UNDEFINED = -1;
};

#endif  // BIDI_UCHARACTERENUMS_H
