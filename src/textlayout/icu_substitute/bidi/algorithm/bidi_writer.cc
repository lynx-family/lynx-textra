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

#include "bidi_writer.h"

#include "bidi.h"
#include "bidi_run.h"
#include "u_character.h"

std::u32string BidiWriter::doWriteForward(const std::u32string& src,
                                          int options) {
  /* optimize for several combinations of options */
  switch (options & (Bidi::REMOVE_BIDI_CONTROLS | Bidi::DO_MIRRORING)) {
    case 0: {
      /* simply return the LTR run */
      return src;
    }
    case Bidi::DO_MIRRORING: {
      std::u32string dest;

      /* do mirroring */
      int i = 0;

      do {
        dest += UCharacter::getMirror(src[i]);
      } while (i < src.length());
      return dest;
    }
    case Bidi::REMOVE_BIDI_CONTROLS: {
      std::u32string dest;

      /* copy the LTR run and remove any Bidi control characters */
      int i = 0;
      char32_t c;
      do {
        c = src[i++];
        if (!Bidi::IsBidiControlChar(c)) {
          dest += c;
        }
      } while (i < src.length());
      return dest;
    }
    default: {
      std::u32string dest;

      /* remove Bidi control characters and do mirroring */
      int i = 0;
      do {
        auto c = src[i];
        if (!Bidi::IsBidiControlChar(c)) {
          dest += UCharacter::getMirror(c);
        }
      } while (i < src.length());
      return dest;
    }
  } /* end of switch */
}

std::u32string BidiWriter::doWriteForward(const char32_t* text, int start,
                                          int limit, int options) {
  std::u32string str = U"";
  for (int i = start; i < limit; i++) {
    str += text[i];
  }
  // return doWriteForward(new String(text, start, limit - start), options);
  return doWriteForward(str, options);
}

std::u32string BidiWriter::writeReverse(const std::u32string& src,
                                        int options) {
  /*
   * RTL run -
   *
   * RTL runs need to be copied to the destination in reverse order
   * of code points, not code units, to keep Unicode characters intact.
   *
   * The general strategy for this is to read the source text
   * in backward order, collect all code units for a code point
   * (and optionally following combining characters, see below),
   * and copy all these code units in ascending order
   * to the destination for this run.
   *
   * Several options request whether combining characters
   * should be kept after their base characters,
   * whether Bidi control characters should be removed, and
   * whether characters should be replaced by their mirror-image
   * equivalent Unicode characters.
   */
  std::u32string dest = U"";

  /* optimize for several combinations of options */
  switch (options & (Bidi::REMOVE_BIDI_CONTROLS | Bidi::DO_MIRRORING |
                     Bidi::KEEP_BASE_COMBINING)) {
    case 0: {
      /*
       * With none of the "complicated" options set, the destination
       * run will have the same length as the source run,
       * and there is no mirroring and no keeping combining characters
       * with their base characters.
       *
       * XXX: or dest = UTF16.reverse(new StringBuffer(src));
       */

      auto srcLength = src.length();

      /* preserve character integrity */
      do {
        /* copy this base character */
        dest += src[--srcLength];
      } while (srcLength > 0);
      break;
    }

    case Bidi::KEEP_BASE_COMBINING: {
      /*
       * Here, too, the destination
       * run will have the same length as the source run,
       * and there is no mirroring.
       * We do need to keep combining characters with their base
       * characters.
       */
      auto srcLength = src.length();

      /* preserve character integrity */
      do {
        /* i is always after the last code unit known to need to be kept
         *  in this segment */
        char32_t c = 0;

        /* collect code units and modifier letters for one base
         * character */
        do {
          c = src[--srcLength];
        } while (srcLength > 0 && IsCombining(Character::getType(c)));

        /* copy this "user character" */
        dest += src[srcLength];
      } while (srcLength > 0);
      break;
    }
    default: {
      /*
       * With several "complicated" options set, this is the most
       * general and the slowest copying of an RTL run.
       * We will do mirroring, remove Bidi controls, and
       * keep combining characters with their base characters
       * as requested.
       */
      auto srcLength = src.length();

      /* preserve character integrity */
      do {
        /* i is always after the last code unit known to need to be kept
         *  in this segment */

        /* collect code units for one base character */
        auto c = src[--srcLength];
        if ((options & Bidi::KEEP_BASE_COMBINING) != 0) {
          /* collect modifier letters for this base character */
          while (srcLength > 0 && IsCombining(Character::getType(c))) {
            c = src[--srcLength];
          }
        }

        if ((options & Bidi::REMOVE_BIDI_CONTROLS) != 0 &&
            Bidi::IsBidiControlChar(c)) {
          /* do not copy this Bidi control character */
          continue;
        }

        /* copy this "user character" */
        auto j = srcLength;
        if ((options & Bidi::DO_MIRRORING) != 0) {
          /* mirror only the base character */
          c = UCharacter::getMirror(c);
          dest += c;
          j++;
        }
        // dest.append(src.substring(j, i));
        dest += src[j];
      } while (srcLength > 0);
      break;
    }
  } /* end of switch */

  return dest;
}

std::u32string BidiWriter::doWriteReverse(const char32_t* text, int start,
                                          int limit, int options) {
  std::u32string str = U"";  // TODO: whether to add '\0' at the end of string
  for (int i = start; i < limit; i++) {
    str += text[i];
  }
  return writeReverse(str, options);
}

std::u32string BidiWriter::writeReordered(Bidi* bidi, int options) {
  int run, runCount;
  std::u32string dest;
  const auto& text = bidi->text_;
  runCount = bidi->countRuns();

  /*
   * Option "insert marks" implies Bidi.INSERT_LRM_FOR_NUMERIC if the
   * reordering mode (checked below) is appropriate.
   */
  if ((bidi->reorderingOptions & Bidi::OPTION_INSERT_MARKS) != 0) {
    options |= Bidi::INSERT_LRM_FOR_NUMERIC;
    options &= ~Bidi::REMOVE_BIDI_CONTROLS;
  }
  /*
   * Option "remove controls" implies Bidi.REMOVE_BIDI_CONTROLS
   * and cancels Bidi.INSERT_LRM_FOR_NUMERIC.
   */
  if ((bidi->reorderingOptions & Bidi::OPTION_REMOVE_CONTROLS) != 0) {
    options |= Bidi::REMOVE_BIDI_CONTROLS;
    options &= ~Bidi::INSERT_LRM_FOR_NUMERIC;
  }
  /*
   * If we do not perform the "inverse Bidi" algorithm, then we
   * don't need to insert any LRMs, and don't need to test for it.
   */
  if ((bidi->reorderingMode != Bidi::REORDER_INVERSE_NUMBERS_AS_L) &&
      (bidi->reorderingMode != Bidi::REORDER_INVERSE_LIKE_DIRECT) &&
      (bidi->reorderingMode != Bidi::REORDER_INVERSE_FOR_NUMBERS_SPECIAL) &&
      (bidi->reorderingMode != Bidi::REORDER_RUNS_ONLY)) {
    options &= ~Bidi::INSERT_LRM_FOR_NUMERIC;
  }
  //        dest = new StringBuilder((options & Bidi.INSERT_LRM_FOR_NUMERIC) !=
  //        0 ?
  //                                 bidi.length * 2 : bidi.length);
  dest = U"";
  /*
   * Iterate through all visual runs and copy the run text segments to
   * the destination, according to the options.
   *
   * The tests for where to insert LRMs ignore the fact that there may be
   * BN codes or non-BMP code points at the beginning and end of a run;
   * they may insert LRMs unnecessarily but the tests are faster this way
   * (this would have to be improved for UTF-8).
   */
  if ((options & Bidi::OUTPUT_REVERSE) == 0) {
    /* forward output */
    if ((options & Bidi::INSERT_LRM_FOR_NUMERIC) == 0) {
      /* do not insert Bidi controls */
      for (run = 0; run < runCount; ++run) {
        BidiRun bidiRun = bidi->getVisualRun(run);
        if (bidiRun.isEvenRun()) {
          //                    dest.append(doWriteForward(text, bidiRun.start,
          //                                               bidiRun.limit,
          //                                               options &
          //                                               ~Bidi::DO_MIRRORING));
          dest += doWriteForward(text.c_str(), bidiRun.start, bidiRun.limit,
                                 options & ~Bidi::DO_MIRRORING);
        } else {
          //                    dest.append(doWriteReverse(text, bidiRun.start,
          //                                               bidiRun.limit,
          //                                               options));
          dest += doWriteReverse(text.c_str(), bidiRun.start, bidiRun.limit,
                                 options);
        }
      }
    } else {
      /* insert Bidi controls for "inverse Bidi" */
      std::vector<short> dirProps = bidi->dirProps;
      char32_t uc;
      int markFlag;

      for (run = 0; run < runCount; ++run) {
        BidiRun bidiRun = bidi->getVisualRun(run);
        markFlag = 0;
        /* check if something relevant in insertPoints */
        markFlag = bidi->runs[run].insertRemove;
        if (markFlag < 0) { /* bidi controls count */
          markFlag = 0;
        }
        if (bidiRun.isEvenRun()) {
          if (bidi->getIsInverse() && dirProps[bidiRun.start] != Bidi::L) {
            markFlag |= Bidi::LRM_BEFORE;
          }
          if ((markFlag & Bidi::LRM_BEFORE) != 0) {
            uc = LRM_CHAR;
          } else if ((markFlag & Bidi::RLM_BEFORE) != 0) {
            uc = RLM_CHAR;
          } else {
            uc = 0;
          }
          if (uc != 0) {
            dest += uc;
          }
          dest.append(doWriteForward(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options & ~Bidi::DO_MIRRORING));

          if (bidi->getIsInverse() && dirProps[bidiRun.limit - 1] != Bidi::L) {
            markFlag |= Bidi::LRM_AFTER;
          }
          if ((markFlag & Bidi::LRM_AFTER) != 0) {
            uc = LRM_CHAR;
          } else if ((markFlag & Bidi::RLM_AFTER) != 0) {
            uc = RLM_CHAR;
          } else {
            uc = 0;
          }
          if (uc != 0) {
            dest += uc;
          }
        } else { /* RTL run */
          if (bidi->getIsInverse() &&
              !bidi->testDirPropFlagAt(MASK_R_AL, bidiRun.limit - 1)) {
            markFlag |= Bidi::RLM_BEFORE;
          }
          if ((markFlag & Bidi::LRM_BEFORE) != 0) {
            uc = LRM_CHAR;
          } else if ((markFlag & Bidi::RLM_BEFORE) != 0) {
            uc = RLM_CHAR;
          } else {
            uc = 0;
          }
          if (uc != 0) {
            dest += uc;
          }
          dest.append(doWriteReverse(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options));

          if (bidi->getIsInverse() &&
              (MASK_R_AL & Bidi::DirPropFlag(dirProps[bidiRun.start])) == 0) {
            markFlag |= Bidi::RLM_AFTER;
          }
          if ((markFlag & Bidi::LRM_AFTER) != 0) {
            uc = LRM_CHAR;
          } else if ((markFlag & Bidi::RLM_AFTER) != 0) {
            uc = RLM_CHAR;
          } else {
            uc = 0;
          }
          if (uc != 0) {
            dest += uc;
          }
        }
      }
    }
  } else {
    /* reverse output */
    if ((options & Bidi::INSERT_LRM_FOR_NUMERIC) == 0) {
      /* do not insert Bidi controls */
      for (run = runCount; --run >= 0;) {
        BidiRun bidiRun = bidi->getVisualRun(run);
        if (bidiRun.isEvenRun()) {
          dest.append(doWriteReverse(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options & ~Bidi::DO_MIRRORING));
        } else {
          dest.append(doWriteForward(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options));
        }
      }
    } else {
      /* insert Bidi controls for "inverse Bidi" */

      std::vector<short> dirProps = bidi->dirProps;

      for (run = runCount; --run >= 0;) {
        /* reverse output */
        BidiRun bidiRun = bidi->getVisualRun(run);
        if (bidiRun.isEvenRun()) {
          if (dirProps[bidiRun.limit - 1] != Bidi::L) {
            dest += LRM_CHAR;
          }

          dest.append(doWriteReverse(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options & ~Bidi::DO_MIRRORING));

          if (dirProps[bidiRun.start] != Bidi::L) {
            dest += LRM_CHAR;
          }
        } else {
          if ((MASK_R_AL & Bidi::DirPropFlag(dirProps[bidiRun.start])) == 0) {
            dest += RLM_CHAR;
          }

          dest.append(doWriteForward(text.c_str(), bidiRun.start, bidiRun.limit,
                                     options));

          if ((MASK_R_AL & Bidi::DirPropFlag(dirProps[bidiRun.limit - 1])) ==
              0) {
            dest += RLM_CHAR;
          }
        }
      }
    }
  }

  return dest;
}
