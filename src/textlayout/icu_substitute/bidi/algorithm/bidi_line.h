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

#ifndef BIDI_BIDILINE_H
#define BIDI_BIDILINE_H

#include <vector>

class Bidi;
class BidiRun;
class BidiLine {
 public:
  static short getLevelAt(Bidi* bidi, int charIndex);
  static std::vector<short> getLevels(Bidi* bidi);
  static BidiRun getLogicalRun(Bidi* bidi, int logicalPosition);
  static BidiRun getVisualRun(Bidi* bidi, int runIndex);
  static void getSingleRun(Bidi* bidi, short level);
  static void reorderLine(Bidi* bidi, short minLevel, short maxLevel);
  static int getRunFromLogicalIndex(Bidi* bidi, int logicalIndex);
  static void getRuns(Bidi* bidi);
  static std::vector<uint32_t> getLogicalMap(Bidi* bidi);
  static std::vector<uint32_t> getVisualMap(Bidi* bidi);
};

#endif  // BIDI_BIDILINE_H
