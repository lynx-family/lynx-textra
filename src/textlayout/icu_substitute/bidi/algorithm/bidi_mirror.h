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

#include <unordered_map>

#ifndef SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_MIRROR_H_
#define SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_MIRROR_H_
class BidiMirror {
  typedef std::unordered_map<int, int> DtMap;

 public:
  BidiMirror();
  static int getMirror(char32_t c);

 private:
  static DtMap mData;
  static DtMap Create_mData();
};
#endif  // SRC_TEXTLAYOUT_ICU_SUBSTITUTE_BIDI_ALGORITHM_BIDI_MIRROR_H_
