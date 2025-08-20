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

#ifndef BIDI_BIDIMIRROR_H
#define BIDI_BIDIMIRROR_H
class BidiMirror {
  typedef std::unordered_map<int, int> DtMap;

 public:
  BidiMirror();
  static int getMirror(char32_t c);

 private:
  static DtMap mData;
  static DtMap Create_mData();
};
#endif  // BIDI_BIDIMIRROR_H
