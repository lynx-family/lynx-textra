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

#ifndef BIDI_BIDI_H
#define BIDI_BIDI_H

#include <string>
#include <vector>

#include "bidi_character.h"
#include "bidi_run.h"
#include "bidi_writer.h"
#include "log_util.h"
#include "u_character.h"
#include "ucharacter_enums.h"

class Bidi {
 public:
  struct Point {
    int pos;
    int flag;
  };
  struct InsertPoints {
    int size;
    int confirmed;
    std::vector<Point> points;
  };
  struct Opening {
    int position;     /* position of opening bracket */
    int match;        /* matching char or -position of closing bracket */
    int contextPos;   /* position of last strong char found before opening */
    short flags;      /* bits for L or R/AL found within the pair */
    short contextDir; /* L or R according to last strong char before opening */
  };
  struct IsoRun {
    int contextPos;   /* position of char determining context */
    short start;      /* index of first opening entry for this run */
    short limit;      /* index after last opening entry for this run */
    short level;      /* level of this run */
    short lastStrong; /* bidi class of last strong char found in this run */
    short lastBase;   /* bidi class of last base char found in this run */
    short contextDir; /* L or R to use as context for following openings */
  };
  struct BracketData {
    // Todo
    std::vector<Opening> openings = std::vector<Opening>(SIMPLE_OPENINGS_COUNT);
    int isoRunLast; /* index of last used entry */
    /* array of nested isolated sequence entries; can never excess
       UBIDI_MAX_EXPLICIT_LEVEL
       + 1 for index 0, + 1 for before the first isolated sequence */
    std::vector<IsoRun> isoRuns = std::vector<IsoRun>(MAX_EXPLICIT_LEVEL + 2);
    bool isNumbersSpecial; /*reordering mode for NUMBERS_SPECIAL */
  };
  struct Isolate {
    int startON;
    int start1;
    short stateImp;
    short state;
  };

  static const short LEVEL_DEFAULT_LTR = (short)0x7e;

  static const short LEVEL_DEFAULT_RTL = (short)0x7f;

  static const short MAX_EXPLICIT_LEVEL = 125;

  static const short LEVEL_OVERRIDE = (short)0x80;

  static const int MAP_NOWHERE = -1;

  static const short LTR = 0;

  static const short RTL = 1;

  static const short MIXED = 2;

  static const short NEUTRAL = 3;

  static const short KEEP_BASE_COMBINING = 1;

  static const short DO_MIRRORING = 2;

  static const short INSERT_LRM_FOR_NUMERIC = 4;

  static const short REMOVE_BIDI_CONTROLS = 8;

  static const short OUTPUT_REVERSE = 16;

  static const short REORDER_DEFAULT = 0;

  static const short REORDER_NUMBERS_SPECIAL = 1;

  static const short REORDER_GROUP_NUMBERS_WITH_R = 2;

  static const short REORDER_RUNS_ONLY = 3;

  static const short REORDER_INVERSE_NUMBERS_AS_L = 4;

  static const short REORDER_INVERSE_LIKE_DIRECT = 5;

  static const short REORDER_INVERSE_FOR_NUMBERS_SPECIAL = 6;

  static const short REORDER_COUNT = 7;

  static const short REORDER_LAST_LOGICAL_TO_VISUAL = REORDER_NUMBERS_SPECIAL;

  static const int OPTION_DEFAULT = 0;

  static const int OPTION_INSERT_MARKS = 1;

  static const int OPTION_REMOVE_CONTROLS = 2;

  static const int OPTION_STREAMING = 4;

  static const short L = UCharacterEnums::LEFT_TO_RIGHT;               /*  0 */
  static const short R = UCharacterEnums::RIGHT_TO_LEFT;               /*  1 */
  static const short EN = UCharacterEnums::EUROPEAN_NUMBER;            /*  2 */
  static const short ES = UCharacterEnums::EUROPEAN_NUMBER_SEPARATOR;  /*  3 */
  static const short ET = UCharacterEnums::EUROPEAN_NUMBER_TERMINATOR; /*  4 */
  static const short AN = UCharacterEnums::ARABIC_NUMBER;              /*  5 */
  static const short CS = UCharacterEnums::COMMON_NUMBER_SEPARATOR;    /*  6 */
  static const short B = UCharacterEnums::BLOCK_SEPARATOR;             /*  7 */
  static const short S = UCharacterEnums::SEGMENT_SEPARATOR;           /*  8 */
  static const short WS = UCharacterEnums::WHITE_SPACE_NEUTRAL;        /*  9 */
  static const short ON = UCharacterEnums::OTHER_NEUTRAL;              /* 10 */
  static const short LRE = UCharacterEnums::LEFT_TO_RIGHT_EMBEDDING;   /* 11 */
  static const short LRO = UCharacterEnums::LEFT_TO_RIGHT_OVERRIDE;    /* 12 */
  static const short AL = UCharacterEnums::RIGHT_TO_LEFT_ARABIC;       /* 13 */
  static const short RLE = UCharacterEnums::RIGHT_TO_LEFT_EMBEDDING;   /* 14 */
  static const short RLO = UCharacterEnums::RIGHT_TO_LEFT_OVERRIDE;    /* 15 */
  static const short PDF = UCharacterEnums::POP_DIRECTIONAL_FORMAT;    /* 16 */
  static const short NSM = UCharacterEnums::DIR_NON_SPACING_MARK;      /* 17 */
  static const short BN = UCharacterEnums::BOUNDARY_NEUTRAL;           /* 18 */
  static const short FSI = UCharacterEnums::FIRST_STRONG_ISOLATE;      /* 19 */
  static const short LRI = UCharacterEnums::LEFT_TO_RIGHT_ISOLATE;     /* 20 */
  static const short RLI = UCharacterEnums::RIGHT_TO_LEFT_ISOLATE;     /* 21 */
  static const short PDI = UCharacterEnums::POP_DIRECTIONAL_ISOLATE;   /* 22 */
  static const short ENL = PDI + 1; /* EN after W7 */                  /* 23 */
  static const short ENR = ENL + 1; /* EN not subject to W7 */         /* 24 */

  /* number of paras entries allocated initially */
  static const int SIMPLE_PARAS_COUNT = 10;
  /* number of isolate run entries for paired brackets allocated initially */
  static const int SIMPLE_OPENINGS_COUNT = 20;

  static const char32_t CR = '\r';
  static const char32_t LF = '\n';

  static const int LRM_BEFORE = 1;
  static const int LRM_AFTER = 2;
  static const int RLM_BEFORE = 4;
  static const int RLM_AFTER = 8;

  /* flags for Opening.flags */
  static const short FOUND_L = 1 << L;  // DirPropFlag(L);
  static const short FOUND_R = 1 << R;  // DirPropFlag(R);

  /*
   * The following bit is used for the directional isolate status.
   * Stack entries corresponding to isolate sequences are greater than ISOLATE.
   */
  static const int ISOLATE = 0x0100;

  /* character array representing the current text */
  // char *text;
  // std::u32string text_utf32;
  // BidiString* bidi_text;
  std::u32string text_;

  /* length of the current text */
  int originalLength;

  /* if the option OPTION_STREAMING is set, this is the length of
   * text actually processed by <code>setPara</code>, which may be shorter
   * than the original length. Otherwise, it is identical to the original
   * length.
   */
  int length;

  /* if option OPTION_REMOVE_CONTROLS is set, and/or Bidi
   * marks are allowed to be inserted in one of the reordering modes, the
   * length of the result string may be different from the processed length.
   */
  int resultLength;

  /* indicators for whether memory may be allocated after construction */
  bool mayAllocateText;
  bool mayAllocateRuns;

  /* arrays with one value per text-character */
  // std::vector<short> dirPropsMemory;
  // std::vector<short> levelsMemory;
  std::vector<short> dirProps;
  std::vector<short> levels;

  /* are we performing an approximation of the "inverse Bidi" algorithm? */
  bool isInverse = false;  // TODO: if this should be initiate too?

  /* are we using the basic algorithm or its variation? */
  int reorderingMode = REORDER_DEFAULT;

  /* bitmask for reordering options */
  int reorderingOptions = OPTION_DEFAULT;

  /* must block separators receive level 0? */
  bool orderParagraphsLTR = false;  // this must be initiated to false

  /* the paragraph level */
  short paraLevel;
  /* original paraLevel when contextual */
  /* must be one of DEFAULT_xxx or 0 if not contextual */
  short defaultParaLevel;

  /* context data */
  std::string prologue;
  std::string epilogue;

  struct ImpTabPair {
    std::vector<std::vector<std::vector<short>>> imptab;
    std::vector<std::vector<short>> impact;
    ImpTabPair(){};  // TODO: if this initialization function is not added,
                     // Bidi() will prompt that impTabPair cannot be
                     // initialized

    ImpTabPair(std::vector<std::vector<short>> table1,
               std::vector<std::vector<short>> table2, std::vector<short> act1,
               std::vector<short> act2) {
      imptab = std::vector<std::vector<std::vector<short>>>{table1, table2};
      impact = std::vector<std::vector<short>>{act1, act2};
    }
  };

  /* the following is set in setPara, used in processPropertySeq */

  ImpTabPair impTabPair; /* reference to levels state table pair */
  /* the overall paragraph or line directionality*/
  short direction;

  /* flags is a bit set for which directional properties are in the text */
  int flags;

  /* lastArabicPos is index to the last AL in the text, -1 if none */
  int lastArabicPos;

  /* characters after trailingWSStart are WS and are */
  /* implicitly at the paraLevel (rule (L1)) - levels may not reflect that */
  int trailingWSStart;

  /* fields for paragraph handling, set in getDirProps() */
  int paraCount;
  std::vector<int> paras_limit = std::vector<int>(SIMPLE_PARAS_COUNT);
  std::vector<short> paras_level = std::vector<short>(SIMPLE_PARAS_COUNT);

  /* fields for line reordering */
  int runCount; /* ==-1: runs not set up yet */
  // std::vector<BidiRun> runsMemory;
  std::vector<BidiRun> runs;

  /* for non-mixed text, we only need a tiny array of runs (no allocation) */
  std::vector<BidiRun> simpleRuns = {BidiRun()};

  /* fields for managing isolate sequences */
  std::vector<Isolate> isolates;
  /* maximum or current nesting depth of isolate sequences */
  /* Within resolveExplicitLevels() and checkExplicitLevels(), this is the
     maximal nesting encountered. Within resolveImplicitLevels(), this is the
     index of the current isolates stack entry. */
  int isolateCount;

  /* mapping of runs in logical order to visual order */
  std::vector<int> logicalToVisualRunsMap;
  /* flag to indicate that the map has been updated */
  bool isGoodLogicalToVisualRunsMap;

  /* customized class provider */
  // BidiClassifier      customClassifier = null;

  /* for inverse Bidi with insertion of directional marks */
  InsertPoints insertPoints;

  /* for option OPTION_REMOVE_CONTROLS */
  int controlCount;

  /*
   * Sometimes, bit values are more appropriate
   * to deal with directionality properties.
   * Abbreviations in these method names refer to names
   * used in the Bidi algorithm.
   */
  static int DirPropFlag(short dir) { return (1 << dir); }

  bool testDirPropFlagAt(int flag, int index) {
    return ((DirPropFlag(dirProps[index]) & flag) != 0);
  }

  // static const int DirPropFlagMultiRuns = DirPropFlag((short) 31);
  static const int DirPropFlagMultiRuns;

  /* to avoid some conditional statements, use tiny constant arrays */
  // static const std::vector<int> DirPropFlagLR[] = {DirPropFlag(L),
  // DirPropFlag(R)};
  static const std::vector<int> DirPropFlagLR;
  // static const int DirPropFlagE[] = {DirPropFlag(LRE), DirPropFlag(RLE)};
  static const std::vector<int> DirPropFlagE;
  // static const int DirPropFlagO[] = {DirPropFlag(LRO), DirPropFlag(RLO)};
  static const std::vector<int> DirPropFlagO;

  static int getDirPropFlagLR(short level) { return DirPropFlagLR[level & 1]; }

  static int getDirPropFlagE(short level) { return DirPropFlagE[level & 1]; }

  static int getDirPropFlagO(short level) { return DirPropFlagO[level & 1]; }

  static short DirFromStrong(short strong) { return strong == L ? L : R; }

  static short NoOverride(short level) {
    return (short)(level & ~LEVEL_OVERRIDE);
  }

  /*  are there any characters that are LTR or RTL? */
  //    static const int MASK_LTR =
  //            DirPropFlag(L) | DirPropFlag(EN) | DirPropFlag(ENL) |
  //            DirPropFlag(ENR) | DirPropFlag(AN) | DirPropFlag(LRE) |
  //            DirPropFlag(LRO) | DirPropFlag(LRI);
  static const int MASK_LTR;
  //    static const int MASK_RTL =
  //            DirPropFlag(R) | DirPropFlag(AL) | DirPropFlag(RLE) |
  //            DirPropFlag(RLO) | DirPropFlag(RLI);
  static const int MASK_RTL;
  //    static const int MASK_R_AL = DirPropFlag(R) | DirPropFlag(AL);
  static const int MASK_R_AL;
  //    static const int MASK_STRONG_EN_AN =
  //            DirPropFlag(L) | DirPropFlag(R) | DirPropFlag(AL) |
  //            DirPropFlag(EN) | DirPropFlag(AN);
  static const int MASK_STRONG_EN_AN;
  /* explicit embedding codes */
  //    static const int MASK_EXPLICIT =
  //            DirPropFlag(LRE) | DirPropFlag(LRO) | DirPropFlag(RLE) |
  //            DirPropFlag(RLO) | DirPropFlag(PDF);
  static const int MASK_EXPLICIT;
  //    static const int MASK_BN_EXPLICIT = DirPropFlag(BN) | MASK_EXPLICIT;
  static const int MASK_BN_EXPLICIT;

  /* explicit isolate codes */
  //    static const int MASK_ISO = DirPropFlag(LRI) | DirPropFlag(RLI) |
  //    DirPropFlag(FSI) | DirPropFlag(PDI);
  static const int MASK_ISO;

  /* paragraph and segment separators */
  //    static const int MASK_B_S = DirPropFlag(B) | DirPropFlag(S);
  static const int MASK_B_S;

  /* all types that are counted as White Space or Neutral in some steps */
  //    static const int MASK_WS = MASK_B_S | DirPropFlag(WS) | MASK_BN_EXPLICIT
  //    | MASK_ISO;
  static const int MASK_WS;
  /* types that are neutrals or could becomes neutrals in (Wn) */
  //    static const int MASK_POSSIBLE_N = DirPropFlag(ON) | DirPropFlag(CS) |
  //    DirPropFlag(ES) | DirPropFlag(ET) | MASK_WS;
  static const int MASK_POSSIBLE_N;

  /*
   * These types may be changed to "e",
   * the embedding type (L or R) of the run,
   * in the Bidi algorithm (N2)
   */
  // static const int MASK_EMBEDDING = DirPropFlag(NSM) | MASK_POSSIBLE_N;
  static const int MASK_EMBEDDING;

  /*
   *  the dirProp's L and R are defined to 0 and 1 values in
   * UCharacterDirection.java
   */
  static inline short GetLRFromLevel(short level) { return (short)(level & 1); }

  static inline bool IsDefaultLevel(short level) {
    return ((level & LEVEL_DEFAULT_LTR) == LEVEL_DEFAULT_LTR);
  }

  static inline bool IsBidiControlChar(char32_t c) {
    /* check for range 0x200c to 0x200f (ZWNJ, ZWJ, LRM, RLM) or
                       0x202a to 0x202e (LRE, RLE, PDF, LRO, RLO) */
    return (((c & 0xfffffffc) == 0x200c) || ((c >= 0x202a) && (c <= 0x202e)) ||
            ((c >= 0x2066) && (c <= 0x2069)));
  }

  void verifyRange(int index, int start, int limit) {
    if (index < start || index >= limit) {
      LogUtil::E("verifyRange failed");
      //                throw new IllegalArgumentException("Value " + index +
      //                                                   " is out of range " +
      //                                                   start
      //                                                   + " to " + limit);
    }
  }

  Bidi() { Bidi(0, 0); }

  Bidi(int maxLength, int maxRunCount) {
    /* check the argument values */
    //        if (maxLength < 0 || maxRunCount < 0) {
    //            throw new IllegalArgumentException();
    //        }

    /* allocate memory for arrays as requested */
    if (maxLength > 0) {
      getInitialDirPropsMemory(maxLength);
      getInitialLevelsMemory(maxLength);
    } else {
      mayAllocateText = true;
    }

    if (maxRunCount > 0) {
      // if maxRunCount == 1, use simpleRuns[]
      if (maxRunCount > 1) {
        getInitialRunsMemory(maxRunCount);
      }
    } else {
      mayAllocateRuns = true;
    }
  }

  /* additional methods used by constructor - always allow allocation */
  void getInitialDirPropsMemory(int len) {
    dirProps = std::vector<short>(len, 0);
  }

  void getInitialLevelsMemory(int len) { levels = std::vector<short>(len, 0); }

  void getInitialRunsMemory(int len) { runs = std::vector<BidiRun>(len); }

  // isInverse
  bool getIsInverse() { return isInverse; }

  // TODO: prologue may be unused
  short firstL_R_AL() {
    short result = ON;
    for (size_t i = 0; i < prologue.length();) {
      // int uchar = prologue.codePointAt(i);
      int uchar = prologue[i];
      i += Character::charCount(uchar);
      short dirProp = (short)getCustomizedClass(uchar);
      if (result == ON) {
        if (dirProp == L || dirProp == R || dirProp == AL) {
          result = dirProp;
        }
      } else {
        if (dirProp == B) {
          result = ON;
        }
      }
    }
    return result;
  }

  // TODO: this operation seems redundant for vector
  void checkParaCount() {
    std::vector<int> saveLimits;
    std::vector<short> saveLevels;
    size_t count = paraCount;
    if (count <= paras_level.size()) return;
    // int oldLength = paras_level.size();
    saveLimits = paras_limit;
    saveLevels = paras_level;
    paras_limit = std::vector<int>(count * 2);
    paras_level = std::vector<short>(count * 2);
    for (size_t i = 0; i < saveLimits.size(); i++) {
      paras_limit[i] = saveLimits[i];
    }
    for (size_t i = 0; i < saveLevels.size(); i++) {
      paras_level[i] = saveLevels[i];
    }
  }

  /*
   * Get the directional properties for the text, calculate the flags bit-set,
   * and determine the paragraph level if necessary (in paras_level[i]). FSI
   * initiators are also resolved and their dirProp replaced with LRI or RLI.
   * When encountering an FSI, it is initially replaced with an LRI, which is
   * the default. Only if a strong R or AL is found within its scope will the
   * LRI be replaced by an RLI.
   */
  static const int NOT_SEEKING_STRONG =
      0; /* 0: not contextual paraLevel, not after FSI */
  static const int SEEKING_STRONG_FOR_PARA =
      1; /* 1: looking for first strong char in para */
  static const int SEEKING_STRONG_FOR_FSI =
      2; /* 2: looking for first strong after FSI */
  static const int LOOKING_FOR_PDI =
      3; /* 3: found strong after FSI, looking for PDI */

  void getDirProps() {
    int i = 0, i0, i1;
    flags = 0; /* collect all directionalities in the text */
    char32_t uchar;
    short dirProp;
    short defaultParaLevel = 0; /* initialize to avoid compiler warnings */
    bool isDefaultLevel = IsDefaultLevel(paraLevel);
    /* for inverse Bidi, the default para level is set to RTL if there is a
       strong R or AL character at either end of the text                */
    bool isDefaultLevelInverse =
        isDefaultLevel &&
        (reorderingMode == REORDER_INVERSE_LIKE_DIRECT ||
         reorderingMode == REORDER_INVERSE_FOR_NUMBERS_SPECIAL);
    lastArabicPos = -1;
    int controlCount = 0;
    bool removeBidiControls = (reorderingOptions & OPTION_REMOVE_CONTROLS) != 0;

    short state;
    short lastStrong = ON; /* for default level & inverse Bidi */
    /* The following stacks are used to manage isolate sequences. Those
       sequences may be nested, but obviously never more deeply than the
       maximum explicit embedding level.
       lastStack is the index of the last used entry in the stack. A value of -1
       means that there is no open isolate sequence.
       lastStack is reset to -1 on paragraph boundaries. */
    /* The following stack contains the position of the initiator of
       each open isolate sequence */
    std::vector<int> isolateStartStack =
        std::vector<int>(MAX_EXPLICIT_LEVEL + 1);
    /* The following stack contains the last known state before
       encountering the initiator of an isolate sequence */
    std::vector<short> previousStateStack =
        std::vector<short>(MAX_EXPLICIT_LEVEL + 1);
    int stackLast = -1;

    if ((reorderingOptions & OPTION_STREAMING) != 0) {
      length = 0;
    }
    defaultParaLevel = (short)(paraLevel & 1);

    if (isDefaultLevel) {
      paras_level[0] = defaultParaLevel;
      lastStrong = defaultParaLevel;
      if (prologue.length() &&               /* there is a prologue */
          (dirProp = firstL_R_AL()) != ON) { /* with a strong character */
        if (dirProp == L)
          paras_level[0] = 0; /* set the default para level */
        else
          paras_level[0] = 1; /* set the default para level */
        state = NOT_SEEKING_STRONG;
      } else {
        state = SEEKING_STRONG_FOR_PARA;
      }
    } else {
      paras_level[0] = paraLevel;
      state = NOT_SEEKING_STRONG;
    }
    /* count paragraphs and determine the paragraph level (P2..P3) */
    /*
     * see comment on constant fields:
     * the LEVEL_DEFAULT_XXX values are designed so that
     * their low-order bit alone yields the intended default
     */
    //    auto str = std::u32string(bidi_text->getU32Str());
    // const auto& str = text_;
    for (i = 0; i < originalLength; /* i is incremented in the loop */) {
      i0 = i; /* index of first code unit */
      uchar = text_[i];
      i++;
      i1 = i - 1; /* index of last code unit, gets the directional property */

      dirProp = (short)getCustomizedClass(uchar);
      flags |= DirPropFlag(dirProp);
      dirProps[i1] = dirProp;
      if (i1 > i0) { /* set previous code units' properties to BN */
        flags |= DirPropFlag(BN);
        do {
          dirProps[--i1] = BN;
        } while (i1 > i0);
      }
      if (removeBidiControls && IsBidiControlChar(uchar)) {
        controlCount++;
      }
      if (dirProp == L) {
        if (state == SEEKING_STRONG_FOR_PARA) {
          paras_level[paraCount - 1] = 0;
          state = NOT_SEEKING_STRONG;
        } else if (state == SEEKING_STRONG_FOR_FSI) {
          if (stackLast <= MAX_EXPLICIT_LEVEL) {
            /* no need for next statement, already set by default */
            /* dirProps[isolateStartStack[stackLast]] = LRI; */
            flags |= DirPropFlag(LRI);
          }
          state = LOOKING_FOR_PDI;
        }
        lastStrong = L;
        continue;
      }
      if (dirProp == R || dirProp == AL) {
        if (state == SEEKING_STRONG_FOR_PARA) {
          paras_level[paraCount - 1] = 1;
          state = NOT_SEEKING_STRONG;
        } else if (state == SEEKING_STRONG_FOR_FSI) {
          if (stackLast <= MAX_EXPLICIT_LEVEL) {
            dirProps[isolateStartStack[stackLast]] = RLI;
            flags |= DirPropFlag(RLI);
          }
          state = LOOKING_FOR_PDI;
        }
        lastStrong = R;
        if (dirProp == AL) lastArabicPos = i - 1;
        continue;
      }
      if (dirProp >= FSI && dirProp <= RLI) { /* FSI, LRI or RLI */
        stackLast++;
        if (stackLast <= MAX_EXPLICIT_LEVEL) {
          isolateStartStack[stackLast] = i - 1;
          previousStateStack[stackLast] = state;
        }
        if (dirProp == FSI) {
          dirProps[i - 1] = LRI; /* default if no strong char */
          state = SEEKING_STRONG_FOR_FSI;
        } else
          state = LOOKING_FOR_PDI;
        continue;
      }
      if (dirProp == PDI) {
        if (state == SEEKING_STRONG_FOR_FSI) {
          if (stackLast <= MAX_EXPLICIT_LEVEL) {
            /* no need for next statement, already set by default */
            /* dirProps[isolateStartStack[stackLast]] = LRI; */
            flags |= DirPropFlag(LRI);
          }
        }
        if (stackLast >= 0) {
          if (stackLast <= MAX_EXPLICIT_LEVEL)
            state = previousStateStack[stackLast];
          stackLast--;
        }
        continue;
      }
      if (dirProp == B) {
        //                if (i < originalLength && uchar == CR && text_utf32[i]
        //                == LF) /* do nothing on the CR */
        //                    continue;
        if (i < originalLength && uchar == CR && text_[i] == LF) {
          continue;
        }
        paras_limit[paraCount - 1] = i;
        if (isDefaultLevelInverse && lastStrong == R)
          paras_level[paraCount - 1] = 1;
        if ((reorderingOptions & OPTION_STREAMING) != 0) {
          /* When streaming, we only process whole paragraphs
             thus some updates are only done on paragraph boundaries */
          length = i; /* i is index to next character */
          this->controlCount = controlCount;
        }
        if (i < originalLength) { /* B not last char in text */
          paraCount++;
          checkParaCount(); /* check that there is enough memory for a new para
                               entry */
          if (isDefaultLevel) {
            paras_level[paraCount - 1] = defaultParaLevel;
            state = SEEKING_STRONG_FOR_PARA;
            lastStrong = defaultParaLevel;
          } else {
            paras_level[paraCount - 1] = paraLevel;
            state = NOT_SEEKING_STRONG;
          }
          stackLast = -1;
        }
        continue;
      }
    }
    /* +Ignore still open isolate sequences with overflow */
    if (stackLast > MAX_EXPLICIT_LEVEL) {
      stackLast = MAX_EXPLICIT_LEVEL;
      state = SEEKING_STRONG_FOR_FSI; /* to be on the safe side */
    }
    /* Resolve direction of still unresolved open FSI sequences */
    while (stackLast >= 0) {
      if (state == SEEKING_STRONG_FOR_FSI) {
        /* no need for next statement, already set by default */
        /* dirProps[isolateStartStack[stackLast]] = LRI; */
        flags |= DirPropFlag(LRI);
        break;
      }
      state = previousStateStack[stackLast];
      stackLast--;
    }
    /* When streaming, ignore text after the last paragraph separator */
    if ((reorderingOptions & OPTION_STREAMING) != 0) {
      if (length < originalLength) paraCount--;
    } else {
      paras_limit[paraCount - 1] = originalLength;
      this->controlCount = controlCount;
    }
    /* For inverse bidi, default para direction is RTL if there is
       a strong R or AL at either end of the paragraph */
    if (isDefaultLevelInverse && lastStrong == R) {
      paras_level[paraCount - 1] = 1;
    }
    if (isDefaultLevel) {
      paraLevel = paras_level[0];
    }
    /* The following is needed to resolve the text direction for default level
       paragraphs containing no strong character */
    for (i = 0; i < paraCount; i++) flags |= getDirPropFlagLR(paras_level[i]);

    if (orderParagraphsLTR && (flags & DirPropFlag(B)) != 0) {
      flags |= DirPropFlag(L);
    }
  }

  /* determine the paragraph level at position index */
  short GetParaLevelAt(int pindex) {
    if (defaultParaLevel == 0 || pindex < paras_limit[0]) return paraLevel;
    int i;
    for (i = 1; i < paraCount; i++)
      if (pindex < paras_limit[i]) break;
    if (i >= paraCount) i = paraCount - 1;
    return paras_level[i];
  }

  /* Functions for handling paired brackets -----------------------------------
   */
  void bracketInit(BracketData& bd) {
    bd.isoRunLast = 0;
    bd.isoRuns[0] = IsoRun();
    bd.isoRuns[0].start = 0;
    bd.isoRuns[0].limit = 0;
    bd.isoRuns[0].level = GetParaLevelAt(0);
    bd.isoRuns[0].lastStrong = bd.isoRuns[0].lastBase =
        bd.isoRuns[0].contextDir = (short)(GetParaLevelAt(0) & 1);
    bd.isoRuns[0].contextPos = 0;
    bd.openings = std::vector<Opening>(SIMPLE_OPENINGS_COUNT);
    bd.isNumbersSpecial = reorderingMode == REORDER_NUMBERS_SPECIAL ||
                          reorderingMode == REORDER_INVERSE_FOR_NUMBERS_SPECIAL;
  }

  void bracketProcessB(BracketData& bd, short level) {
    bd.isoRunLast = 0;
    bd.isoRuns[0].limit = 0;
    bd.isoRuns[0].level = level;
    bd.isoRuns[0].lastStrong = bd.isoRuns[0].lastBase =
        bd.isoRuns[0].contextDir = (short)(level & 1);
    bd.isoRuns[0].contextPos = 0;
  }

  void bracketProcessBoundary(BracketData& bd, int lastCcPos,
                              short contextLevel, short embeddingLevel) {
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    if ((DirPropFlag(dirProps[lastCcPos]) & MASK_ISO) !=
        0) /* after an isolate */
      return;
    if (NoOverride(embeddingLevel) > NoOverride(contextLevel)) /* not a PDF */
      contextLevel = embeddingLevel;
    pLastIsoRun.limit = pLastIsoRun.start;
    pLastIsoRun.level = embeddingLevel;
    pLastIsoRun.lastStrong = pLastIsoRun.lastBase = pLastIsoRun.contextDir =
        (short)(contextLevel & 1);
    pLastIsoRun.contextPos = lastCcPos;
  }

  void bracketProcessLRI_RLI(BracketData& bd, short level) {
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    short lastLimit;
    pLastIsoRun.lastBase = ON;
    lastLimit = pLastIsoRun.limit;
    bd.isoRunLast++;
    IsoRun& pLastIsoRun_ = bd.isoRuns[bd.isoRunLast];
    // if (pLastIsoRun == nullptr)//TODO
    pLastIsoRun_ = bd.isoRuns[bd.isoRunLast] = IsoRun();
    pLastIsoRun_.start = pLastIsoRun_.limit = lastLimit;
    pLastIsoRun_.level = level;
    pLastIsoRun_.lastStrong = pLastIsoRun_.lastBase = pLastIsoRun_.contextDir =
        (short)(level & 1);
    pLastIsoRun_.contextPos = 0;
  }

  void bracketProcessPDI(BracketData& bd) {
    // IsoRun pLastIsoRun;
    bd.isoRunLast--;
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    pLastIsoRun.lastBase = ON;
  }

  void bracketAddOpening(BracketData& bd, char32_t match, int position) {
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    Opening& pOpening = bd.openings[pLastIsoRun.limit];
    if ((uint32_t)pLastIsoRun.limit >=
        bd.openings.size()) { /* no available new entry */
      std::vector<Opening> saveOpenings = bd.openings;
      int count;
      //            try {
      count = bd.openings.size();
      bd.openings = std::vector<Opening>(count * 2);
      //            } catch (Exception e) {
      //                throw new OutOfMemoryError("Failed to allocate memory
      //                for openings");
      //            }
      // System.arraycopy(saveOpenings, 0, bd.openings, 0, count);
      for (int i = 0; i < count; i++) {
        bd.openings[i] = saveOpenings[i];
      }
    }
    // pOpening = bd.openings[pLastIsoRun.limit];
    // if (pOpening == null)//TODO
    //     pOpening = bd.openings[pLastIsoRun.limit] = Opening();
    pOpening.position = position;
    pOpening.match = match;
    pOpening.contextDir = pLastIsoRun.contextDir;
    pOpening.contextPos = pLastIsoRun.contextPos;
    pOpening.flags = 0;
    pLastIsoRun.limit++;
  }

  void fixN0c(BracketData& bd, int openingIndex, int newPropPosition,
              short newProp) {
    /* This function calls itself recursively */
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    // Opening qOpening;
    int k, openingPosition, closingPosition;
    for (k = openingIndex + 1; k < pLastIsoRun.limit; k++) {
      Opening& qOpening = bd.openings[k];
      if (qOpening.match >= 0) /* not an N0c match */
        continue;
      if (newPropPosition < qOpening.contextPos) break;
      if (newPropPosition >= qOpening.position) continue;
      if (newProp == qOpening.contextDir) break;
      openingPosition = qOpening.position;
      dirProps[openingPosition] = newProp;
      closingPosition = -(qOpening.match);
      dirProps[closingPosition] = newProp;
      qOpening.match = 0; /* prevent further changes */
      fixN0c(bd, k, openingPosition, newProp);
      fixN0c(bd, k, closingPosition, newProp);
    }
  }

  short bracketProcessClosing(BracketData& bd, int openIdx, int position) {
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    // Opening pOpening, qOpening;
    short direction;
    bool stable;
    short newProp;
    Opening& pOpening = bd.openings[openIdx];
    direction = (short)(pLastIsoRun.level & 1);
    stable = true; /* assume stable until proved otherwise */

    /* The stable flag is set when brackets are paired and their
       level is resolved and cannot be changed by what will be
       found later in the source string.
       An unstable match can occur only when applying N0c, where
       the resolved level depends on the preceding context, and
       this context may be affected by text occurring later.
       Example: RTL paragraph containing:  abc[(latin) HEBREW]
       When the closing parenthesis is encountered, it appears
       that N0c1 must be applied since 'abc' sets an opposite
       direction context and both parentheses receive level 2.
       However, when the closing square bracket is processed,
       N0b applies because of 'HEBREW' being included within the
       brackets, thus the square brackets are treated like R and
       receive level 1. However, this changes the preceding
       context of the opening parenthesis, and it now appears
       that N0c2 must be applied to the parentheses rather than
       N0c1. */

    if ((direction == 0 && (pOpening.flags & FOUND_L) > 0) ||
        (direction == 1 && (pOpening.flags & FOUND_R) > 0)) { /* N0b */
      newProp = direction;
    } else if ((pOpening.flags & (FOUND_L | FOUND_R)) != 0) { /* N0c */
      /* it is stable if there is no preceding text or in
         conditions too complicated and not worth checking */
      stable = (openIdx == pLastIsoRun.start);
      if (direction != pOpening.contextDir)
        newProp = pOpening.contextDir; /* N0c1 */
      else
        newProp = direction; /* N0c2 */
    } else {
      /* forget this and any brackets nested within this pair */
      pLastIsoRun.limit = (short)openIdx;
      return ON; /* N0d */
    }
    dirProps[pOpening.position] = newProp;
    dirProps[position] = newProp;
    /* Update nested N0c pairs that may be affected */
    fixN0c(bd, openIdx, pOpening.position, newProp);
    if (stable) {
      pLastIsoRun.limit =
          (short)openIdx; /* forget any brackets nested within this pair */
      /* remove lower located synonyms if any */
      while (pLastIsoRun.limit > pLastIsoRun.start &&
             bd.openings[pLastIsoRun.limit - 1].position == pOpening.position)
        pLastIsoRun.limit--;
    } else {
      int k;
      pOpening.match = -position;
      /* neutralize lower located synonyms if any */
      k = openIdx - 1;
      while (k >= pLastIsoRun.start &&
             bd.openings[k].position == pOpening.position)
        bd.openings[k--].match = 0;
      /* neutralize any unmatched opening between the current pair;
         this will also neutralize higher located synonyms if any */
      for (k = openIdx + 1; k < pLastIsoRun.limit; k++) {
        Opening& qOpening = bd.openings[k];
        if (qOpening.position >= position) break;
        if (qOpening.match > 0) qOpening.match = 0;
      }
    }
    return newProp;
  }

  void bracketProcessChar(BracketData& bd, int position) {
    IsoRun& pLastIsoRun = bd.isoRuns[bd.isoRunLast];
    short dirProp, newProp;
    short level;
    dirProp = dirProps[position];
    if (dirProp == ON) {
      char32_t c, match;
      int idx;
      /* First see if it is a matching closing bracket. Hopefully, this is
         more efficient than checking if it is a closing bracket at all */
      // c = text_utf32[position];
      c = text_[position];
      for (idx = pLastIsoRun.limit - 1; idx >= pLastIsoRun.start; idx--) {
        if (bd.openings[idx].match != (int)c) continue;
        /* We have a match */
        newProp = bracketProcessClosing(bd, idx, position);
        if (newProp == ON) { /* N0d */
          c = 0;             /* prevent handling as an opening */
          break;
        }
        pLastIsoRun.lastBase = ON;
        pLastIsoRun.contextDir = newProp;
        pLastIsoRun.contextPos = position;
        level = levels[position];
        if ((level & LEVEL_OVERRIDE) != 0) { /* X4, X5 */
          short flag;
          int i;
          newProp = (short)(level & 1);
          pLastIsoRun.lastStrong = newProp;
          flag = (short)DirPropFlag(newProp);
          for (i = pLastIsoRun.start; i < idx; i++)
            bd.openings[i].flags |= flag;
          /* matching brackets are not overridden by LRO/RLO */
          levels[position] &= ~LEVEL_OVERRIDE;
        }
        /* matching brackets are not overridden by LRO/RLO */
        levels[bd.openings[idx].position] &= ~LEVEL_OVERRIDE;
        return;
      }
      /* We get here only if the ON character is not a matching closing
         bracket or it is a case of N0d */
      /* Now see if it is an opening bracket */
      if (c != 0) {
        match = (char32_t)UCharacter::getBidiPairedBracket(
            c); /* get the matching char */
      } else {
        match = 0;
      }
      if (match != c && /* has a matching char */
          UCharacter::getBidiPairedBracketType(c) ==
              /* opening bracket */ UCharacter::BidiPairedBracketType::kOPEN) {
        /* special case: process synonyms
           create an opening entry for each synonym */
        if (match == 0x232A) { /* RIGHT-POINTING ANGLE BRACKET */
          bracketAddOpening(bd, (char32_t)0x3009, position);
        } else if (match == 0x3009) { /* RIGHT ANGLE BRACKET */
          bracketAddOpening(bd, (char32_t)0x232A, position);
        }
        bracketAddOpening(bd, match, position);
      }
    }
    level = levels[position];
    if ((level & LEVEL_OVERRIDE) != 0) { /* X4, X5 */
      newProp = (short)(level & 1);
      if (dirProp != S && dirProp != WS && dirProp != ON)
        dirProps[position] = newProp;
      pLastIsoRun.lastBase = newProp;
      pLastIsoRun.lastStrong = newProp;
      pLastIsoRun.contextDir = newProp;
      pLastIsoRun.contextPos = position;
    } else if (dirProp <= R || dirProp == AL) {
      newProp = DirFromStrong(dirProp);
      pLastIsoRun.lastBase = dirProp;
      pLastIsoRun.lastStrong = dirProp;
      pLastIsoRun.contextDir = newProp;
      pLastIsoRun.contextPos = position;
    } else if (dirProp == EN) {
      pLastIsoRun.lastBase = EN;
      if (pLastIsoRun.lastStrong == L) {
        newProp = L; /* W7 */
        if (!bd.isNumbersSpecial) dirProps[position] = ENL;
        pLastIsoRun.contextDir = L;
        pLastIsoRun.contextPos = position;
      } else {
        newProp = R; /* N0 */
        if (pLastIsoRun.lastStrong == AL)
          dirProps[position] = AN; /* W2 */
        else
          dirProps[position] = ENR;
        pLastIsoRun.contextDir = R;
        pLastIsoRun.contextPos = position;
      }
    } else if (dirProp == AN) {
      newProp = R; /* N0 */
      pLastIsoRun.lastBase = AN;
      pLastIsoRun.contextDir = R;
      pLastIsoRun.contextPos = position;
    } else if (dirProp == NSM) {
      /* if the last real char was ON, change NSM to ON so that it
         will stay ON even if the last real char is a bracket which
         may be changed to L or R */
      newProp = pLastIsoRun.lastBase;
      if (newProp == ON) dirProps[position] = newProp;
    } else {
      newProp = dirProp;
      pLastIsoRun.lastBase = dirProp;
    }
    if (newProp <= R || newProp == AL) {
      int i;
      short flag = (short)DirPropFlag(DirFromStrong(newProp));
      for (i = pLastIsoRun.start; i < pLastIsoRun.limit; i++)
        if (position > bd.openings[i].position) bd.openings[i].flags |= flag;
    }
  }

  short directionFromFlags() {
    /* if the text contains AN and neutrals, then some neutrals may become RTL
     */
    if (!((flags & MASK_RTL) != 0 ||
          ((flags & DirPropFlag(AN)) != 0 && (flags & MASK_POSSIBLE_N) != 0))) {
      return LTR;
    } else if ((flags & MASK_LTR) == 0) {
      return RTL;
    } else {
      return MIXED;
    }
  }

  short resolveExplicitLevels() {
    int i = 0;
    short dirProp;
    short level = GetParaLevelAt(0);
    short dirct;
    isolateCount = 0;

    /* determine if the text is mixed-directional or single-directional */
    dirct = directionFromFlags();

    /* we may not need to resolve any explicit levels */
    if (dirct != MIXED) {
      /* not mixed directionality: levels don't matter - trailingWSStart will be
       * 0 */
      return dirct;
    }
    if (reorderingMode > REORDER_LAST_LOGICAL_TO_VISUAL) {
      /* inverse BiDi: mixed, but all characters are at the same embedding level
       */
      /* set all levels to the paragraph level */
      int paraIndex, start, limit;
      for (paraIndex = 0; paraIndex < paraCount; paraIndex++) {
        if (paraIndex == 0)
          start = 0;
        else
          start = paras_limit[paraIndex - 1];
        limit = paras_limit[paraIndex];
        level = paras_level[paraIndex];
        for (i = start; i < limit; i++) levels[i] = level;
      }
      return dirct; /* no bracket matching for inverse BiDi */
    }
    if ((flags & (MASK_EXPLICIT | MASK_ISO)) == 0) {
      /* no embeddings, set all levels to the paragraph level */
      /* we still have to perform bracket matching */
      int paraIndex, start, limit;
      BracketData bracketData = BracketData();
      bracketInit(bracketData);
      for (paraIndex = 0; paraIndex < paraCount; paraIndex++) {
        if (paraIndex == 0)
          start = 0;
        else
          start = paras_limit[paraIndex - 1];
        limit = paras_limit[paraIndex];
        level = paras_level[paraIndex];
        for (i = start; i < limit; i++) {
          levels[i] = level;
          dirProp = dirProps[i];
          if (dirProp == BN) continue;
          if (dirProp == B) {
            if ((i + 1) < length) {
              //                            if (text_utf32[i] == CR &&
              //                            text_utf32[i + 1] == LF)
              //                                continue;   /* skip CR when
              //                                followed by LF */
              if (text_[i] == CR && text_[i + 1] == LF) {
                continue;
              }
              bracketProcessB(bracketData, level);
            }
            continue;
          }
          bracketProcessChar(bracketData, i);
        }
      }
      return dirct;
    }

    /* continue to perform (Xn) */

    /* (X1) level is set for all codes, embeddingLevel keeps track of the
     * push/pop operations */
    /* both variables may carry the LEVEL_OVERRIDE flag to indicate the override
     * status */
    uint8_t embeddingLevel = level, newLevel;
    uint8_t previousLevel =
        level;         /* previous level for regular (not CC) characters */
    int lastCcPos = 0; /* index of last effective LRx,RLx, PDx */

    /* The following stack remembers the embedding level and the ISOLATE flag of
       level runs. stackLast points to its current entry. */
    std::vector<short> stack = std::vector<short>(
        MAX_EXPLICIT_LEVEL + 2); /* we never push anything >= MAX_EXPLICIT_LEVEL
                but we need one more entry as base */
    int stackLast = 0;
    int overflowIsolateCount = 0;
    int overflowEmbeddingCount = 0;
    int validIsolateCount = 0;
    BracketData bracketData = BracketData();
    bracketInit(bracketData);
    stack[0] = level; /* initialize base entry to para level, no override, no
                         isolate */

    /* recalculate the flags */
    flags = 0;

    for (i = 0; i < length; i++) {
      dirProp = dirProps[i];
      switch (dirProp) {
        case LRE:
        case RLE:
        case LRO:
        case RLO:
          /* (X2, X3, X4, X5) */
          flags |= DirPropFlag(BN);
          levels[i] = previousLevel;
          if (dirProp == LRE || dirProp == LRO) /* least greater even level */
            newLevel = (uint8_t)((embeddingLevel + 2) & ~(LEVEL_OVERRIDE | 1));
          else
            /* least greater odd level */
            newLevel = (uint8_t)((NoOverride(embeddingLevel) + 1) | 1);
          if (newLevel <= MAX_EXPLICIT_LEVEL && overflowIsolateCount == 0 &&
              overflowEmbeddingCount == 0) {
            lastCcPos = i;
            embeddingLevel = newLevel;
            if (dirProp == LRO || dirProp == RLO)
              embeddingLevel |= LEVEL_OVERRIDE;
            stackLast++;
            stack[stackLast] = embeddingLevel;
            /* we don't need to set LEVEL_OVERRIDE off for LRE and RLE
               since this has already been done for newLevel which is
               the source for embeddingLevel.
             */
          } else {
            if (overflowIsolateCount == 0) overflowEmbeddingCount++;
          }
          break;
        case PDF:
          /* (X7) */
          flags |= DirPropFlag(BN);
          levels[i] = previousLevel;
          /* handle all the overflow cases first */
          if (overflowIsolateCount > 0) {
            break;
          }
          if (overflowEmbeddingCount > 0) {
            overflowEmbeddingCount--;
            break;
          }
          if (stackLast > 0 &&
              stack[stackLast] < ISOLATE) { /* not an isolate entry */
            lastCcPos = i;
            stackLast--;
            embeddingLevel = (uint8_t)stack[stackLast];
          }
          break;
        case LRI:
        case RLI:
          flags |= DirPropFlag(ON) | getDirPropFlagLR(embeddingLevel);
          levels[i] = NoOverride(embeddingLevel);
          if (NoOverride(embeddingLevel) != NoOverride(previousLevel)) {
            bracketProcessBoundary(bracketData, lastCcPos, previousLevel,
                                   embeddingLevel);
            flags |= DirPropFlagMultiRuns;
          }
          previousLevel = embeddingLevel;
          /* (X5a, X5b) */
          if (dirProp == LRI) /* least greater even level */
            newLevel = (short)((embeddingLevel + 2) & ~(LEVEL_OVERRIDE | 1));
          else
            /* least greater odd level */
            newLevel = (short)((NoOverride(embeddingLevel) + 1) | 1);
          if (newLevel <= MAX_EXPLICIT_LEVEL && overflowIsolateCount == 0 &&
              overflowEmbeddingCount == 0) {
            flags |= DirPropFlag(dirProp);
            lastCcPos = i;
            validIsolateCount++;
            if (validIsolateCount > isolateCount)
              isolateCount = validIsolateCount;
            embeddingLevel = newLevel;
            /* we can increment stackLast without checking because newLevel
               will exceed UBIDI_MAX_EXPLICIT_LEVEL before stackLast overflows
             */
            stackLast++;
            stack[stackLast] = (short)(embeddingLevel + ISOLATE);
            bracketProcessLRI_RLI(bracketData, embeddingLevel);
          } else {
            /* make it WS so that it is handled by adjustWSLevels() */
            dirProps[i] = WS;
            overflowIsolateCount++;
          }
          break;
        case PDI:
          if (NoOverride(embeddingLevel) != NoOverride(previousLevel)) {
            bracketProcessBoundary(bracketData, lastCcPos, previousLevel,
                                   embeddingLevel);
            flags |= DirPropFlagMultiRuns;
          }
          /* (X6a) */
          if (overflowIsolateCount > 0) {
            overflowIsolateCount--;
            /* make it WS so that it is handled by adjustWSLevels() */
            dirProps[i] = WS;
          } else if (validIsolateCount > 0) {
            flags |= DirPropFlag(PDI);
            lastCcPos = i;
            overflowEmbeddingCount = 0;
            while (stack[stackLast] < ISOLATE) /* pop embedding entries */
              stackLast--; /* until the last isolate entry */
            stackLast--;   /* pop also the last isolate entry */
            validIsolateCount--;
            bracketProcessPDI(bracketData);
          } else
            /* make it WS so that it is handled by adjustWSLevels() */
            dirProps[i] = WS;
          embeddingLevel = (short)(stack[stackLast] & ~ISOLATE);
          flags |= DirPropFlag(ON) | getDirPropFlagLR(embeddingLevel);
          previousLevel = embeddingLevel;
          levels[i] = NoOverride(embeddingLevel);
          break;
        case B:
          flags |= DirPropFlag(B);
          levels[i] = GetParaLevelAt(i);
          if ((i + 1) < length) {
            //                        if (text_utf32[i] == CR && text_utf32[i +
            //                        1] == LF)
            //                            break;          /* skip CR when
            //                            followed by LF */
            if (text_[i] == CR && text_[i + 1] == LF) {
              break;
            }
            overflowEmbeddingCount = overflowIsolateCount = 0;
            validIsolateCount = 0;
            stackLast = 0;
            previousLevel = embeddingLevel = GetParaLevelAt(i + 1);
            stack[0] = embeddingLevel; /* initialize base entry to para level,
                                          no override, no isolate */
            bracketProcessB(bracketData, embeddingLevel);
          }
          break;
        case BN:
          /* BN, LRE, RLE, and PDF are supposed to be removed (X9) */
          /* they will get their levels set correctly in adjustWSLevels() */
          levels[i] = previousLevel;
          flags |= DirPropFlag(BN);
          break;
        default:
          /* all other types are normal characters and get the "real" level */
          if (NoOverride(embeddingLevel) != NoOverride(previousLevel)) {
            bracketProcessBoundary(bracketData, lastCcPos, previousLevel,
                                   embeddingLevel);
            flags |= DirPropFlagMultiRuns;
            if ((embeddingLevel & LEVEL_OVERRIDE) != 0)
              flags |= getDirPropFlagO(embeddingLevel);
            else
              flags |= getDirPropFlagE(embeddingLevel);
          }
          previousLevel = embeddingLevel;
          levels[i] = embeddingLevel;
          bracketProcessChar(bracketData, i);
          /* the dirProp may have been changed in bracketProcessChar() */
          flags |= DirPropFlag(dirProps[i]);
          break;
      }
    }
    if ((flags & MASK_EMBEDDING) != 0) {
      flags |= getDirPropFlagLR(paraLevel);
    }
    if (orderParagraphsLTR && (flags & DirPropFlag(B)) != 0) {
      flags |= DirPropFlag(L);
    }
    /* again, determine if the text is mixed-directional or single-directional
     */
    dirct = directionFromFlags();

    return dirct;
  }

  short checkExplicitLevels() {
    int isolateCount = 0;

    flags = 0; /* collect all directionalities in the text */
    isolateCount = 0;

    int currentParaIndex = 0;
    int currentParaLimit = paras_limit[0];
    short currentParaLevel = paraLevel;

    for (int i = 0; i < length; ++i) {
      short level = levels[i];
      short dirProp = dirProps[i];
      if (dirProp == LRI || dirProp == RLI) {
        isolateCount++;
        if (isolateCount > this->isolateCount)
          this->isolateCount = isolateCount;
      } else if (dirProp == PDI)
        isolateCount--;
      else if (dirProp == B)
        isolateCount = 0;

      // optimized version of  byte currentParaLevel = GetParaLevelAt(i);
      if (defaultParaLevel != 0 && i == currentParaLimit &&
          (currentParaIndex + 1) < paraCount) {
        currentParaLevel = paras_level[++currentParaIndex];
        currentParaLimit = paras_limit[currentParaIndex];
      }

      int overrideFlag = level & LEVEL_OVERRIDE;
      level &= ~LEVEL_OVERRIDE;
      if (level < currentParaLevel || MAX_EXPLICIT_LEVEL < level) {
        if (level == 0) {
          if (dirProp == B) {
            // Paragraph separators are ok with explicit level 0.
            // Prevents reordering of paragraphs.
          } else {
            // Treat explicit level 0 as a wildcard for the paragraph level.
            // Avoid making the caller guess what the paragraph level would be.
            level = currentParaLevel;
            levels[i] = (short)(level | overrideFlag);
          }
        } else {
          // 1 <= level < currentParaLevel or MAX_EXPLICIT_LEVEL < level
          //                    throw new IllegalArgumentException("level " +
          //                    level +
          //                                                       " out of
          //                                                       bounds at " +
          //                                                       i);
        }
      }
      if (overrideFlag != 0) {
        /* keep the override flag in levels[i] but adjust the flags */
        flags |= getDirPropFlagO(level);
      } else {
        /* set the flags */
        flags |= getDirPropFlagE(level) | DirPropFlag(dirProp);
      }
    }
    if ((flags & MASK_EMBEDDING) != 0) flags |= getDirPropFlagLR(paraLevel);
    /* determine if the text is mixed-directional or single-directional */
    return directionFromFlags();
  }

  /*********************************************************************/
  /* The Properties state machine table                                */
  /*********************************************************************/
  /*                                                                   */
  /* All table cells are 8 bits:                                       */
  /*      bits 0..4:  next state                                       */
  /*      bits 5..7:  action to perform (if > 0)                       */
  /*                                                                   */
  /* Cells may be of format "n" where n represents the next state      */
  /* (except for the rightmost column).                                */
  /* Cells may also be of format "_(x,y)" where x represents an action */
  /* to perform and y represents the next state.                       */
  /*                                                                   */
  /*********************************************************************/
  /* Definitions and type for properties state tables                  */
  /*********************************************************************/
  static const int IMPTABPROPS_COLUMNS = 16;
  static const int IMPTABPROPS_RES = IMPTABPROPS_COLUMNS - 1;
  static short GetStateProps(short cell) { return (short)(cell & 0x1f); }

  static short GetActionProps(short cell) { return (short)(cell >> 5); }

  static const std::vector<short> groupProp;
  static const short _L = 0;
  static const short _R = 1;
  static const short _EN = 2;
  static const short _AN = 3;
  static const short _ON = 4;
  static const short _S = 5;
  static const short _B = 6; /* reduced dirProp */

  /*********************************************************************/
  /*                                                                   */
  /*      PROPERTIES  STATE  TABLE                                     */
  /*                                                                   */
  /* In table impTabProps,                                             */
  /*      - the ON column regroups ON and WS, FSI, RLI, LRI and PDI    */
  /*      - the BN column regroups BN, LRE, RLE, LRO, RLO, PDF         */
  /*      - the Res column is the reduced property assigned to a run   */
  /*                                                                   */
  /* Action 1: process current run1, init new run1                     */
  /*        2: init new run2                                           */
  /*        3: process run1, process run2, init new run1               */
  /*        4: process run1, set run1=run2, init new run2              */
  /*                                                                   */
  /* Notes:                                                            */
  /*  1) This table is used in resolveImplicitLevels().                */
  /*  2) This table triggers actions when there is a change in the Bidi*/
  /*     property of incoming characters (action 1).                   */
  /*  3) Most such property sequences are processed immediately (in    */
  /*     fact, passed to processPropertySeq().                         */
  /*  4) However, numbers are assembled as one sequence. This means    */
  /*     that undefined situations (like CS following digits, until    */
  /*     it is known if the next char will be a digit) are held until  */
  /*     following chars define them.                                  */
  /*     Example: digits followed by CS, then comes another CS or ON;  */
  /*              the digits will be processed, then the CS assigned   */
  /*              as the start of an ON sequence (action 3).           */
  /*  5) There are cases where more than one sequence must be          */
  /*     processed, for instance digits followed by CS followed by L:  */
  /*     the digits must be processed as one sequence, and the CS      */
  /*     must be processed as an ON sequence, all this before starting */
  /*     assembling chars for the opening L sequence.                  */
  /*                                                                   */
  /*                                                                   */
  static const std::vector<std::vector<short>> impTabProps;

  /*********************************************************************/
  /* The levels state machine tables                                   */
  /*********************************************************************/
  /*                                                                   */
  /* All table cells are 8 bits:                                       */
  /*      bits 0..3:  next state                                       */
  /*      bits 4..7:  action to perform (if > 0)                       */
  /*                                                                   */
  /* Cells may be of format "n" where n represents the next state      */
  /* (except for the rightmost column).                                */
  /* Cells may also be of format "_(x,y)" where x represents an action */
  /* to perform and y represents the next state.                       */
  /*                                                                   */
  /* This format limits each table to 16 states each and to 15 actions.*/
  /*                                                                   */
  /*********************************************************************/
  /* Definitions and type for levels state tables                      */
  /*********************************************************************/
  static const int IMPTABLEVELS_COLUMNS = _B + 2;
  static const int IMPTABLEVELS_RES = IMPTABLEVELS_COLUMNS - 1;
  static short GetState(short cell) { return (short)(cell & 0x0f); }
  static short GetAction(short cell) { return (short)(cell >> 4); }

  /*********************************************************************/
  /*                                                                   */
  /*      LEVELS  STATE  TABLES                                        */
  /*                                                                   */
  /* In all levels state tables,                                       */
  /*      - state 0 is the initial state                               */
  /*      - the Res column is the increment to add to the text level   */
  /*        for this property sequence.                                */
  /*                                                                   */
  /* The impact arrays for each table of a pair map the local action   */
  /* numbers of the table to the total list of actions. For instance,  */
  /* action 2 in a given table corresponds to the action number which  */
  /* appears in entry [2] of the impact array for that table.          */
  /* The first entry of all impact arrays must be 0.                   */
  /*                                                                   */
  /* Action 1: init conditional sequence                               */
  /*        2: prepend conditional sequence to current sequence        */
  /*        3: set ON sequence to new level - 1                        */
  /*        4: init EN/AN/ON sequence                                  */
  /*        5: fix EN/AN/ON sequence followed by R                     */
  /*        6: set previous level sequence to level 2                  */
  /*                                                                   */
  /* Notes:                                                            */
  /*  1) These tables are used in processPropertySeq(). The input      */
  /*     is property sequences as determined by resolveImplicitLevels. */
  /*  2) Most such property sequences are processed immediately        */
  /*     (levels are assigned).                                        */
  /*  3) However, some sequences cannot be assigned a final level till */
  /*     one or more following sequences are received. For instance,   */
  /*     ON following an R sequence within an even-level paragraph.    */
  /*     If the following sequence is R, the ON sequence will be       */
  /*     assigned basic run level+1, and so will the R sequence.       */
  /*  4) S is generally handled like ON, since its level will be fixed */
  /*     to paragraph level in adjustWSLevels().                       */
  /*                                                                   */
  static const std::vector<std::vector<short>> impTabL_DEFAULT;

  static const std::vector<std::vector<short>> impTabR_DEFAULT;

  static const std::vector<short> impAct0;
  static const ImpTabPair impTab_DEFAULT;
  static const std::vector<std::vector<short>> impTabL_NUMBERS_SPECIAL;

  static const ImpTabPair impTab_NUMBERS_SPECIAL;

  static const std::vector<std::vector<short>> impTabL_GROUP_NUMBERS_WITH_R;

  static const std::vector<std::vector<short>> impTabR_GROUP_NUMBERS_WITH_R;

  static const ImpTabPair impTab_GROUP_NUMBERS_WITH_R;

  static const std::vector<std::vector<short>> impTabL_INVERSE_NUMBERS_AS_L;

  static const std::vector<std::vector<short>> impTabR_INVERSE_NUMBERS_AS_L;

  static const ImpTabPair impTab_INVERSE_NUMBERS_AS_L;

  static const std::vector<std::vector<short>> impTabR_INVERSE_LIKE_DIRECT;
  static const std::vector<short> impAct1;
  static const ImpTabPair impTab_INVERSE_LIKE_DIRECT;
  static const std::vector<std::vector<short>>
      impTabL_INVERSE_LIKE_DIRECT_WITH_MARKS;
  static const std::vector<std::vector<short>>
      impTabR_INVERSE_LIKE_DIRECT_WITH_MARKS;

  static const std::vector<short> impAct2;
  static const std::vector<short> impAct3;
  static const ImpTabPair impTab_INVERSE_LIKE_DIRECT_WITH_MARKS;
  static const ImpTabPair impTab_INVERSE_FOR_NUMBERS_SPECIAL;
  static const std::vector<std::vector<short>>
      impTabL_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS;
  static const ImpTabPair impTab_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS;

  struct LevState {
    std::vector<std::vector<short>> impTab; /* level table pointer          */
    std::vector<short> impAct;              /* action map array             */
    int startON;                            /* start of ON sequence         */
    int startL2EN;                          /* start of level 2 sequence    */
    int lastStrongRTL;                      /* index of last found R or AL  */
    int runStart;                           /* start position of the run    */
    short state;                            /* current state                */
    short runLevel; /* run level before implicit solving */
  };

  /*------------------------------------------------------------------------*/

  static const int FIRSTALLOC = 10;
  /*
   *  param pos:     position where to insert
   *  param flag:    one of LRM_BEFORE, LRM_AFTER, RLM_BEFORE, RLM_AFTER
   */
  void addPoint(int pos, int flag) {
    Point point = Point();

    int len = insertPoints.points.size();
    if (len == 0) {
      insertPoints.points = std::vector<Point>(FIRSTALLOC);
      len = FIRSTALLOC;
    }
    if (insertPoints.size >= len) { /* no room for new point */
      std::vector<Point> savePoints = insertPoints.points;
      insertPoints.points = std::vector<Point>(len * 2);
      // System.arraycopy(savePoints, 0, insertPoints.points, 0, len);
      for (int i = 0; i < len; i++) {
        insertPoints.points[i] = savePoints[i];
      }
    }
    point.pos = pos;
    point.flag = flag;
    insertPoints.points[insertPoints.size] = point;
    insertPoints.size++;
  }

  void setLevelsOutsideIsolates(int start, int limit, short level) {
    short dirProp;
    int isolateCount = 0, k;
    for (k = start; k < limit; k++) {
      dirProp = dirProps[k];
      if (dirProp == PDI) isolateCount--;
      if (isolateCount == 0) levels[k] = level;
      if (dirProp == LRI || dirProp == RLI) isolateCount++;
    }
  }

  /* perform rules (Wn), (Nn), and (In) on a run of the text ------------------
   */

  /*
   * This implementation of the (Wn) rules applies all rules in one pass.
   * In order to do so, it needs a look-ahead of typically 1 character
   * (except for W5: sequences of ET) and keeps track of changes
   * in a rule Wp that affect a later Wq (p<q).
   *
   * The (Nn) and (In) rules are also performed in that same single loop,
   * but effectively one iteration behind for white space.
   *
   * Since all implicit rules are performed in one step, it is not necessary
   * to actually store the intermediate directional properties in dirProps[].
   */

  void processPropertySeq(LevState& levState, short _prop, int start,
                          int limit) {
    short cell;
    std::vector<std::vector<short>> impTab = levState.impTab;
    std::vector<short> impAct = levState.impAct;
    short oldStateSeq, actionSeq;
    short level, addLevel;
    int start0, k;

    start0 = start; /* save original start position */
    oldStateSeq = levState.state;
    cell = impTab[oldStateSeq][_prop];
    levState.state = GetState(cell);     /* isolate the new state */
    actionSeq = impAct[GetAction(cell)]; /* isolate the action */
    addLevel = impTab[levState.state][IMPTABLEVELS_RES];

    if (actionSeq != 0) {
      switch (actionSeq) {
        case 1: /* init ON seq */
          levState.startON = start0;
          break;

        case 2: /* prepend ON seq to current seq */
          start = levState.startON;
          break;

        case 3: /* EN/AN after R+ON */
          level = (short)(levState.runLevel + 1);
          setLevelsOutsideIsolates(levState.startON, start0, level);
          break;

        case 4: /* EN/AN before R for NUMBERS_SPECIAL */
          level = (short)(levState.runLevel + 2);
          setLevelsOutsideIsolates(levState.startON, start0, level);
          break;

        case 5: /* L or S after possible relevant EN/AN */
          /* check if we had EN after R/AL */
          if (levState.startL2EN >= 0) {
            addPoint(levState.startL2EN, LRM_BEFORE);
          }
          levState.startL2EN =
              -1; /* not within previous if since could also be -2 */
          /* check if we had any relevant EN/AN after R/AL */
          if ((insertPoints.points.size() == 0) ||
              (insertPoints.size <= insertPoints.confirmed)) {
            /* nothing, just clean up */
            levState.lastStrongRTL = -1;
            /* check if we have a pending conditional segment */
            level = impTab[oldStateSeq][IMPTABLEVELS_RES];
            if ((level & 1) != 0 && levState.startON > 0) { /* after ON */
              start = levState.startON; /* reset to basic run level */
            }
            if (_prop == _S) { /* add LRM before S */
              addPoint(start0, LRM_BEFORE);
              insertPoints.confirmed = insertPoints.size;
            }
            break;
          }
          /* reset previous RTL cont to level for LTR text */
          for (k = levState.lastStrongRTL + 1; k < start0; k++) {
            /* reset odd level, leave runLevel+2 as is */
            levels[k] = (short)((levels[k] - 2) & ~1);
          }
          /* mark insert points as confirmed */
          insertPoints.confirmed = insertPoints.size;
          levState.lastStrongRTL = -1;
          if (_prop == _S) { /* add LRM before S */
            addPoint(start0, LRM_BEFORE);
            insertPoints.confirmed = insertPoints.size;
          }
          break;

        case 6: /* R/AL after possible relevant EN/AN */
          /* just clean up */
          if (insertPoints.points.size() > 0)
            /* remove all non confirmed insert points */
            insertPoints.size = insertPoints.confirmed;
          levState.startON = -1;
          levState.startL2EN = -1;
          levState.lastStrongRTL = limit - 1;
          break;

        case 7: /* EN/AN after R/AL + possible cont */
          /* check for real AN */
          if ((_prop == _AN) && (dirProps[start0] == AN) &&
              (reorderingMode != REORDER_INVERSE_FOR_NUMBERS_SPECIAL)) {
            /* real AN */
            if (levState.startL2EN ==
                -1) { /* if no relevant EN already found */
              /* just note the rightmost digit as a strong RTL */
              levState.lastStrongRTL = limit - 1;
              break;
            }
            if (levState.startL2EN >= 0) { /* after EN, no AN */
              addPoint(levState.startL2EN, LRM_BEFORE);
              levState.startL2EN = -2;
            }
            /* note AN */
            addPoint(start0, LRM_BEFORE);
            break;
          }
          /* if first EN/AN after R/AL */
          if (levState.startL2EN == -1) {
            levState.startL2EN = start0;
          }
          break;

        case 8: /* note location of latest R/AL */
          levState.lastStrongRTL = limit - 1;
          levState.startON = -1;
          break;

        case 9: /* L after R+ON/EN/AN */
          /* include possible adjacent number on the left */
          for (k = start0 - 1; k >= 0 && ((levels[k] & 1) == 0); k--) {
          }
          if (k >= 0) {
            addPoint(k, RLM_BEFORE);                    /* add RLM before */
            insertPoints.confirmed = insertPoints.size; /* confirm it */
          }
          levState.startON = start0;
          break;

        case 10: /* AN after L */
          /* AN numbers between L text on both sides may be trouble. */
          /* tentatively bracket with LRMs; will be confirmed if followed by L
           */
          addPoint(start0, LRM_BEFORE); /* add LRM before */
          addPoint(start0, LRM_AFTER);  /* add LRM after  */
          break;

        case 11: /* R after L+ON/EN/AN */
          /* false alert, infirm LRMs around previous AN */
          insertPoints.size = insertPoints.confirmed;
          if (_prop == _S) { /* add RLM before S */
            addPoint(start0, RLM_BEFORE);
            insertPoints.confirmed = insertPoints.size;
          }
          break;

        case 12: /* L after L+ON/AN */
          level = (short)(levState.runLevel + addLevel);
          for (k = levState.startON; k < start0; k++) {
            if (levels[k] < level) {
              levels[k] = level;
            }
          }
          insertPoints.confirmed = insertPoints.size; /* confirm inserts */
          levState.startON = start0;
          break;

        case 13: /* L after L+ON+EN/AN/ON */
          level = levState.runLevel;
          for (k = start0 - 1; k >= levState.startON; k--) {
            if (levels[k] == level + 3) {
              while (levels[k] == level + 3) {
                levels[k--] -= 2;
              }
              while (levels[k] == level) {
                k--;
              }
            }
            if (levels[k] == level + 2) {
              levels[k] = level;
              continue;
            }
            levels[k] = (short)(level + 1);
          }
          break;

        case 14: /* R after L+ON+EN/AN/ON */
          level = (short)(levState.runLevel + 1);
          for (k = start0 - 1; k >= levState.startON; k--) {
            if (levels[k] > level) {
              levels[k] -= 2;
            }
          }
          break;

        default: /* we should never get here */
          // throw new IllegalStateException("Internal ICU error in
          // processPropertySeq");
          break;
      }
    }
    if ((addLevel) != 0 || (start < start0)) {
      level = (short)(levState.runLevel + addLevel);
      if (start >= levState.runStart) {
        for (k = start; k < limit; k++) {
          levels[k] = level;
        }
      } else {
        setLevelsOutsideIsolates(start, limit, level);
      }
    }
  }

  /**
   * Returns the directionality of the last strong character at the end of the
   * prologue, if any. Requires prologue!=null.
   */
  short lastL_R_AL() {
    for (int i = prologue.length(); i > 0;) {
      // int uchar = prologue.codePointBefore(i);//TODO
      int uchar =
          prologue[i];  // TODO: temporarily run, but the interface is
                        // different, but this place actually does not use it
      i -= Character::charCount(uchar);
      short dirProp = (short)getCustomizedClass(uchar);
      if (dirProp == L) {
        return _L;
      }
      if (dirProp == R || dirProp == AL) {
        return _R;
      }
      if (dirProp == B) {
        return _ON;
      }
    }
    return _ON;
  }

  /**
   * Returns the directionality of the first strong character, or digit, in the
   * epilogue, if any. Requires epilogue!=null.
   */
  short firstL_R_AL_EN_AN() {
    for (size_t i = 0; i < epilogue.length();) {
      // int uchar = epilogue.codePointAt(i);//TODO
      int uchar = epilogue[i];
      i += Character::charCount(uchar);
      short dirProp = (short)getCustomizedClass(uchar);
      if (dirProp == L) {
        return _L;
      }
      if (dirProp == R || dirProp == AL) {
        return _R;
      }
      if (dirProp == EN) {
        return _EN;
      }
      if (dirProp == AN) {
        return _AN;
      }
    }
    return _ON;
  }

  void resolveImplicitLevels(int start, int limit, short sor, short eor) {
    short dirProp;
    LevState levState = LevState();
    int i, start1, start2;
    short oldStateImp, stateImp, actionImp;
    short gprop, resProp, cell;
    bool inverseRTL;
    short nextStrongProp = R;
    int nextStrongPos = -1;

    /* check for RTL inverse Bidi mode */
    /* FOOD FOR THOUGHT: in case of RTL inverse Bidi, it would make sense to
     * loop on the text characters from end to start.
     * This would need a different properties state table (at least different
     * actions) and different levels state tables (maybe very similar to the
     * LTR corresponding ones.
     */
    inverseRTL =
        ((start < lastArabicPos) && ((GetParaLevelAt(start) & 1) > 0) &&
         (reorderingMode == REORDER_INVERSE_LIKE_DIRECT ||
          reorderingMode == REORDER_INVERSE_FOR_NUMBERS_SPECIAL));
    /* initialize for property and levels state table */
    levState.startL2EN = -1;     /* used for INVERSE_LIKE_DIRECT_WITH_MARKS */
    levState.lastStrongRTL = -1; /* used for INVERSE_LIKE_DIRECT_WITH_MARKS */
    levState.runStart = start;
    levState.runLevel = levels[start];
    levState.impTab = impTabPair.imptab[levState.runLevel & 1];
    levState.impAct = impTabPair.impact[levState.runLevel & 1];
    if (start == 0 &&
        prologue.length()) {  // TODO: start == 0 && prologue != null
      short lastStrong = lastL_R_AL();
      if (lastStrong != _ON) {
        sor = lastStrong;
      }
    }
    /* The isolates[] entries contain enough information to
       resume the bidi algorithm in the same state as it was
       when it was interrupted by an isolate sequence. */
    if (dirProps[start] == PDI) {
      levState.startON = isolates[isolateCount].startON;
      start1 = isolates[isolateCount].start1;
      stateImp = isolates[isolateCount].stateImp;
      levState.state = isolates[isolateCount].state;
      isolateCount--;
    } else {
      levState.startON = -1;
      start1 = start;
      if (dirProps[start] == NSM)
        stateImp = (short)(1 + sor);
      else
        stateImp = 0;
      levState.state = 0;
      processPropertySeq(levState, sor, start, start);
    }
    start2 = start; /* to make the Java compiler happy */

    for (i = start; i <= limit; i++) {
      if (i >= limit) {
        int k;
        for (k = limit - 1;
             k > start && (DirPropFlag(dirProps[k]) & MASK_BN_EXPLICIT) != 0;
             k--)
          ;
        dirProp = dirProps[k];
        if (dirProp == LRI || dirProp == RLI)
          break; /* no forced closing for sequence ending with LRI/RLI */
        gprop = eor;
      } else {
        short prop, prop1;
        prop = dirProps[i];
        if (prop == B)
          isolateCount = -1; /* current isolates stack entry == none */
        if (inverseRTL) {
          if (prop == AL) {
            /* AL before EN does not make it AN */
            prop = R;
          } else if (prop == EN) {
            if (nextStrongPos <= i) {
              /* look for next strong char (L/R/AL) */
              int j;
              nextStrongProp = R; /* set default */
              nextStrongPos = limit;
              for (j = i + 1; j < limit; j++) {
                prop1 = dirProps[j];
                if (prop1 == L || prop1 == R || prop1 == AL) {
                  nextStrongProp = prop1;
                  nextStrongPos = j;
                  break;
                }
              }
            }
            if (nextStrongProp == AL) {
              prop = AN;
            }
          }
        }
        gprop = groupProp[prop];
      }
      oldStateImp = stateImp;
      cell = impTabProps[oldStateImp][gprop];
      stateImp = GetStateProps(cell);   /* isolate the new state */
      actionImp = GetActionProps(cell); /* isolate the action */
      if ((i == limit) && (actionImp == 0)) {
        /* there is an unprocessed sequence if its property == eor   */
        actionImp = 1; /* process the last sequence */
      }
      if (actionImp != 0) {
        resProp = impTabProps[oldStateImp][IMPTABPROPS_RES];
        switch (actionImp) {
          case 1: /* process current seq1, init new seq1 */
            processPropertySeq(levState, resProp, start1, i);
            start1 = i;
            break;
          case 2: /* init new seq2 */
            start2 = i;
            break;
          case 3: /* process seq1, process seq2, init new seq1 */
            processPropertySeq(levState, resProp, start1, start2);
            processPropertySeq(levState, _ON, start2, i);
            start1 = i;
            break;
          case 4: /* process seq1, set seq1=seq2, init new seq2 */
            processPropertySeq(levState, resProp, start1, start2);
            start1 = start2;
            start2 = i;
            break;
          default: /* we should never get here */
            // throw new IllegalStateException("Internal ICU error in
            // resolveImplicitLevels");
            break;
        }
      }
    }

    /* flush possible pending sequence, e.g. ON */
    if (limit == length &&
        epilogue.length()) {  // TODO: limit == length && epilogue != null
      short firstStrong = firstL_R_AL_EN_AN();
      if (firstStrong != _ON) {
        eor = firstStrong;
      }
    }

    /* look for the last char not a BN or LRE/RLE/LRO/RLO/PDF */
    for (i = limit - 1;
         i > start && (DirPropFlag(dirProps[i]) & MASK_BN_EXPLICIT) != 0; i--)
      ;
    dirProp = dirProps[i];
    if ((dirProp == LRI || dirProp == RLI) && limit < length) {
      isolateCount++;
      // if (isolates[isolateCount] == null)//TODO
      isolates[isolateCount] = Isolate();
      isolates[isolateCount].stateImp = stateImp;
      isolates[isolateCount].state = levState.state;
      isolates[isolateCount].start1 = start1;
      isolates[isolateCount].startON = levState.startON;
    } else
      processPropertySeq(levState, eor, limit, limit);
  }

  /* perform (L1) and (X9) ----------------------------------------------------
   */

  /*
   * Reset the embedding levels for some non-graphic characters (L1).
   * This method also sets appropriate levels for BN, and
   * explicit embedding types that are supposed to have been removed
   * from the paragraph in (X9).
   */
  void adjustWSLevels() {
    int i;

    if ((flags & MASK_WS) != 0) {
      int flag;
      i = trailingWSStart;
      while (i > 0) {
        /* reset a sequence of WS/BN before eop and B/S to the paragraph
         * paraLevel */
        while (i > 0 && ((flag = DirPropFlag(dirProps[--i])) & MASK_WS) != 0) {
          if (orderParagraphsLTR && (flag & DirPropFlag(B)) != 0) {
            levels[i] = 0;
          } else {
            levels[i] = GetParaLevelAt(i);
          }
        }

        /* reset BN to the next character's paraLevel until B/S, which restarts
         * above loop */
        /* here, i+1 is guaranteed to be <length */
        while (i > 0) {
          flag = DirPropFlag(dirProps[--i]);
          if ((flag & MASK_BN_EXPLICIT) != 0) {
            levels[i] = levels[i + 1];
          } else if (orderParagraphsLTR && (flag & DirPropFlag(B)) != 0) {
            levels[i] = 0;
            break;
          } else if ((flag & MASK_B_S) != 0) {
            levels[i] = GetParaLevelAt(i);
            break;
          }
        }
      }
    }
  }

  void setContext(std::string prologue, std::string epilogue) {
    prologue = prologue.length() > 0 ? prologue
                                     : nullptr;  // TODO: can string be nullptr?
    epilogue = epilogue.length() > 0 ? epilogue : nullptr;
  }

  void setParaSuccess() {
    prologue = ""; /* forget the last context */
    epilogue = "";
    // paraBidi = this;                /* mark successful setPara */
  }

  int Bidi_Min(int x, int y) { return x < y ? x : y; }

  int Bidi_Abs(int x) { return x >= 0 ? x : -x; }

  void setParaRunsOnly(const std::u32string& parmText, short parmParaLevel);

  void setPara(const std::u32string& text, short paraLevel,
               std::vector<short> embeddingLevels) {
    if (text.length() == 0) return;
    setPara(text.c_str(), paraLevel, embeddingLevels);
  }

  void setPara(const char32_t* chars, short paraLevel,
               std::vector<short> embeddingLevels) {
    /* check the argument values */
    if (paraLevel < LEVEL_DEFAULT_LTR) {
      verifyRange(paraLevel, 0, MAX_EXPLICIT_LEVEL + 1);
    }
    if (chars == nullptr) {
      return;
    }

    /* special treatment for RUNS_ONLY mode */
    if (reorderingMode == REORDER_RUNS_ONLY) {
      setParaRunsOnly(chars, paraLevel);
      return;
    }

    /* initialize the Bidi object */
    // this.paraBidi = null;          /* mark unfinished setPara */
    text_ = std::u32string(chars);
    auto len = text_.length();
    length = originalLength = resultLength = len;
    this->paraLevel = paraLevel;
    direction = (short)(paraLevel & 1);
    paraCount = 1;

    /* Allocate zero-length arrays instead of setting to null here; then
     * checks for null in various places can be eliminated.
     */
    dirProps = std::vector<short>();  // TODO: cannot ensure the length of
                                      // vector
    levels = std::vector<short>();
    runs = std::vector<BidiRun>();
    isGoodLogicalToVisualRunsMap = false;
    insertPoints.size = 0;      /* clean up from last call */
    insertPoints.confirmed = 0; /* clean up from last call */

    /*
     * Save the original paraLevel if contextual; otherwise, set to 0.
     */
    defaultParaLevel = IsDefaultLevel(paraLevel) ? paraLevel : 0;

    if (length == 0) {
      /*
       * For an empty paragraph, create a Bidi object with the paraLevel and
       * the flags and the direction set but without allocating zero-length
       * arrays. There is nothing more to do.
       */
      if (IsDefaultLevel(paraLevel)) {
        paraLevel &= 1;
        defaultParaLevel = 0;
      }
      flags = getDirPropFlagLR(paraLevel);
      runCount = 0;
      paraCount = 0;
      setParaSuccess();
      return;
    }

    runCount = -1;

    /*
     * Get the directional properties,
     * the flags bit-set, and
     * determine the paragraph level if necessary.
     */
    // getDirPropsMemory(length);
    dirProps = std::vector<short>(length, 0);
    getDirProps();
    /* the processed length may have changed if OPTION_STREAMING is set */
    trailingWSStart = length; /* the levels[] will reflect the WS run */

    /* are explicit levels specified? */
    if (embeddingLevels.size() == 0) {
      /* no: determine explicit levels according to the (Xn) rules */
      // getLevelsMemory(length);
      // levels = levelsMemory;
      levels = std::vector<short>(length);
      direction = resolveExplicitLevels();
    } else {
      /* set BN for all explicit codes, check that all levels are 0 or
       * paraLevel..MAX_EXPLICIT_LEVEL */
      levels = embeddingLevels;
      direction = checkExplicitLevels();
    }

    /* allocate isolate memory */
    if (isolateCount > 0) {
      if (isolates.size() == 0 || (int)isolates.size() < isolateCount)
        isolates =
            std::vector<Isolate>(isolateCount + 3); /* keep some reserve */
    }
    isolateCount = -1; /* current isolates stack entry == none */

    /*
     * The steps after (X9) in the Bidi algorithm are performed only if
     * the paragraph text has mixed directionality!
     */
    switch (direction) {
      case LTR:
        /* all levels are implicitly at paraLevel (important for getLevels()) */
        trailingWSStart = 0;
        break;
      case RTL:
        /* all levels are implicitly at paraLevel (important for getLevels()) */
        trailingWSStart = 0;
        break;
      default:
        /*
         *  Choose the right implicit state table
         */
        switch (reorderingMode) {
          case REORDER_DEFAULT:
            impTabPair = impTab_DEFAULT;
            break;
          case REORDER_NUMBERS_SPECIAL:
            impTabPair = impTab_NUMBERS_SPECIAL;
            break;
          case REORDER_GROUP_NUMBERS_WITH_R:
            impTabPair = impTab_GROUP_NUMBERS_WITH_R;
            break;
          case REORDER_RUNS_ONLY:
            /* we should never get here */
            // throw new InternalError("Internal ICU error in setPara");
            break;
            /* break; */
          case REORDER_INVERSE_NUMBERS_AS_L:
            impTabPair = impTab_INVERSE_NUMBERS_AS_L;
            break;
          case REORDER_INVERSE_LIKE_DIRECT:
            if ((reorderingOptions & OPTION_INSERT_MARKS) != 0) {
              impTabPair = impTab_INVERSE_LIKE_DIRECT_WITH_MARKS;
            } else {
              impTabPair = impTab_INVERSE_LIKE_DIRECT;
            }
            break;
          case REORDER_INVERSE_FOR_NUMBERS_SPECIAL:
            if ((reorderingOptions & OPTION_INSERT_MARKS) != 0) {
              impTabPair = impTab_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS;
            } else {
              impTabPair = impTab_INVERSE_FOR_NUMBERS_SPECIAL;
            }
            break;
        }
        /*
         * If there are no external levels specified and there
         * are no significant explicit level codes in the text,
         * then we can treat the entire paragraph as one run.
         * Otherwise, we need to perform the following rules on runs of
         * the text with the same embedding levels. (X10)
         * "Significant" explicit level codes are ones that actually
         * affect non-BN characters.
         * Examples for "insignificant" ones are empty embeddings
         * LRE-PDF, LRE-RLE-PDF-PDF, etc.
         */
        if (embeddingLevels.size() == 0 && paraCount <= 1 &&
            (flags & DirPropFlagMultiRuns) == 0) {
          resolveImplicitLevels(0, length, GetLRFromLevel(GetParaLevelAt(0)),
                                GetLRFromLevel(GetParaLevelAt(length - 1)));
        } else {
          /* sor, eor: start and end types of same-level-run */
          int start, limit = 0;
          short level, nextLevel;
          short sor, eor;

          /* determine the first sor and set eor to it because of the loop body
           * (sor=eor there) */
          level = GetParaLevelAt(0);
          nextLevel = levels[0];
          if (level < nextLevel) {
            eor = GetLRFromLevel(nextLevel);
          } else {
            eor = GetLRFromLevel(level);
          }

          do {
            /* determine start and limit of the run (end points just behind the
             * run) */

            /* the values for this run's start are the same as for the previous
             * run's end */
            start = limit;
            level = nextLevel;
            if ((start > 0) && (dirProps[start - 1] == B)) {
              /* except if this is a new paragraph, then set sor = para level */
              sor = GetLRFromLevel(GetParaLevelAt(start));
            } else {
              sor = eor;
            }

            /* search for the limit of this run */
            while ((++limit < length) &&
                   ((levels[limit] == level) ||
                    ((DirPropFlag(dirProps[limit]) & MASK_BN_EXPLICIT) != 0))) {
            }

            /* get the correct level of the next run */
            if (limit < length) {
              nextLevel = levels[limit];
            } else {
              nextLevel = GetParaLevelAt(length - 1);
            }

            /* determine eor from max(level, nextLevel); sor is last run's eor
             */
            if (NoOverride(level) < NoOverride(nextLevel)) {
              eor = GetLRFromLevel(nextLevel);
            } else {
              eor = GetLRFromLevel(level);
            }

            /* if the run consists of overridden directional types, then there
               are no implicit types to be resolved */
            if ((level & LEVEL_OVERRIDE) == 0) {
              resolveImplicitLevels(start, limit, sor, eor);
            } else {
              /* remove the LEVEL_OVERRIDE flags */
              do {
                levels[start++] &= ~LEVEL_OVERRIDE;
              } while (start < limit);
            }
          } while (limit < length);
        }

        /* reset the embedding levels for some non-graphic characters (L1), (X9)
         */
        adjustWSLevels();

        break;
    }
    /* add RLM for inverse Bidi with contextual orientation resolving
     * to RTL which would not round-trip otherwise
     */
    if ((defaultParaLevel > 0) &&
        ((reorderingOptions & OPTION_INSERT_MARKS) != 0) &&
        ((reorderingMode == REORDER_INVERSE_LIKE_DIRECT) ||
         (reorderingMode == REORDER_INVERSE_FOR_NUMBERS_SPECIAL))) {
      int start, last;
      short level;
      short dirProp;
      for (int i = 0; i < paraCount; i++) {
        last = paras_limit[i] - 1;
        level = paras_level[i];
        if (level == 0) continue; /* LTR paragraph */
        start = i == 0 ? 0 : paras_limit[i - 1];
        for (int j = last; j >= start; j--) {
          dirProp = dirProps[j];
          if (dirProp == L) {
            if (j < last) {
              while (dirProps[last] == B) {
                last--;
              }
            }
            addPoint(last, RLM_BEFORE);
            break;
          }
          if ((DirPropFlag(dirProp) & MASK_R_AL) != 0) {
            break;
          }
        }
      }
    }

    if ((reorderingOptions & OPTION_REMOVE_CONTROLS) != 0) {
      resultLength -= controlCount;
    } else {
      resultLength += insertPoints.size;
    }
    setParaSuccess();
  }

  short getDirection() {
    // verifyValidParaOrLine();
    return direction;
  }

  int getCustomizedClass(int c) {
    int dir;
    dir = UCharacter::getDirection(c);
    if (dir >= UCharacterEnums::CHAR_DIRECTION_COUNT) dir = ON;
    return dir;
  }

  /**
   * Get the level for one character.
   *
   * @param charIndex the index of a character.
   *
   * @return The level for the character at <code>charIndex</code>.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   * @throws IllegalArgumentException if charIndex is not in the range
   *         <code>0&lt;=charIndex&lt;getProcessedLength()</code>
   *
   * @stable ICU 3.8
   */
  short getLevelAt(int charIndex);

  /**
   * Get an array of levels for each character.<p>
   *
   * Note that this method may allocate memory under some
   * circumstances, unlike <code>getLevelAt()</code>.
   *
   * @return The levels array for the text,
   *         or <code>null</code> if an error occurs.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   * @stable ICU 3.8
   */
  std::vector<short> getLevels();

  /**
   * Get a logical run.
   * This method returns information about a run and is used
   * to retrieve runs in logical order.<p>
   * This is especially useful for line-breaking on a paragraph.
   *
   * @param logicalPosition is a logical position within the source text.
   *
   * @return a BidiRun object filled with <code>start</code> containing
   *        the first character of the run, <code>limit</code> containing
   *        the limit of the run, and <code>embeddingLevel</code> containing
   *        the level of the run.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   * @throws IllegalArgumentException if logicalPosition is not in the range
   *         <code>0&lt;=logicalPosition&lt;getProcessedLength()</code>
   *
   * @see BidiRun
   * @see BidiRun#getStart()
   * @see BidiRun#getLimit()
   * @see BidiRun#getEmbeddingLevel()
   *
   * @stable ICU 3.8
   */
  BidiRun getLogicalRun(int logicalPosition);

  /**
   * Get the number of runs.
   * This method may invoke the actual reordering on the
   * <code>Bidi</code> object, after <code>setPara()</code>
   * may have resolved only the levels of the text. Therefore,
   * <code>countRuns()</code> may have to allocate memory,
   * and may throw an exception if it fails to do so.
   *
   * @return The number of runs.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   * @stable ICU 3.8
   */
  int countRuns();

  BidiRun getVisualRun(int runIndex);

  std::vector<uint32_t> getLogicalMap();

  std::vector<uint32_t> getVisualMap();
  /*
   * Fields and methods for compatibility with java.text.bidi (Sun
   * implementation)
   */

  /**
   * Constant indicating base direction is left-to-right.
   * @stable ICU 3.8
   */
  static const int DIRECTION_LEFT_TO_RIGHT = LTR;

  /**
   * Constant indicating base direction is right-to-left.
   * @stable ICU 3.8
   */
  static const int DIRECTION_RIGHT_TO_LEFT = RTL;

  /**
   * Constant indicating that the base direction depends on the first strong
   * directional character in the text according to the Unicode Bidirectional
   * Algorithm. If no strong directional character is present, the base
   * direction is left-to-right.
   * @stable ICU 3.8
   */
  static const int DIRECTION_DEFAULT_LEFT_TO_RIGHT = LEVEL_DEFAULT_LTR;

  /**
   * Constant indicating that the base direction depends on the first strong
   * directional character in the text according to the Unicode Bidirectional
   * Algorithm. If no strong directional character is present, the base
   * direction is right-to-left.
   * @stable ICU 3.8
   */
  static const int DIRECTION_DEFAULT_RIGHT_TO_LEFT = LEVEL_DEFAULT_RTL;

  /**
   * Return true if the line is not left-to-right or right-to-left. This means
   * it either has mixed runs of left-to-right and right-to-left text, or the
   * base direction differs from the direction of the only run of text.
   *
   * @return true if the line is not left-to-right or right-to-left.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code>
   * @stable ICU 3.8
   */
  bool isMixed() { return (!isLeftToRight() && !isRightToLeft()); }

  /**
   * Return true if the line is all left-to-right text and the base direction
   * is left-to-right.
   *
   * @return true if the line is all left-to-right text and the base direction
   *         is left-to-right.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code>
   * @stable ICU 3.8
   */
  bool isLeftToRight() {
    return (getDirection() == LTR && (paraLevel & 1) == 0);
  }

  /**
   * Return true if the line is all right-to-left text, and the base direction
   * is right-to-left
   *
   * @return true if the line is all right-to-left text, and the base
   *         direction is right-to-left
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code>
   * @stable ICU 3.8
   */
  bool isRightToLeft() {
    return (getDirection() == RTL && (paraLevel & 1) == 1);
  }

  /**
   * Return the number of level runs.
   *
   * @return the number of level runs
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   *
   * @stable ICU 3.8
   */
  int getRunCount() { return countRuns(); }

  /**
   * Take a <code>Bidi</code> object containing the reordering
   * information for a piece of text (one or more paragraphs) set by
   * <code>setPara()</code> or for a line of text set by <code>setLine()</code>
   * and return a string containing the reordered text.
   *
   * <p>The text may have been aliased (only a reference was stored
   * without copying the contents), thus it must not have been modified
   * since the <code>setPara()</code> call.
   *
   * This method preserves the integrity of characters with multiple
   * code units and (optionally) combining characters.
   * Characters in RTL runs can be replaced by mirror-image characters
   * in the returned string. Note that "real" mirroring has to be done in a
   * rendering engine by glyph selection and that for many "mirrored"
   * characters there are no Unicode characters as mirror-image equivalents.
   * There are also options to insert or remove Bidi control
   * characters; see the descriptions of the return value and the
   * <code>options</code> parameter, and of the option bit flags.
   *
   * @param options A bit set of options for the reordering that control
   *                how the reordered text is written.
   *                The options include mirroring the characters on a code
   *                point basis and inserting LRM characters, which is used
   *                especially for transforming visually stored text
   *                to logically stored text (although this is still an
   *                imperfect implementation of an "inverse Bidi" algorithm
   *                because it uses the "forward Bidi" algorithm at its core).
   *                The available options are:
   *                <code>DO_MIRRORING</code>,
   *                <code>INSERT_LRM_FOR_NUMERIC</code>,
   *                <code>KEEP_BASE_COMBINING</code>,
   *                <code>OUTPUT_REVERSE</code>,
   *                <code>REMOVE_BIDI_CONTROLS</code>,
   *                <code>STREAMING</code>
   *
   * @return The reordered text.
   *         If the <code>INSERT_LRM_FOR_NUMERIC</code> option is set, then
   *         the length of the returned string could be as large as
   *         <code>getLength()+2*countRuns()</code>.<br>
   *         If the <code>REMOVE_BIDI_CONTROLS</code> option is set, then the
   *         length of the returned string may be less than
   *         <code>getLength()</code>.<br>
   *         If none of these options is set, then the length of the returned
   *         string will be exactly <code>getProcessedLength()</code>.
   *
   * @throws IllegalStateException if this call is not preceded by a successful
   *         call to <code>setPara</code> or <code>setLine</code>
   *
   * @see #DO_MIRRORING
   * @see #INSERT_LRM_FOR_NUMERIC
   * @see #KEEP_BASE_COMBINING
   * @see #OUTPUT_REVERSE
   * @see #REMOVE_BIDI_CONTROLS
   * @see #OPTION_STREAMING
   * @stable ICU 3.8
   */
  std::u32string writeReordered(int options) {
    // verifyValidParaOrLine();
    if (length == 0) {
      /* nothing to do */
      return U"";
    }
    return BidiWriter::writeReordered(this, options);
  }
};

#endif  // BIDI_BIDI_H
