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

#ifndef BIDI_BIDIRUN_H
#define BIDI_BIDIRUN_H

class BidiRun {
 public:
  int start;        /* first logical position of the run */
  int limit;        /* last visual position of the run +1 */
  int insertRemove; /* if >0, flags for inserting LRM/RLM before/after run,
                       if <0, count of bidi controls within run            */
  short level;

  BidiRun() { BidiRun(0, 0, 0); }

  /*
   * Constructor
   */
  BidiRun(int start_, int limit_, short embeddingLevel_) {
    start = start_;
    limit = limit_;
    level = embeddingLevel_;
  }

  void copyFrom(BidiRun run) {
    start = run.start;
    limit = run.limit;
    level = run.level;
    insertRemove = run.insertRemove;
  }

  int getStart() { return start; }

  int getLimit() { return limit; }

  int getLength() { return limit - start; }

  short getEmbeddingLevel() { return level; }

  bool isOddRun() { return (level & 1) == 1; }

  bool isEvenRun() { return (level & 1) == 0; }

  short getDirection() { return (level & 1); }
};

#endif  // BIDI_BIDIRUN_H
