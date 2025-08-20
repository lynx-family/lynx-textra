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

#include "bidi_line.h"

#include "bidi.h"
#include "bidi_run.h"
short BidiLine::getLevelAt(Bidi* bidi, int charIndex) {
  /* return paraLevel if in the trailing WS run, otherwise the real level */
  if (bidi->direction != Bidi::MIXED || charIndex >= bidi->trailingWSStart) {
    return bidi->GetParaLevelAt(charIndex);
  } else {
    return bidi->levels[charIndex];
  }
}

std::vector<short> BidiLine::getLevels(Bidi* bidi) {
  int start = bidi->trailingWSStart;
  int length = bidi->length;

  if (start != length) {
    /* the current levels array does not reflect the WS run */
    /*
     * After the previous if(), we know that the levels array
     * has an implicit trailing WS run and therefore does not fully
     * reflect itself all the levels.
     * This must be a Bidi object for a line, and
     * we need to create a new levels array.
     */
    /* bidi.paraLevel is ok even if contextual multiple paragraphs,
       since bidi is a line object                                     */
    // Arrays.fill(bidi.levels, start, length, bidi.paraLevel);
    for (int i = start; i < start + length; i++) {
      bidi->levels[i] = bidi->paraLevel;
    }

    /* this new levels array is set for the line and reflects the WS run */
    bidi->trailingWSStart = length;
  }
  if (length < bidi->levels.size()) {
    std::vector<short> levels = std::vector<short>(length);
    // System.arraycopy(bidi.levels, 0, levels, 0, length);
    for (int i = 0; i < length; i++) {
      levels[i] = bidi->levels[i];
    }
    return levels;
  }
  return bidi->levels;
}

BidiRun BidiLine::getLogicalRun(Bidi* bidi, int logicalPosition) {
  /* this is done based on runs rather than on levels since levels have
     a special interpretation when REORDER_RUNS_ONLY
   */
  BidiRun newRun = BidiRun(), iRun;
  getRuns(bidi);
  int runCount = bidi->runCount;
  int visualStart = 0, logicalLimit = 0;
  iRun = bidi->runs[0];

  for (int i = 0; i < runCount; i++) {
    iRun = bidi->runs[i];
    logicalLimit = iRun.start + iRun.limit - visualStart;
    if ((logicalPosition >= iRun.start) && (logicalPosition < logicalLimit)) {
      break;
    }
    visualStart = iRun.limit;
  }
  newRun.start = iRun.start;
  newRun.limit = logicalLimit;
  newRun.level = iRun.level;
  return newRun;
}

BidiRun BidiLine::getVisualRun(Bidi* bidi, int runIndex) {
  int start = bidi->runs[runIndex].start;
  int limit;
  short level = bidi->runs[runIndex].level;

  if (runIndex > 0) {
    limit = start + bidi->runs[runIndex].limit - bidi->runs[runIndex - 1].limit;
  } else {
    limit = start + bidi->runs[0].limit;
  }
  return BidiRun(start, limit, level);
}

void BidiLine::getSingleRun(Bidi* bidi, short level) {
  /* simple, single-run case */
  bidi->runs = bidi->simpleRuns;
  bidi->runCount = 1;

  /* fill and reorder the single run */
  bidi->runs[0] = BidiRun(0, bidi->length, level);
}

void BidiLine::reorderLine(Bidi* bidi, short minLevel, short maxLevel) {
  /* nothing to do? */
  if (maxLevel <= (minLevel | 1)) {
    return;
  }

  std::vector<BidiRun>& runs = bidi->runs;
  // BidiRun tempRun;
  std::vector<short>& levels = bidi->levels;
  int firstRun, endRun, limitRun, runCount;

  /*
   * Reorder only down to the lowest odd level
   * and reorder at an odd minLevel in a separate, simpler loop.
   * See comments above for why minLevel is always incremented.
   */
  ++minLevel;

  // runs = bidi->runs;
  // levels = bidi->levels;
  runCount = bidi->runCount;

  /* do not include the WS run at paraLevel<=old minLevel except in the simple
   * loop */
  if (bidi->trailingWSStart < bidi->length) {
    --runCount;
  }

  while (--maxLevel >= minLevel) {
    firstRun = 0;

    /* loop for all sequences of runs */
    for (;;) {
      /* look for a sequence of runs that are all at >=maxLevel */
      /* look for the first run of such a sequence */
      while (firstRun < runCount && levels[runs[firstRun].start] < maxLevel) {
        ++firstRun;
      }
      if (firstRun >= runCount) {
        break; /* no more such runs */
      }

      /* look for the limit run of such a sequence (the run behind it) */
      for (limitRun = firstRun;
           ++limitRun < runCount && levels[runs[limitRun].start] >= maxLevel;) {
      }

      /* Swap the entire sequence of runs from firstRun to limitRun-1. */
      endRun = limitRun - 1;
      while (firstRun < endRun) {
        BidiRun tempRun = runs[firstRun];
        runs[firstRun] = runs[endRun];
        runs[endRun] = tempRun;
        ++firstRun;
        --endRun;
      }

      if (limitRun == runCount) {
        break; /* no more such runs */
      } else {
        firstRun = limitRun + 1;
      }
    }
  }

  /* now do maxLevel==old minLevel (==odd!), see above */
  if ((minLevel & 1) == 0) {
    firstRun = 0;

    /* include the trailing WS run in this complete reordering */
    if (bidi->trailingWSStart == bidi->length) {
      --runCount;
    }

    /* Swap the entire sequence of all runs. (endRun==runCount) */
    while (firstRun < runCount) {
      BidiRun tempRun = runs[firstRun];
      runs[firstRun] = runs[runCount];
      runs[runCount] = tempRun;
      ++firstRun;
      --runCount;
    }
  }
}

int BidiLine::getRunFromLogicalIndex(Bidi* bidi, int logicalIndex) {
  std::vector<BidiRun> runs = bidi->runs;
  int runCount = bidi->runCount, visualStart = 0, i, length, logicalStart;

  for (i = 0; i < runCount; i++) {
    length = runs[i].limit - visualStart;
    logicalStart = runs[i].start;
    if ((logicalIndex >= logicalStart) &&
        (logicalIndex < (logicalStart + length))) {
      return i;
    }
    visualStart += length;
  }
  /// CLOVER:OFF
  /* we should never get here */
  // throw new IllegalStateException("Internal ICU error in
  // getRunFromLogicalIndex");
  /// CLOVER:ON
  return -1;
}

void BidiLine::getRuns(Bidi* bidi) {
  /*
   * This method returns immediately if the runs are already set. This
   * includes the case of length==0 (handled in setPara)..
   */
  if (bidi->runCount >= 0) {
    return;
  }
  if (bidi->direction != Bidi::MIXED) {
    /* simple, single-run case - this covers length==0 */
    /* bidi.paraLevel is ok even for contextual multiple paragraphs */
    getSingleRun(bidi, bidi->paraLevel);
  } else /* Bidi.MIXED, length>0 */ {
    /* mixed directionality */
    int length = bidi->length, limit;
    std::vector<short> levels = bidi->levels;
    int i, runCount;
    short level = -1; /* initialize with no valid level */
    /*
     * If there are WS characters at the end of the line
     * and the run preceding them has a level different from
     * paraLevel, then they will form their own run at paraLevel (L1).
     * Count them separately.
     * We need some special treatment for this in order to not
     * modify the levels array which a line Bidi object shares
     * with its paragraph parent and its other line siblings.
     * In other words, for the trailing WS, it may be
     * levels[]!=paraLevel but we have to treat it like it were so.
     */
    limit = bidi->trailingWSStart;
    /* count the runs, there is at least one non-WS run, and limit>0 */
    runCount = 0;
    for (i = 0; i < limit; ++i) {
      /* increment runCount at the start of each run */
      if (levels[i] != level) {
        ++runCount;
        level = levels[i];
      }
    }

    /*
     * We don't need to see if the last run can be merged with a trailing
     * WS run because setTrailingWSStart() would have done that.
     */
    if (runCount == 1 && limit == length) {
      /* There is only one non-WS run and no trailing WS-run. */
      getSingleRun(bidi, levels[0]);
    } else /* runCount>1 || limit<length */ {
      /* allocate and set the runs */
      std::vector<BidiRun> runs;
      int runIndex, start;
      short minLevel = Bidi::MAX_EXPLICIT_LEVEL + 1;
      short maxLevel = 0;

      /* now, count a (non-mergeable) WS run */
      if (limit < length) {
        ++runCount;
      }

      /* runCount > 1 */
      //            bidi->getRunsMemory(runCount);
      //            runs = bidi->runsMemory;
      runs = std::vector<BidiRun>(runCount);

      /* set the runs */
      /* FOOD FOR THOUGHT: this could be optimized, e.g.:
       * 464->444, 484->444, 575->555, 595->555
       * However, that would take longer. Check also how it would
       * interact with BiDi control removal and inserting Marks.
       */
      runIndex = 0;

      /* search for the run limits and initialize visualLimit values with the
       * run lengths */
      i = 0;
      do {
        /* prepare this run */
        start = i;
        level = levels[i];
        if (level < minLevel) {
          minLevel = level;
        }
        if (level > maxLevel) {
          maxLevel = level;
        }

        /* look for the run limit */
        while (++i < limit && levels[i] == level) {
        }

        /* i is another run limit */
        runs[runIndex] = BidiRun(start, i - start, level);
        ++runIndex;
      } while (i < limit);

      if (limit < length) {
        /* there is a separate WS run */
        runs[runIndex] = BidiRun(limit, length - limit, bidi->paraLevel);
        /* For the trailing WS run, bidi.paraLevel is ok even
           if contextual multiple paragraphs.                   */
        if (bidi->paraLevel < minLevel) {
          minLevel = bidi->paraLevel;
        }
      }

      /* set the object fields */
      bidi->runs = runs;
      bidi->runCount = runCount;

      reorderLine(bidi, minLevel, maxLevel);

      /* now add the direction flags and adjust the visualLimit's to be just
       * that */
      /* this loop will also handle the trailing WS run */
      limit = 0;
      for (i = 0; i < runCount; ++i) {
        bidi->runs[i].level = levels[bidi->runs[i].start];
        limit = (bidi->runs[i].limit += limit);
      }

      /* Set the embedding level for the trailing WS run. */
      /* For a RTL paragraph, it will be the *first* run in visual order. */
      /* For the trailing WS run, bidi.paraLevel is ok even if
         contextual multiple paragraphs.                          */
      if (runIndex < runCount) {
        int trailingRun = ((bidi->paraLevel & 1) != 0) ? 0 : runIndex;
        bidi->runs[trailingRun].level = bidi->paraLevel;
      }
    }
  }

  /* handle insert LRM/RLM BEFORE/AFTER run */
  if (bidi->insertPoints.size > 0) {
    Bidi::Point point;
    int runIndex, ip;
    for (ip = 0; ip < bidi->insertPoints.size; ip++) {
      point = bidi->insertPoints.points[ip];
      runIndex = getRunFromLogicalIndex(bidi, point.pos);
      bidi->runs[runIndex].insertRemove |= point.flag;
    }
  }

  /* handle remove BiDi control characters */
  if (bidi->controlCount > 0) {
    int runIndex, ic;
    char32_t c;
    for (ic = 0; ic < bidi->length; ic++) {
      c = bidi->text_[ic];
      if (Bidi::IsBidiControlChar(c)) {
        runIndex = getRunFromLogicalIndex(bidi, ic);
        bidi->runs[runIndex].insertRemove--;
      }
    }
  }
}

std::vector<uint32_t> BidiLine::getLogicalMap(Bidi* bidi) {
  /* fill a logical-to-visual index map using the runs[] */
  std::vector<BidiRun> runs = bidi->runs;
  int logicalStart, visualStart, logicalLimit, visualLimit;
  std::vector<uint32_t> indexMap = std::vector<uint32_t>(bidi->length);
  if (bidi->length > bidi->resultLength) {
    // Arrays.fill(indexMap, Bidi::MAP_NOWHERE);
    for (int i = 0; i < indexMap.size(); i++) {
      indexMap[i] = Bidi::MAP_NOWHERE;
    }
  }

  visualStart = 0;
  for (int j = 0; j < bidi->runCount; ++j) {
    logicalStart = runs[j].start;
    // visualLimit = runs[j].start +
    // runs[j].limit;//TODO: There's an issue with the definition of limit in
    // runs
    visualLimit = runs[j].limit;
    if (runs[j].isEvenRun()) {
      do { /* LTR */
        indexMap[logicalStart++] = visualStart++;
      } while (visualStart < visualLimit);
    } else {
      logicalStart += visualLimit - visualStart; /* logicalLimit */
      do {                                       /* RTL */
        indexMap[--logicalStart] = visualStart++;
      } while (visualStart < visualLimit);
    }
    /* visualStart==visualLimit; */
  }

  if (bidi->insertPoints.size > 0) {
    int markFound = 0, runCount = bidi->runCount;
    int length, insertRemove, i, j;
    runs = bidi->runs;
    visualStart = 0;
    /* add number of marks found until each index */
    for (i = 0; i < runCount; i++, visualStart += length) {
      length = runs[i].limit - visualStart;
      insertRemove = runs[i].insertRemove;
      if ((insertRemove & (Bidi::LRM_BEFORE | Bidi::RLM_BEFORE)) > 0) {
        markFound++;
      }
      if (markFound > 0) {
        logicalStart = runs[i].start;
        logicalLimit = logicalStart + length;
        for (j = logicalStart; j < logicalLimit; j++) {
          indexMap[j] += markFound;
        }
      }
      if ((insertRemove & (Bidi::LRM_AFTER | Bidi::RLM_AFTER)) > 0) {
        markFound++;
      }
    }
  } else if (bidi->controlCount > 0) {
    int controlFound = 0, runCount = bidi->runCount;
    int length, insertRemove, i, j, k;
    bool evenRun;
    char32_t uchar;
    runs = bidi->runs;
    visualStart = 0;
    /* subtract number of controls found until each index */
    for (i = 0; i < runCount; i++, visualStart += length) {
      length = runs[i].limit - visualStart;
      insertRemove = runs[i].insertRemove;
      /* no control found within previous runs nor within this run */
      if ((controlFound - insertRemove) == 0) {
        continue;
      }
      logicalStart = runs[i].start;
      evenRun = runs[i].isEvenRun();
      logicalLimit = logicalStart + length;
      /* if no control within this run */
      if (insertRemove == 0) {
        for (j = logicalStart; j < logicalLimit; j++) {
          indexMap[j] -= controlFound;
        }
        continue;
      }
      for (j = 0; j < length; j++) {
        k = evenRun ? logicalStart + j : logicalLimit - j - 1;
        uchar = bidi->text_[k];
        if (Bidi::IsBidiControlChar(uchar)) {
          controlFound++;
          indexMap[k] = Bidi::MAP_NOWHERE;
          continue;
        }
        indexMap[k] -= controlFound;
      }
    }
  }
  return indexMap;
}

std::vector<uint32_t> BidiLine::getVisualMap(Bidi* bidi) {
  /* fill a visual-to-logical index map using the runs[] */
  std::vector<BidiRun> runs = bidi->runs;
  int logicalStart, visualStart, visualLimit;
  int allocLength =
      bidi->length > bidi->resultLength ? bidi->length : bidi->resultLength;
  std::vector<uint32_t> indexMap = std::vector<uint32_t>(allocLength);

  visualStart = 0;
  int idx = 0;
  for (int j = 0; j < bidi->runCount; ++j) {
    logicalStart = runs[j].start;
    // visualLimit = runs[j].start + runs[j].limit;
    visualLimit = runs[j].limit;
    if (runs[j].isEvenRun()) {
      do { /* LTR */
        indexMap[idx++] = logicalStart++;
      } while (++visualStart < visualLimit);
    } else {
      logicalStart += visualLimit - visualStart; /* logicalLimit */
      do {                                       /* RTL */
        indexMap[idx++] = --logicalStart;
      } while (++visualStart < visualLimit);
    }
    /* visualStart==visualLimit; */
  }

  if (bidi->insertPoints.size > 0) {
    int markFound = 0, runCount = bidi->runCount;
    int insertRemove, i, j, k;
    runs = bidi->runs;
    /* count all inserted marks */
    for (i = 0; i < runCount; i++) {
      insertRemove = runs[i].insertRemove;
      if ((insertRemove & (Bidi::LRM_BEFORE | Bidi::RLM_BEFORE)) > 0) {
        markFound++;
      }
      if ((insertRemove & (Bidi::LRM_AFTER | Bidi::RLM_AFTER)) > 0) {
        markFound++;
      }
    }
    /* move back indexes by number of preceding marks */
    k = bidi->resultLength;
    for (i = runCount - 1; i >= 0 && markFound > 0; i--) {
      insertRemove = runs[i].insertRemove;
      if ((insertRemove & (Bidi::LRM_AFTER | Bidi::RLM_AFTER)) > 0) {
        indexMap[--k] = Bidi::MAP_NOWHERE;
        markFound--;
      }
      visualStart = i > 0 ? runs[i - 1].limit : 0;
      for (j = runs[i].limit - 1; j >= visualStart && markFound > 0; j--) {
        indexMap[--k] = indexMap[j];
      }
      if ((insertRemove & (Bidi::LRM_BEFORE | Bidi::RLM_BEFORE)) > 0) {
        indexMap[--k] = Bidi::MAP_NOWHERE;
        markFound--;
      }
    }
  } else if (bidi->controlCount > 0) {
    int runCount = bidi->runCount, logicalEnd;
    int insertRemove, length, i, j, k, m;
    char32_t uchar;
    bool evenRun;
    runs = bidi->runs;
    visualStart = 0;
    /* move forward indexes by number of preceding controls */
    k = 0;
    for (i = 0; i < runCount; i++, visualStart += length) {
      length = runs[i].limit - visualStart;
      insertRemove = runs[i].insertRemove;
      /* if no control found yet, nothing to do in this run */
      if ((insertRemove == 0) && (k == visualStart)) {
        k += length;
        continue;
      }
      /* if no control in this run */
      if (insertRemove == 0) {
        visualLimit = runs[i].limit;
        for (j = visualStart; j < visualLimit; j++) {
          indexMap[k++] = indexMap[j];
        }
        continue;
      }
      logicalStart = runs[i].start;
      evenRun = runs[i].isEvenRun();
      logicalEnd = logicalStart + length - 1;
      for (j = 0; j < length; j++) {
        m = evenRun ? logicalStart + j : logicalEnd - j;
        uchar = bidi->text_[m];
        if (!Bidi::IsBidiControlChar(uchar)) {
          indexMap[k++] = m;
        }
      }
    }
  }
  if (allocLength == bidi->resultLength) {
    return indexMap;
  }
  std::vector<uint32_t> newMap = std::vector<uint32_t>(bidi->resultLength);
  // System.arraycopy(indexMap, 0, newMap, 0, bidi.resultLength);
  for (int i = 0; i < bidi->resultLength; i++) {
    newMap[i] = indexMap[i];
  }
  return newMap;
}
