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

#ifndef SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_U_CHARACTER_H
#define SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_U_CHARACTER_H
#include "bidi_brackets.h"
#include "bidi_character.h"
#include "bidi_mirror.h"
#include "ucharacter_enums.h"
class UCharacter {
 public:
  /* Predefined Directional Formatting Characters */
  /**
   * Unicode "Left-To-Right Embedding" (LRE) Character::
   */
  static const int LRE = 0x202A;
  /**
   * Unicode "Right-To-Left Embedding" (RLE) Character::
   */
  static const int RLE = 0x202B;
  /**
   * Unicode "Left-To-Right Override" (LRO) Character::
   */
  static const int LRO = 0x202D;
  /**
   * Unicode "Right-To-Left Override" (RLO) Character::
   */
  static const int RLO = 0x202E;
  /**
   * Unicode "Pop Directional Formatting" (PDF) Character::
   */
  static const int PDF = 0x202C;
  /**
   * Unicode "Left-To-Right Isolate" (LRI) Character::
   */
  static const int LRI = 0x2066;
  /**
   * Unicode "Right-To-Left Isolate" (RLI) Character::
   */
  static const int RLI = 0x2067;
  /**
   * Unicode "First Strong Isolate" (FSI) Character::
   */
  static const int FSI = 0x2068;
  /**
   * Unicode "Pop Directional Isolate" (PDI) Character::
   */
  static const int PDI = 0x2069;
  /**
   *  Unicode "Left-To-Right Mark" (LRM) Character::
   */
  static const int LRM = 0x200E;
  /*
   * Unicode "Right-To-Left Mark" (RLM) Character::
   */
  static const int RLM = 0x200F;

  enum BidiPairedBracketType : uint8_t {
    kNONE = 0,
    kOPEN = 1,
    kCLOSE = 2,
    kCOUNT = 3
  };

  static int getDirection(int ch) {
    // codes below is for test
    //      if(ch == 'A'){
    //        return UCharacterEnums::LEFT_TO_RIGHT;
    //      }else if(ch == 'B'){
    //        return UCharacterEnums::RIGHT_TO_LEFT;
    //      }else if(ch == 'C'){
    //        return UCharacterEnums::RIGHT_TO_LEFT_ARABIC;
    //      }else if(ch == 'D'){
    //        return UCharacterEnums::EUROPEAN_NUMBER;
    //      }else if(ch == 'E'){
    //        return UCharacterEnums::ARABIC_NUMBER;
    //      }else if(ch == 'F'){
    //        return UCharacterEnums::EUROPEAN_NUMBER_SEPARATOR;
    //      }else if(ch == 'G'){
    //        return UCharacterEnums::EUROPEAN_NUMBER_TERMINATOR;
    //      }else if(ch == 'H'){
    //        return UCharacterEnums::COMMON_NUMBER_SEPARATOR;
    //      }else if(ch == 'I'){
    //        return UCharacterEnums::DIR_NON_SPACING_MARK;
    //      }else if(ch == 'J'){
    //        return UCharacterEnums::BOUNDARY_NEUTRAL;
    //      }else if(ch == 'K'){
    //        return UCharacterEnums::BLOCK_SEPARATOR;
    //      }else if(ch == 'L'){
    //        return UCharacterEnums::SEGMENT_SEPARATOR;
    //      }else if(ch == 'M'){
    //        return UCharacterEnums::WHITE_SPACE_NEUTRAL;
    //      }else if(ch == 'N'){
    //        return UCharacterEnums::OTHER_NEUTRAL;
    //      }else if(ch == 'O'){
    //        return UCharacterEnums::LEFT_TO_RIGHT_EMBEDDING;
    //      }else if(ch == 'P'){
    //        return UCharacterEnums::RIGHT_TO_LEFT_EMBEDDING;
    //      }else if(ch == 'Q'){
    //        return UCharacterEnums::LEFT_TO_RIGHT_OVERRIDE;
    //      }else if(ch == 'R'){
    //        return UCharacterEnums::RIGHT_TO_LEFT_OVERRIDE;
    //      }else if(ch == 'S'){
    //        return UCharacterEnums::POP_DIRECTIONAL_FORMAT;
    //      }else if(ch == 'T'){
    //        return UCharacterEnums::LEFT_TO_RIGHT_ISOLATE;
    //      }else if(ch == 'U'){
    //        return UCharacterEnums::RIGHT_TO_LEFT_ISOLATE;
    //      }else if(ch == 'V'){
    //        return UCharacterEnums::FIRST_STRONG_ISOLATE;
    //      }else if(ch == 'W'){
    //        return UCharacterEnums::POP_DIRECTIONAL_ISOLATE;
    //      }
    int dir = Character::getDirectionality(ch);
    if (dir == Character::DIRECTIONALITY_LEFT_TO_RIGHT) {
      dir = UCharacterEnums::LEFT_TO_RIGHT;
    } else if (dir == Character::DIRECTIONALITY_RIGHT_TO_LEFT) {
      dir = UCharacterEnums::RIGHT_TO_LEFT;
    } else if (dir == Character::DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC) {
      dir = UCharacterEnums::RIGHT_TO_LEFT_ARABIC;
    } else if (dir == Character::DIRECTIONALITY_EUROPEAN_NUMBER) {
      dir = UCharacterEnums::EUROPEAN_NUMBER;
    } else if (dir == Character::DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR) {
      dir = UCharacterEnums::EUROPEAN_NUMBER_SEPARATOR;
    } else if (dir == Character::DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR) {
      dir = UCharacterEnums::EUROPEAN_NUMBER_TERMINATOR;
    } else if (dir == Character::DIRECTIONALITY_ARABIC_NUMBER) {
      dir = UCharacterEnums::ARABIC_NUMBER;
    } else if (dir == Character::DIRECTIONALITY_COMMON_NUMBER_SEPARATOR) {
      dir = UCharacterEnums::COMMON_NUMBER_SEPARATOR;
    } else if (dir == Character::DIRECTIONALITY_NONSPACING_MARK) {
      dir = UCharacterEnums::DIR_NON_SPACING_MARK;
    } else if (dir == Character::DIRECTIONALITY_BOUNDARY_NEUTRAL) {
      dir = UCharacterEnums::BOUNDARY_NEUTRAL;
    } else if (dir == Character::DIRECTIONALITY_PARAGRAPH_SEPARATOR) {
      dir = UCharacterEnums::BLOCK_SEPARATOR;
    } else if (dir == Character::DIRECTIONALITY_SEGMENT_SEPARATOR) {
      dir = UCharacterEnums::SEGMENT_SEPARATOR;
    } else if (dir == Character::DIRECTIONALITY_WHITESPACE) {
      dir = UCharacterEnums::WHITE_SPACE_NEUTRAL;
    } else if (dir == Character::DIRECTIONALITY_OTHER_NEUTRALS) {
      dir = UCharacterEnums::OTHER_NEUTRAL;
    } else if (dir == Character::DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING) {
      dir = UCharacterEnums::LEFT_TO_RIGHT_EMBEDDING;
    } else if (dir == Character::DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE) {
      dir = UCharacterEnums::LEFT_TO_RIGHT_OVERRIDE;
    } else if (dir == Character::DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING) {
      dir = UCharacterEnums::RIGHT_TO_LEFT_EMBEDDING;
    } else if (dir == Character::DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE) {
      dir = UCharacterEnums::RIGHT_TO_LEFT_OVERRIDE;
    } else if (dir == Character::DIRECTIONALITY_POP_DIRECTIONAL_FORMAT) {
      dir = UCharacterEnums::POP_DIRECTIONAL_FORMAT;
    } else {
      dir = UCharacterEnums::LEFT_TO_RIGHT;
    }

    return dir;
  }
  static char32_t getMirror(char32_t ch) { return BidiMirror::getMirror(ch); }
  static char32_t getBidiPairedBracket(char32_t c) {
    return BidiBrackets::getBidiPairedBracket(c);
  }

  static int getBidiPairedBracketType(char32_t c) {
    return BidiBrackets::getBidiPairedBracketType(c);
  }
};

#endif  // SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_U_CHARACTER_H
