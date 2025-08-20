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

#ifndef BIDI_BIDIBRACKETS_H
#define BIDI_BIDIBRACKETS_H
#include <unordered_map>
class BidiBrackets {
 public:
  struct BracketsData {
    char32_t paired;
    int type;
    BracketsData(char32_t c = 0, int t = 0) {
      paired = c;
      type = t;
    }
  };

  typedef std::unordered_map<char32_t, BracketsData> DtMap;

  static const int O = 1;  // BidiPairedBracketType.OPEN;
  static const int C = 2;  // BidiPairedBracketType.CLOSE;
  static const int N = 0;  // BidiPairedBracketType.NONE;

  static char32_t getBidiPairedBracket(char32_t c) {
    BracketsData bd = mData[c];
    return bd.paired;
  }

  static int getBidiPairedBracketType(char32_t c) {
    BracketsData bd = mData[c];
    return bd.type;
  }

  static DtMap mData;
  static DtMap Create_mData();
};
#endif  // BIDI_BIDIBRACKETS_H
