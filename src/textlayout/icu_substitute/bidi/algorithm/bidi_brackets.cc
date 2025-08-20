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

#include "bidi_brackets.h"

BidiBrackets::DtMap BidiBrackets::Create_mData() {
  BidiBrackets::DtMap data;
  data.insert(std::make_pair(
      0x0028, BidiBrackets::BracketsData(
                  0x0029, BidiBrackets::O)));  // LEFT PARENTHESIS
  data.insert(std::make_pair(
      0x0029, BidiBrackets::BracketsData(
                  0x0028, BidiBrackets::C)));  // RIGHT PARENTHESIS
  data.insert(std::make_pair(
      0x005B, BidiBrackets::BracketsData(
                  0x005D, BidiBrackets::O)));  // LEFT SQUARE BRACKET
  data.insert(std::make_pair(
      0x005D, BidiBrackets::BracketsData(
                  0x005B, BidiBrackets::C)));  // RIGHT SQUARE BRACKET
  data.insert(std::make_pair(
      0x007B, BidiBrackets::BracketsData(
                  0x007D, BidiBrackets::O)));  // LEFT CURLY BRACKET
  data.insert(std::make_pair(
      0x007D, BidiBrackets::BracketsData(
                  0x007B, BidiBrackets::C)));  // RIGHT CURLY BRACKET
  data.insert(std::make_pair(
      0x0F3A,
      BidiBrackets::BracketsData(
          0x0F3B,
          BidiBrackets::O)));  // TIBETAN MARK GUG RTAGS GYBidiBracket::ON
  data.insert(std::make_pair(
      0x0F3B, BidiBrackets::BracketsData(
                  0x0F3A, BidiBrackets::C)));  // TIBETAN MARK GUG RTAGS GYAS
  data.insert(std::make_pair(
      0x0F3C,
      BidiBrackets::BracketsData(
          0x0F3D,
          BidiBrackets::O)));  // TIBETAN MARK ANG KHANG GYBidiBracket::ON
  data.insert(std::make_pair(
      0x0F3D, BidiBrackets::BracketsData(
                  0x0F3C, BidiBrackets::C)));  // TIBETAN MARK ANG KHANG GYAS
  data.insert(std::make_pair(
      0x169B,
      BidiBrackets::BracketsData(
          0x169C, BidiBrackets::O)));  // BidiBracket::OGHAM FEATHER MARK
  data.insert(std::make_pair(
      0x169C,
      BidiBrackets::BracketsData(
          0x169B,
          BidiBrackets::C)));  // BidiBracket::OGHAM REVERSED FEATHER MARK
  data.insert(std::make_pair(
      0x2045, BidiBrackets::BracketsData(
                  0x2046, BidiBrackets::O)));  // LEFT SQUARE BRACKET WITH QUILL
  data.insert(std::make_pair(
      0x2046,
      BidiBrackets::BracketsData(
          0x2045, BidiBrackets::C)));  // RIGHT SQUARE BRACKET WITH QUILL
  data.insert(std::make_pair(
      0x207D, BidiBrackets::BracketsData(
                  0x207E, BidiBrackets::O)));  // SUPERSCRIPT LEFT PARENTHESIS
  data.insert(std::make_pair(
      0x207E, BidiBrackets::BracketsData(
                  0x207D, BidiBrackets::C)));  // SUPERSCRIPT RIGHT PARENTHESIS
  data.insert(std::make_pair(
      0x208D, BidiBrackets::BracketsData(
                  0x208E, BidiBrackets::O)));  // SUBSCRIPT LEFT PARENTHESIS
  data.insert(std::make_pair(
      0x208E, BidiBrackets::BracketsData(
                  0x208D, BidiBrackets::C)));  // SUBSCRIPT RIGHT PARENTHESIS
  data.insert(std::make_pair(
      0x2308,
      BidiBrackets::BracketsData(0x2309, BidiBrackets::O)));  // LEFT CEILING
  data.insert(std::make_pair(
      0x2309,
      BidiBrackets::BracketsData(0x2308, BidiBrackets::C)));  // RIGHT CEILING
  data.insert(std::make_pair(
      0x230A, BidiBrackets::BracketsData(
                  0x230B,
                  BidiBrackets::O)));  // LEFT FLBidiBracket::OBidiBracket::OR
  data.insert(std::make_pair(
      0x230B, BidiBrackets::BracketsData(
                  0x230A,
                  BidiBrackets::C)));  // RIGHT FLBidiBracket::OBidiBracket::OR
  data.insert(std::make_pair(
      0x2329,
      BidiBrackets::BracketsData(
          0x232A,
          BidiBrackets::O)));  // LEFT-PBidiBracket::OINTING ANGLE BRACKET
  data.insert(std::make_pair(
      0x232A,
      BidiBrackets::BracketsData(
          0x2329,
          BidiBrackets::C)));  // RIGHT-PBidiBracket::OINTING ANGLE BRACKET
  data.insert(std::make_pair(
      0x2768,
      BidiBrackets::BracketsData(
          0x2769,
          BidiBrackets::O)));  // MEDIUM LEFT PARENTHESIS BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2769,
      BidiBrackets::BracketsData(
          0x2768,
          BidiBrackets::C)));  // MEDIUM RIGHT PARENTHESIS BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276A,
      BidiBrackets::BracketsData(
          0x276B, BidiBrackets::O)));  // MEDIUM FLATTENED LEFT PARENTHESIS
                                       // BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276B,
      BidiBrackets::BracketsData(
          0x276A, BidiBrackets::C)));  // MEDIUM FLATTENED RIGHT PARENTHESIS
                                       // BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276C,
      BidiBrackets::BracketsData(
          0x276D, BidiBrackets::O)));  // MEDIUM LEFT-PBidiBracket::OINTING
                                       // ANGLE BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276D,
      BidiBrackets::BracketsData(
          0x276C, BidiBrackets::C)));  // MEDIUM RIGHT-PBidiBracket::OINTING
                                       // ANGLE BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276E, BidiBrackets::BracketsData(
                  0x276F,
                  BidiBrackets::O)));  // HEAVY LEFT-PBidiBracket::OINTING ANGLE
                                       // QUBidiBracket::OTATIBidiBracket::ON
                                       // MARK BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x276F,
      BidiBrackets::BracketsData(
          0x276E,
          BidiBrackets::C)));  // HEAVY RIGHT-PBidiBracket::OINTING ANGLE
                               // QUBidiBracket::OTATIBidiBracket::ON MARK
                               // BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2770,
      BidiBrackets::BracketsData(
          0x2771, BidiBrackets::O)));  // HEAVY LEFT-PBidiBracket::OINTING ANGLE
                                       // BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2771,
      BidiBrackets::BracketsData(
          0x2770, BidiBrackets::C)));  // HEAVY RIGHT-PBidiBracket::OINTING
                                       // ANGLE BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2772,
      BidiBrackets::BracketsData(
          0x2773,
          BidiBrackets::O)));  // LIGHT LEFT TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2773,
      BidiBrackets::BracketsData(
          0x2772,
          BidiBrackets::C)));  // LIGHT RIGHT TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2774, BidiBrackets::BracketsData(
                  0x2775, BidiBrackets::O)));  // MEDIUM LEFT CURLY BRACKET
                                               // BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x2775, BidiBrackets::BracketsData(
                  0x2774, BidiBrackets::C)));  // MEDIUM RIGHT CURLY BRACKET
                                               // BidiBracket::ORNAMENT
  data.insert(std::make_pair(
      0x27C5, BidiBrackets::BracketsData(
                  0x27C6, BidiBrackets::O)));  // LEFT S-SHAPED BAG DELIMITER
  data.insert(std::make_pair(
      0x27C6, BidiBrackets::BracketsData(
                  0x27C5, BidiBrackets::C)));  // RIGHT S-SHAPED BAG DELIMITER
  data.insert(std::make_pair(
      0x27E6, BidiBrackets::BracketsData(
                  0x27E7,
                  BidiBrackets::O)));  // MATHEMATICAL LEFT WHITE SQUARE BRACKET
  data.insert(std::make_pair(
      0x27E7,
      BidiBrackets::BracketsData(
          0x27E6,
          BidiBrackets::C)));  // MATHEMATICAL RIGHT WHITE SQUARE BRACKET
  data.insert(std::make_pair(
      0x27E8,
      BidiBrackets::BracketsData(
          0x27E9, BidiBrackets::O)));  // MATHEMATICAL LEFT ANGLE BRACKET
  data.insert(std::make_pair(
      0x27E9,
      BidiBrackets::BracketsData(
          0x27E8, BidiBrackets::C)));  // MATHEMATICAL RIGHT ANGLE BRACKET
  data.insert(std::make_pair(
      0x27EA,
      BidiBrackets::BracketsData(
          0x27EB, BidiBrackets::O)));  // MATHEMATICAL LEFT DBidiBracket::OUBLE
                                       // ANGLE BRACKET
  data.insert(std::make_pair(
      0x27EB,
      BidiBrackets::BracketsData(
          0x27EA, BidiBrackets::C)));  // MATHEMATICAL RIGHT DBidiBracket::OUBLE
                                       // ANGLE BRACKET
  data.insert(std::make_pair(
      0x27EC,
      BidiBrackets::BracketsData(
          0x27ED, BidiBrackets::O)));  // MATHEMATICAL LEFT WHITE
                                       // TBidiBracket::ORTBidiBracket::OISE
                                       // SHELL BRACKET
  data.insert(std::make_pair(
      0x27ED,
      BidiBrackets::BracketsData(
          0x27EC, BidiBrackets::C)));  // MATHEMATICAL RIGHT WHITE
                                       // TBidiBracket::ORTBidiBracket::OISE
                                       // SHELL BRACKET
  data.insert(std::make_pair(
      0x27EE,
      BidiBrackets::BracketsData(
          0x27EF,
          BidiBrackets::O)));  // MATHEMATICAL LEFT FLATTENED PARENTHESIS
  data.insert(std::make_pair(
      0x27EF,
      BidiBrackets::BracketsData(
          0x27EE,
          BidiBrackets::C)));  // MATHEMATICAL RIGHT FLATTENED PARENTHESIS
  data.insert(std::make_pair(
      0x2983, BidiBrackets::BracketsData(
                  0x2984, BidiBrackets::O)));  // LEFT WHITE CURLY BRACKET
  data.insert(std::make_pair(
      0x2984, BidiBrackets::BracketsData(
                  0x2983, BidiBrackets::C)));  // RIGHT WHITE CURLY BRACKET
  data.insert(std::make_pair(
      0x2985, BidiBrackets::BracketsData(
                  0x2986, BidiBrackets::O)));  // LEFT WHITE PARENTHESIS
  data.insert(std::make_pair(
      0x2986, BidiBrackets::BracketsData(
                  0x2985, BidiBrackets::C)));  // RIGHT WHITE PARENTHESIS
  data.insert(std::make_pair(
      0x2987, BidiBrackets::BracketsData(
                  0x2988,
                  BidiBrackets::O)));  // Z NBidiBracket::OTATIBidiBracket::ON
                                       // LEFT IMAGE BRACKET
  data.insert(std::make_pair(
      0x2988, BidiBrackets::BracketsData(
                  0x2987,
                  BidiBrackets::C)));  // Z NBidiBracket::OTATIBidiBracket::ON
                                       // RIGHT IMAGE BRACKET
  data.insert(std::make_pair(
      0x2989, BidiBrackets::BracketsData(
                  0x298A,
                  BidiBrackets::O)));  // Z NBidiBracket::OTATIBidiBracket::ON
                                       // LEFT BINDING BRACKET
  data.insert(std::make_pair(
      0x298A, BidiBrackets::BracketsData(
                  0x2989,
                  BidiBrackets::C)));  // Z NBidiBracket::OTATIBidiBracket::ON
                                       // RIGHT BINDING BRACKET
  data.insert(std::make_pair(
      0x298B,
      BidiBrackets::BracketsData(
          0x298C, BidiBrackets::O)));  // LEFT SQUARE BRACKET WITH UNDERBAR
  data.insert(std::make_pair(
      0x298C,
      BidiBrackets::BracketsData(
          0x298B, BidiBrackets::C)));  // RIGHT SQUARE BRACKET WITH UNDERBAR
  data.insert(std::make_pair(
      0x298D, BidiBrackets::BracketsData(
                  0x2990,
                  BidiBrackets::O)));  // LEFT SQUARE BRACKET WITH TICK IN
                                       // TBidiBracket::OP CBidiBracket::ORNER
  data.insert(std::make_pair(
      0x298E,
      BidiBrackets::BracketsData(
          0x298F, BidiBrackets::C)));  // RIGHT SQUARE BRACKET WITH TICK IN
                                       // BBidiBracket::OTTBidiBracket::OM
                                       // CBidiBracket::ORNER
  data.insert(std::make_pair(
      0x298F,
      BidiBrackets::BracketsData(
          0x298E, BidiBrackets::O)));  // LEFT SQUARE BRACKET WITH TICK IN
                                       // BBidiBracket::OTTBidiBracket::OM
                                       // CBidiBracket::ORNER
  data.insert(std::make_pair(
      0x2990, BidiBrackets::BracketsData(
                  0x298D,
                  BidiBrackets::C)));  // RIGHT SQUARE BRACKET WITH TICK IN
                                       // TBidiBracket::OP CBidiBracket::ORNER
  data.insert(std::make_pair(
      0x2991,
      BidiBrackets::BracketsData(
          0x2992,
          BidiBrackets::O)));  // LEFT ANGLE BRACKET WITH DBidiBracket::OT
  data.insert(std::make_pair(
      0x2992,
      BidiBrackets::BracketsData(
          0x2991,
          BidiBrackets::C)));  // RIGHT ANGLE BRACKET WITH DBidiBracket::OT
  data.insert(std::make_pair(
      0x2993, BidiBrackets::BracketsData(
                  0x2994, BidiBrackets::O)));  // LEFT ARC LESS-THAN BRACKET
  data.insert(std::make_pair(
      0x2994, BidiBrackets::BracketsData(
                  0x2993, BidiBrackets::C)));  // RIGHT ARC GREATER-THAN BRACKET
  data.insert(std::make_pair(
      0x2995, BidiBrackets::BracketsData(
                  0x2996, BidiBrackets::O)));  // DBidiBracket::OUBLE LEFT ARC
                                               // GREATER-THAN BRACKET
  data.insert(std::make_pair(
      0x2996, BidiBrackets::BracketsData(
                  0x2995, BidiBrackets::C)));  // DBidiBracket::OUBLE RIGHT ARC
                                               // LESS-THAN BRACKET
  data.insert(std::make_pair(
      0x2997,
      BidiBrackets::BracketsData(
          0x2998,
          BidiBrackets::O)));  // LEFT BLACK TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0x2998,
      BidiBrackets::BracketsData(
          0x2997,
          BidiBrackets::C)));  // RIGHT BLACK TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0x29D8, BidiBrackets::BracketsData(
                  0x29D9, BidiBrackets::O)));  // LEFT WIGGLY FENCE
  data.insert(std::make_pair(
      0x29D9, BidiBrackets::BracketsData(
                  0x29D8, BidiBrackets::C)));  // RIGHT WIGGLY FENCE
  data.insert(std::make_pair(
      0x29DA, BidiBrackets::BracketsData(
                  0x29DB,
                  BidiBrackets::O)));  // LEFT DBidiBracket::OUBLE WIGGLY FENCE
  data.insert(std::make_pair(
      0x29DB, BidiBrackets::BracketsData(
                  0x29DA,
                  BidiBrackets::C)));  // RIGHT DBidiBracket::OUBLE WIGGLY FENCE
  data.insert(std::make_pair(
      0x29FC, BidiBrackets::BracketsData(
                  0x29FD, BidiBrackets::O)));  // LEFT-PBidiBracket::OINTING
                                               // CURVED ANGLE BRACKET
  data.insert(std::make_pair(
      0x29FD, BidiBrackets::BracketsData(
                  0x29FC, BidiBrackets::C)));  // RIGHT-PBidiBracket::OINTING
                                               // CURVED ANGLE BRACKET
  data.insert(std::make_pair(
      0x2E22,
      BidiBrackets::BracketsData(
          0x2E23, BidiBrackets::O)));  // TBidiBracket::OP LEFT HALF BRACKET
  data.insert(std::make_pair(
      0x2E23,
      BidiBrackets::BracketsData(
          0x2E22, BidiBrackets::C)));  // TBidiBracket::OP RIGHT HALF BRACKET
  data.insert(std::make_pair(
      0x2E24,
      BidiBrackets::BracketsData(
          0x2E25, BidiBrackets::O)));  // BBidiBracket::OTTBidiBracket::OM
                                       // LEFT HALF BRACKET
  data.insert(std::make_pair(
      0x2E25,
      BidiBrackets::BracketsData(
          0x2E24, BidiBrackets::C)));  // BBidiBracket::OTTBidiBracket::OM
                                       // RIGHT HALF BRACKET
  data.insert(std::make_pair(
      0x2E26, BidiBrackets::BracketsData(
                  0x2E27, BidiBrackets::O)));  // LEFT SIDEWAYS U BRACKET
  data.insert(std::make_pair(
      0x2E27, BidiBrackets::BracketsData(
                  0x2E26, BidiBrackets::C)));  // RIGHT SIDEWAYS U BRACKET
  data.insert(std::make_pair(
      0x2E28, BidiBrackets::BracketsData(
                  0x2E29,
                  BidiBrackets::O)));  // LEFT DBidiBracket::OUBLE PARENTHESIS
  data.insert(std::make_pair(
      0x2E29, BidiBrackets::BracketsData(
                  0x2E28,
                  BidiBrackets::C)));  // RIGHT DBidiBracket::OUBLE PARENTHESIS
  data.insert(std::make_pair(
      0x3008, BidiBrackets::BracketsData(
                  0x3009, BidiBrackets::O)));  // LEFT ANGLE BRACKET
  data.insert(std::make_pair(
      0x3009, BidiBrackets::BracketsData(
                  0x3008, BidiBrackets::C)));  // RIGHT ANGLE BRACKET
  data.insert(std::make_pair(
      0x300A, BidiBrackets::BracketsData(
                  0x300B,
                  BidiBrackets::O)));  // LEFT DBidiBracket::OUBLE ANGLE BRACKET
  data.insert(std::make_pair(
      0x300B,
      BidiBrackets::BracketsData(
          0x300A,
          BidiBrackets::C)));  // RIGHT DBidiBracket::OUBLE ANGLE BRACKET
  data.insert(std::make_pair(
      0x300C,
      BidiBrackets::BracketsData(
          0x300D, BidiBrackets::O)));  // LEFT CBidiBracket::ORNER BRACKET
  data.insert(std::make_pair(
      0x300D,
      BidiBrackets::BracketsData(
          0x300C, BidiBrackets::C)));  // RIGHT CBidiBracket::ORNER BRACKET
  data.insert(std::make_pair(
      0x300E, BidiBrackets::BracketsData(
                  0x300F,
                  BidiBrackets::O)));  // LEFT WHITE CBidiBracket::ORNER BRACKET
  data.insert(std::make_pair(
      0x300F,
      BidiBrackets::BracketsData(
          0x300E,
          BidiBrackets::C)));  // RIGHT WHITE CBidiBracket::ORNER BRACKET
  data.insert(std::make_pair(
      0x3010, BidiBrackets::BracketsData(
                  0x3011, BidiBrackets::O)));  // LEFT BLACK LENTICULAR BRACKET
  data.insert(std::make_pair(
      0x3011, BidiBrackets::BracketsData(
                  0x3010, BidiBrackets::C)));  // RIGHT BLACK LENTICULAR BRACKET
  data.insert(std::make_pair(
      0x3014,
      BidiBrackets::BracketsData(
          0x3015,
          BidiBrackets::O)));  // LEFT TBidiBracket::ORTBidiBracket::OISE SHELL
                               // BRACKET
  data.insert(std::make_pair(
      0x3015,
      BidiBrackets::BracketsData(
          0x3014,
          BidiBrackets::C)));  // RIGHT TBidiBracket::ORTBidiBracket::OISE SHELL
                               // BRACKET
  data.insert(std::make_pair(
      0x3016, BidiBrackets::BracketsData(
                  0x3017, BidiBrackets::O)));  // LEFT WHITE LENTICULAR BRACKET
  data.insert(std::make_pair(
      0x3017, BidiBrackets::BracketsData(
                  0x3016, BidiBrackets::C)));  // RIGHT WHITE LENTICULAR BRACKET
  data.insert(std::make_pair(
      0x3018,
      BidiBrackets::BracketsData(
          0x3019,
          BidiBrackets::O)));  // LEFT WHITE TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0x3019,
      BidiBrackets::BracketsData(
          0x3018,
          BidiBrackets::C)));  // RIGHT WHITE TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0x301A, BidiBrackets::BracketsData(
                  0x301B, BidiBrackets::O)));  // LEFT WHITE SQUARE BRACKET
  data.insert(std::make_pair(
      0x301B, BidiBrackets::BracketsData(
                  0x301A, BidiBrackets::C)));  // RIGHT WHITE SQUARE BRACKET
  data.insert(std::make_pair(
      0xFE59, BidiBrackets::BracketsData(
                  0xFE5A, BidiBrackets::O)));  // SMALL LEFT PARENTHESIS
  data.insert(std::make_pair(
      0xFE5A, BidiBrackets::BracketsData(
                  0xFE59, BidiBrackets::C)));  // SMALL RIGHT PARENTHESIS
  data.insert(std::make_pair(
      0xFE5B, BidiBrackets::BracketsData(
                  0xFE5C, BidiBrackets::O)));  // SMALL LEFT CURLY BRACKET
  data.insert(std::make_pair(
      0xFE5C, BidiBrackets::BracketsData(
                  0xFE5B, BidiBrackets::C)));  // SMALL RIGHT CURLY BRACKET
  data.insert(std::make_pair(
      0xFE5D,
      BidiBrackets::BracketsData(
          0xFE5E,
          BidiBrackets::O)));  // SMALL LEFT TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0xFE5E,
      BidiBrackets::BracketsData(
          0xFE5D,
          BidiBrackets::C)));  // SMALL RIGHT TBidiBracket::ORTBidiBracket::OISE
                               // SHELL BRACKET
  data.insert(std::make_pair(
      0xFF08, BidiBrackets::BracketsData(
                  0xFF09, BidiBrackets::O)));  // FULLWIDTH LEFT PARENTHESIS
  data.insert(std::make_pair(
      0xFF09, BidiBrackets::BracketsData(
                  0xFF08, BidiBrackets::C)));  // FULLWIDTH RIGHT PARENTHESIS
  data.insert(std::make_pair(
      0xFF3B, BidiBrackets::BracketsData(
                  0xFF3D, BidiBrackets::O)));  // FULLWIDTH LEFT SQUARE BRACKET
  data.insert(std::make_pair(
      0xFF3D, BidiBrackets::BracketsData(
                  0xFF3B, BidiBrackets::C)));  // FULLWIDTH RIGHT SQUARE BRACKET
  data.insert(std::make_pair(
      0xFF5B, BidiBrackets::BracketsData(
                  0xFF5D, BidiBrackets::O)));  // FULLWIDTH LEFT CURLY BRACKET
  data.insert(std::make_pair(
      0xFF5D, BidiBrackets::BracketsData(
                  0xFF5B, BidiBrackets::C)));  // FULLWIDTH RIGHT CURLY BRACKET
  data.insert(std::make_pair(
      0xFF5F,
      BidiBrackets::BracketsData(
          0xFF60, BidiBrackets::O)));  // FULLWIDTH LEFT WHITE PARENTHESIS
  data.insert(std::make_pair(
      0xFF60,
      BidiBrackets::BracketsData(
          0xFF5F, BidiBrackets::C)));  // FULLWIDTH RIGHT WHITE PARENTHESIS
  data.insert(std::make_pair(
      0xFF62,
      BidiBrackets::BracketsData(
          0xFF63,
          BidiBrackets::O)));  // HALFWIDTH LEFT CBidiBracket::ORNER BRACKET
  data.insert(std::make_pair(
      0xFF63,
      BidiBrackets::BracketsData(
          0xFF62,
          BidiBrackets::C)));  // HALFWIDTH RIGHT CBidiBracket::ORNER BRACKET
  return data;
}

BidiBrackets::DtMap BidiBrackets::mData = BidiBrackets::Create_mData();
