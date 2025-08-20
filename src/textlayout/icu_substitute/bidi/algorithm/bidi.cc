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

#include "bidi.h"

#include "bidi_line.h"

const int Bidi::DirPropFlagMultiRuns = Bidi::DirPropFlag((short)31);
const std::vector<int> Bidi::DirPropFlagLR = {Bidi::DirPropFlag(Bidi::L),
                                              Bidi::DirPropFlag(Bidi::R)};
const std::vector<int> Bidi::DirPropFlagE = {Bidi::DirPropFlag(Bidi::LRE),
                                             Bidi::DirPropFlag(Bidi::RLE)};
const std::vector<int> Bidi::DirPropFlagO = {Bidi::DirPropFlag(Bidi::LRO),
                                             Bidi::DirPropFlag(Bidi::RLO)};
const int Bidi::MASK_LTR =
    Bidi::DirPropFlag(Bidi::L) | Bidi::DirPropFlag(Bidi::EN) |
    Bidi::DirPropFlag(Bidi::ENL) | Bidi::DirPropFlag(Bidi::ENR) |
    Bidi::DirPropFlag(Bidi::AN) | Bidi::DirPropFlag(Bidi::LRE) |
    Bidi::DirPropFlag(Bidi::LRO) | Bidi::DirPropFlag(Bidi::LRI);
const int Bidi::MASK_RTL = Bidi::DirPropFlag(R) | Bidi::DirPropFlag(AL) |
                           Bidi::DirPropFlag(RLE) | Bidi::DirPropFlag(RLO) |
                           Bidi::DirPropFlag(RLI);
const int Bidi::MASK_R_AL = Bidi::DirPropFlag(R) | Bidi::DirPropFlag(AL);
const int Bidi::MASK_STRONG_EN_AN =
    Bidi::DirPropFlag(L) | Bidi::DirPropFlag(R) | Bidi::DirPropFlag(AL) |
    Bidi::DirPropFlag(EN) | Bidi::DirPropFlag(AN);
const int Bidi::MASK_EXPLICIT =
    Bidi::DirPropFlag(LRE) | Bidi::DirPropFlag(LRO) | Bidi::DirPropFlag(RLE) |
    Bidi::DirPropFlag(RLO) | Bidi::DirPropFlag(PDF);
const int Bidi::MASK_BN_EXPLICIT = Bidi::DirPropFlag(BN) | Bidi::MASK_EXPLICIT;
const int Bidi::MASK_ISO = Bidi::DirPropFlag(LRI) | Bidi::DirPropFlag(RLI) |
                           Bidi::DirPropFlag(FSI) | Bidi::DirPropFlag(PDI);
const int Bidi::MASK_B_S = Bidi::DirPropFlag(B) | Bidi::DirPropFlag(S);
const int Bidi::MASK_WS =
    MASK_B_S | Bidi::DirPropFlag(WS) | MASK_BN_EXPLICIT | MASK_ISO;
const int Bidi::MASK_POSSIBLE_N =
    Bidi::DirPropFlag(ON) | Bidi::DirPropFlag(CS) | Bidi::DirPropFlag(ES) |
    Bidi::DirPropFlag(ET) | MASK_WS;
const int Bidi::MASK_EMBEDDING = Bidi::DirPropFlag(NSM) | MASK_POSSIBLE_N;
const std::vector<short> Bidi::groupProp = /* dirProp regrouped */
    {
        /*  L   R   EN  ES  ET  AN  CS  B   S   WS  ON  LRE LRO AL  RLE RLO PDF
           NSM BN  FSI LRI RLI PDI ENL ENR */
        0,  1,  2,  7,  8,  3,  9, 6, 5, 4, 4,  10, 10,
        12, 10, 10, 10, 11, 10, 4, 4, 4, 4, 13, 14};

const std::vector<std::vector<short>> Bidi::impTabProps = {
    /*                        L,     R,    EN,    AN,    ON,     S,     B, ES,
       ET,    CS,    BN,   NSM,    AL,   ENL,   ENR,   Res */
    /* 0 Init        */ {1, 2, 4, 5, 7, 15, 17, 7, 9, 7, 0, 7, 3, 18, 21, _ON},
    /* 1 L           */
    {1, 32 + 2, 32 + 4, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 32 + 9,
     32 + 7, 1, 1, 32 + 3, 32 + 18, 32 + 21, _L},
    /* 2 R           */
    {32 + 1, 2, 32 + 4, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 32 + 9,
     32 + 7, 2, 2, 32 + 3, 32 + 18, 32 + 21, _R},
    /* 3 AL          */
    {32 + 1, 32 + 2, 32 + 6, 32 + 6, 32 + 8, 32 + 16, 32 + 17, 32 + 8, 32 + 8,
     32 + 8, 3, 3, 3, 32 + 18, 32 + 21, _R},
    /* 4 EN          */
    {32 + 1, 32 + 2, 4, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 64 + 10, 11, 64 + 10,
     4, 4, 32 + 3, 18, 21, _EN},
    /* 5 AN          */
    {32 + 1, 32 + 2, 32 + 4, 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 32 + 9,
     64 + 12, 5, 5, 32 + 3, 32 + 18, 32 + 21, _AN},
    /* 6 AL:EN/AN    */
    {32 + 1, 32 + 2, 6, 6, 32 + 8, 32 + 16, 32 + 17, 32 + 8, 32 + 8, 64 + 13, 6,
     6, 32 + 3, 18, 21, _AN},
    /* 7 ON          */
    {32 + 1, 32 + 2, 32 + 4, 32 + 5, 7, 32 + 15, 32 + 17, 7, 64 + 14, 7, 7, 7,
     32 + 3, 32 + 18, 32 + 21, _ON},
    /* 8 AL:ON       */
    {32 + 1, 32 + 2, 32 + 6, 32 + 6, 8, 32 + 16, 32 + 17, 8, 8, 8, 8, 8, 32 + 3,
     32 + 18, 32 + 21, _ON},
    /* 9 ET          */
    {32 + 1, 32 + 2, 4, 32 + 5, 7, 32 + 15, 32 + 17, 7, 9, 7, 9, 9, 32 + 3, 18,
     21, _ON},
    /*10 EN+ES/CS    */
    {96 + 1, 96 + 2, 4, 96 + 5, 128 + 7, 96 + 15, 96 + 17, 128 + 7, 128 + 14,
     128 + 7, 10, 128 + 7, 96 + 3, 18, 21, _EN},
    /*11 EN+ET       */
    {32 + 1, 32 + 2, 4, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 11, 32 + 7,
     11, 11, 32 + 3, 18, 21, _EN},
    /*12 AN+CS       */
    {96 + 1, 96 + 2, 96 + 4, 5, 128 + 7, 96 + 15, 96 + 17, 128 + 7, 128 + 14,
     128 + 7, 12, 128 + 7, 96 + 3, 96 + 18, 96 + 21, _AN},
    /*13 AL:EN/AN+CS */
    {96 + 1, 96 + 2, 6, 6, 128 + 8, 96 + 16, 96 + 17, 128 + 8, 128 + 8, 128 + 8,
     13, 128 + 8, 96 + 3, 18, 21, _AN},
    /*14 ON+ET       */
    {32 + 1, 32 + 2, 128 + 4, 32 + 5, 7, 32 + 15, 32 + 17, 7, 14, 7, 14, 14,
     32 + 3, 128 + 18, 128 + 21, _ON},
    /*15 S           */
    {32 + 1, 32 + 2, 32 + 4, 32 + 5, 32 + 7, 15, 32 + 17, 32 + 7, 32 + 9,
     32 + 7, 15, 32 + 7, 32 + 3, 32 + 18, 32 + 21, _S},
    /*16 AL:S        */
    {32 + 1, 32 + 2, 32 + 6, 32 + 6, 32 + 8, 16, 32 + 17, 32 + 8, 32 + 8,
     32 + 8, 16, 32 + 8, 32 + 3, 32 + 18, 32 + 21, _S},
    /*17 B           */
    {32 + 1, 32 + 2, 32 + 4, 32 + 5, 32 + 7, 32 + 15, 17, 32 + 7, 32 + 9,
     32 + 7, 17, 32 + 7, 32 + 3, 32 + 18, 32 + 21, _B},
    /*18 ENL         */
    {32 + 1, 32 + 2, 18, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 64 + 19, 20, 64 + 19,
     18, 18, 32 + 3, 18, 21, _L},
    /*19 ENL+ES/CS   */
    {96 + 1, 96 + 2, 18, 96 + 5, 128 + 7, 96 + 15, 96 + 17, 128 + 7, 128 + 14,
     128 + 7, 19, 128 + 7, 96 + 3, 18, 21, _L},
    /*20 ENL+ET      */
    {32 + 1, 32 + 2, 18, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 20, 32 + 7,
     20, 20, 32 + 3, 18, 21, _L},
    /*21 ENR         */
    {32 + 1, 32 + 2, 21, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 64 + 22, 23, 64 + 22,
     21, 21, 32 + 3, 18, 21, _AN},
    /*22 ENR+ES/CS   */
    {96 + 1, 96 + 2, 21, 96 + 5, 128 + 7, 96 + 15, 96 + 17, 128 + 7, 128 + 14,
     128 + 7, 22, 128 + 7, 96 + 3, 18, 21, _AN},
    /*23 ENR+ET      */
    {32 + 1, 32 + 2, 21, 32 + 5, 32 + 7, 32 + 15, 32 + 17, 32 + 7, 23, 32 + 7,
     23, 23, 32 + 3, 18, 21, _AN}};

const std::vector<std::vector<short>>
    Bidi::impTabL_DEFAULT = /* Even paragraph level */
    /*  In this table, conditional sequences receive the lower possible level
        until proven otherwise.
    */
    {
        /*                         L,     R,    EN,    AN,    ON,     S,     B,
           Res */
        /* 0 : init       */ {0, 1, 0, 2, 0, 0, 0, 0},
        /* 1 : R          */ {0, 1, 3, 3, 0x14, 0x14, 0, 1},
        /* 2 : AN         */ {0, 1, 0, 2, 0x15, 0x15, 0, 2},
        /* 3 : R+EN/AN    */ {0, 1, 3, 3, 0x14, 0x14, 0, 2},
        /* 4 : R+ON       */ {0, 0x21, 0x33, 0x33, 4, 4, 0, 0},
        /* 5 : AN+ON      */ {0, 0x21, 0, 0x32, 5, 5, 0, 0}};

const std::vector<std::vector<short>>
    Bidi::impTabR_DEFAULT = /* Odd  paragraph level */
    /*  In this table, conditional sequences receive the lower possible level
        until proven otherwise.
    */
    {
        /*                         L,     R,    EN,    AN,    ON,     S,     B,
           Res */
        /* 0 : init       */ {1, 0, 2, 2, 0, 0, 0, 0},
        /* 1 : L          */ {1, 0, 1, 3, 0x14, 0x14, 0, 1},
        /* 2 : EN/AN      */ {1, 0, 2, 2, 0, 0, 0, 1},
        /* 3 : L+AN       */ {1, 0, 1, 3, 5, 5, 0, 1},
        /* 4 : L+ON       */ {0x21, 0, 0x21, 3, 4, 4, 0, 0},
        /* 5 : L+AN+ON    */ {1, 0, 1, 3, 5, 5, 0, 0}};

const std::vector<short> Bidi::impAct0 = {0, 1, 2, 3, 4};
const Bidi::ImpTabPair Bidi::impTab_DEFAULT =
    Bidi::ImpTabPair(impTabL_DEFAULT, impTabR_DEFAULT, impAct0, impAct0);
const std::vector<std::vector<short>> Bidi::impTabL_NUMBERS_SPECIAL =
    {/* Even paragraph level */
     /* In this table, conditional sequences receive the lower possible
        level until proven otherwise.
     */
     /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
      */
     /* 0 : init       */ {0, 2, 0x11, 0x11, 0, 0, 0, 0},
     /* 1 : L+EN/AN    */ {0, 0x42, 1, 1, 0, 0, 0, 0},
     /* 2 : R          */ {0, 2, 4, 4, 0x13, 0x13, 0, 1},
     /* 3 : R+ON       */ {0, 0x22, 0x34, 0x34, 3, 3, 0, 0},
     /* 4 : R+EN/AN    */ {0, 2, 4, 4, 0x13, 0x13, 0, 2}};

const Bidi::ImpTabPair Bidi::impTab_NUMBERS_SPECIAL = Bidi::ImpTabPair(
    impTabL_NUMBERS_SPECIAL, impTabR_DEFAULT, impAct0, impAct0);

const std::vector<std::vector<short>> Bidi::impTabL_GROUP_NUMBERS_WITH_R = {
    /* In this table, EN/AN+ON sequences receive levels as if associated with R
       until proven that there is L or sor/eor on both sides. AN is handled like
       EN.
    */
    /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
     */
    /* 0 init         */ {0, 3, 0x11, 0x11, 0, 0, 0, 0},
    /* 1 EN/AN        */ {0x20, 3, 1, 1, 2, 0x20, 0x20, 2},
    /* 2 EN/AN+ON     */ {0x20, 3, 1, 1, 2, 0x20, 0x20, 1},
    /* 3 R            */ {0, 3, 5, 5, 0x14, 0, 0, 1},
    /* 4 R+ON         */ {0x20, 3, 5, 5, 4, 0x20, 0x20, 1},
    /* 5 R+EN/AN      */ {0, 3, 5, 5, 0x14, 0, 0, 2}};

const std::vector<std::vector<short>> Bidi::impTabR_GROUP_NUMBERS_WITH_R = {
    /*  In this table, EN/AN+ON sequences receive levels as if associated with R
        until proven that there is L on both sides. AN is handled like EN.
    */
    /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
     */
    /* 0 init         */ {2, 0, 1, 1, 0, 0, 0, 0},
    /* 1 EN/AN        */ {2, 0, 1, 1, 0, 0, 0, 1},
    /* 2 L            */ {2, 0, 0x14, 0x14, 0x13, 0, 0, 1},
    /* 3 L+ON         */ {0x22, 0, 4, 4, 3, 0, 0, 0},
    /* 4 L+EN/AN      */ {0x22, 0, 4, 4, 3, 0, 0, 1}};
const Bidi::ImpTabPair Bidi::impTab_GROUP_NUMBERS_WITH_R =
    Bidi::ImpTabPair(impTabL_GROUP_NUMBERS_WITH_R, impTabR_GROUP_NUMBERS_WITH_R,
                     impAct0, impAct0);
const std::vector<std::vector<short>> Bidi::impTabL_INVERSE_NUMBERS_AS_L = {
    /* This table is identical to the Default LTR table except that EN and AN
       are handled like L.
    */
    /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
     */
    /* 0 : init       */ {0, 1, 0, 0, 0, 0, 0, 0},
    /* 1 : R          */ {0, 1, 0, 0, 0x14, 0x14, 0, 1},
    /* 2 : AN         */ {0, 1, 0, 0, 0x15, 0x15, 0, 2},
    /* 3 : R+EN/AN    */ {0, 1, 0, 0, 0x14, 0x14, 0, 2},
    /* 4 : R+ON       */ {0x20, 1, 0x20, 0x20, 4, 4, 0x20, 1},
    /* 5 : AN+ON      */ {0x20, 1, 0x20, 0x20, 5, 5, 0x20, 1}};
const std::vector<std::vector<short>> Bidi::impTabR_INVERSE_NUMBERS_AS_L = {
    /* This table is identical to the Default RTL table except that EN and AN
       are handled like L.
    */
    /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
     */
    /* 0 : init       */ {1, 0, 1, 1, 0, 0, 0, 0},
    /* 1 : L          */ {1, 0, 1, 1, 0x14, 0x14, 0, 1},
    /* 2 : EN/AN      */ {1, 0, 1, 1, 0, 0, 0, 1},
    /* 3 : L+AN       */ {1, 0, 1, 1, 5, 5, 0, 1},
    /* 4 : L+ON       */ {0x21, 0, 0x21, 0x21, 4, 4, 0, 0},
    /* 5 : L+AN+ON    */ {1, 0, 1, 1, 5, 5, 0, 0}};
const Bidi::ImpTabPair Bidi::impTab_INVERSE_NUMBERS_AS_L =
    Bidi::ImpTabPair(impTabL_INVERSE_NUMBERS_AS_L, impTabR_INVERSE_NUMBERS_AS_L,
                     impAct0, impAct0);
const std::vector<std::vector<short>> Bidi::impTabR_INVERSE_LIKE_DIRECT =
    {/* Odd  paragraph level */
     /*  In this table, conditional sequences receive the lower possible level
         until proven otherwise.
     */
     /*                         L,     R,    EN,    AN,    ON,     S,     B, Res
      */
     /* 0 : init       */ {1, 0, 2, 2, 0, 0, 0, 0},
     /* 1 : L          */ {1, 0, 1, 2, 0x13, 0x13, 0, 1},
     /* 2 : EN/AN      */ {1, 0, 2, 2, 0, 0, 0, 1},
     /* 3 : L+ON       */ {0x21, 0x30, 6, 4, 3, 3, 0x30, 0},
     /* 4 : L+ON+AN    */ {0x21, 0x30, 6, 4, 5, 5, 0x30, 3},
     /* 5 : L+AN+ON    */ {0x21, 0x30, 6, 4, 5, 5, 0x30, 2},
     /* 6 : L+ON+EN    */ {0x21, 0x30, 6, 4, 3, 3, 0x30, 1}};
const std::vector<short> Bidi::impAct1 = {0, 1, 13, 14};
const Bidi::ImpTabPair Bidi::impTab_INVERSE_LIKE_DIRECT = Bidi::ImpTabPair(
    impTabL_DEFAULT, impTabR_INVERSE_LIKE_DIRECT, impAct0, impAct1);

const std::vector<std::vector<short>>
    Bidi::impTabL_INVERSE_LIKE_DIRECT_WITH_MARKS = {
        /* The case handled in this table is (visually):  R EN L
         */
        /*                         L,     R,    EN,    AN,    ON,     S,     B,
           Res */
        /* 0 : init       */ {0, 0x63, 0, 1, 0, 0, 0, 0},
        /* 1 : L+AN       */ {0, 0x63, 0, 1, 0x12, 0x30, 0, 4},
        /* 2 : L+AN+ON    */ {0x20, 0x63, 0x20, 1, 2, 0x30, 0x20, 3},
        /* 3 : R          */ {0, 0x63, 0x55, 0x56, 0x14, 0x30, 0, 3},
        /* 4 : R+ON       */ {0x30, 0x43, 0x55, 0x56, 4, 0x30, 0x30, 3},
        /* 5 : R+EN       */ {0x30, 0x43, 5, 0x56, 0x14, 0x30, 0x30, 4},
        /* 6 : R+AN       */ {0x30, 0x43, 0x55, 6, 0x14, 0x30, 0x30, 4}};
const std::vector<std::vector<short>>
    Bidi::impTabR_INVERSE_LIKE_DIRECT_WITH_MARKS = {
        /* The cases handled in this table are (visually):  R EN L
                                                            R L AN L
        */
        /*                         L,     R,    EN,    AN,    ON,     S,     B,
           Res */
        /* 0 : init       */ {0x13, 0, 1, 1, 0, 0, 0, 0},
        /* 1 : R+EN/AN    */ {0x23, 0, 1, 1, 2, 0x40, 0, 1},
        /* 2 : R+EN/AN+ON */ {0x23, 0, 1, 1, 2, 0x40, 0, 0},
        /* 3 : L          */ {3, 0, 3, 0x36, 0x14, 0x40, 0, 1},
        /* 4 : L+ON       */ {0x53, 0x40, 5, 0x36, 4, 0x40, 0x40, 0},
        /* 5 : L+ON+EN    */ {0x53, 0x40, 5, 0x36, 4, 0x40, 0x40, 1},
        /* 6 : L+AN       */ {0x53, 0x40, 6, 6, 4, 0x40, 0x40, 3}};
const std::vector<short> Bidi::impAct2 = {0, 1, 2, 5, 6, 7, 8};
const std::vector<short> Bidi::impAct3 = {0, 1, 9, 10, 11, 12};
const Bidi::ImpTabPair Bidi::impTab_INVERSE_LIKE_DIRECT_WITH_MARKS =
    Bidi::ImpTabPair(impTabL_INVERSE_LIKE_DIRECT_WITH_MARKS,
                     impTabR_INVERSE_LIKE_DIRECT_WITH_MARKS, impAct2, impAct3);

const Bidi::ImpTabPair Bidi::impTab_INVERSE_FOR_NUMBERS_SPECIAL =
    Bidi::ImpTabPair(impTabL_NUMBERS_SPECIAL, impTabR_INVERSE_LIKE_DIRECT,
                     impAct0, impAct1);

const std::vector<std::vector<short>>
    Bidi::impTabL_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS = {
        /*  The case handled in this table is (visually):  R EN L
         */
        /*                         L,     R,    EN,    AN,    ON,     S,     B,
           Res */
        /* 0 : init       */ {0, 0x62, 1, 1, 0, 0, 0, 0},
        /* 1 : L+EN/AN    */ {0, 0x62, 1, 1, 0, 0x30, 0, 4},
        /* 2 : R          */ {0, 0x62, 0x54, 0x54, 0x13, 0x30, 0, 3},
        /* 3 : R+ON       */ {0x30, 0x42, 0x54, 0x54, 3, 0x30, 0x30, 3},
        /* 4 : R+EN/AN    */ {0x30, 0x42, 4, 4, 0x13, 0x30, 0x30, 4}};
const Bidi::ImpTabPair Bidi::impTab_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS =
    Bidi::ImpTabPair(impTabL_INVERSE_FOR_NUMBERS_SPECIAL_WITH_MARKS,
                     impTabR_INVERSE_LIKE_DIRECT_WITH_MARKS, impAct2, impAct3);

void Bidi::setParaRunsOnly(const std::u32string& parmText,
                           short parmParaLevel) {
  std::vector<uint32_t> visualMap;
  int saveLength, saveTrailingWSStart;
  std::vector<short> saveLevels;
  short saveDirection;
  int i, j, visualStart, logicalStart, oldRunCount, runLength, addedRuns,
      insertRemove, start, limit, step, indexOddBit, logicalPos, index, index1;
  int saveOptions;

  reorderingMode = REORDER_DEFAULT;
  int parmLength = parmText.length();
  if (parmLength == 0) {
    setPara(parmText, parmParaLevel, std::vector<short>());
    reorderingMode = REORDER_RUNS_ONLY;
    return;
  }
  /* obtain memory for mapping table and visual text */
  saveOptions = reorderingOptions;
  if ((saveOptions & OPTION_INSERT_MARKS) > 0) {
    reorderingOptions &= ~OPTION_INSERT_MARKS;
    reorderingOptions |= OPTION_REMOVE_CONTROLS;
  }
  parmParaLevel &= 1; /* accept only 0 or 1 */
  setPara(parmText, parmParaLevel, std::vector<short>());
  /* we cannot access directly levels since it is not yet set if
   * direction is not MIXED
   */
  saveLevels = std::vector<short>(length);
  // System.arraycopy(getLevels(), 0, saveLevels, 0, length);
  auto lev = getLevels();
  for (int i = 0; i < length; i++) {
    saveLevels[i] = lev[i];
  }
  saveTrailingWSStart = trailingWSStart;

  /* FOOD FOR THOUGHT: instead of writing the visual text, we could use
   * the visual map and the dirProps array to drive the second call
   * to setPara (but must make provision for possible removal of
   * Bidi controls.  Alternatively, only use the dirProps array via
   * customized classifier callback.
   */
  auto visualText = writeReordered(DO_MIRRORING);
  visualMap = getVisualMap();
  reorderingOptions = saveOptions;
  saveLength = length;
  saveDirection = direction;

  reorderingMode = REORDER_INVERSE_LIKE_DIRECT;
  parmParaLevel ^= 1;
  setPara(visualText, parmParaLevel, std::vector<short>());
  BidiLine::getRuns(this);
  /* check if some runs must be split, count how many splits */
  addedRuns = 0;
  oldRunCount = runCount;
  visualStart = 0;
  for (i = 0; i < oldRunCount; i++, visualStart += runLength) {
    runLength = runs[i].limit - visualStart;
    if (runLength < 2) {
      continue;
    }
    logicalStart = runs[i].start;
    for (j = logicalStart + 1; j < logicalStart + runLength; j++) {
      index = visualMap[j];
      index1 = visualMap[j - 1];
      if ((Bidi_Abs(index - index1) != 1) ||
          (saveLevels[index] != saveLevels[index1])) {
        addedRuns++;
      }
    }
  }
  if (addedRuns > 0) {
    // getRunsMemory(oldRunCount + addedRuns);
    auto runsMemory = std::vector<BidiRun>((oldRunCount + addedRuns));
    if (runCount == 1) {
      /* because we switch from UBiDi.simpleRuns to UBiDi.runs */
      runsMemory[0] = runs[0];
    } else {
      // System.arraycopy(runs, 0, runsMemory, 0, runCount);
      for (int i = 0; i < runCount; i++) {
        runsMemory[i] = runs[i];
      }
    }
    runs = runsMemory;
    runCount += addedRuns;
    for (i = oldRunCount; i < runCount; i++) {
      // if (runs[i] == nullptr) {//TODO
      runs[i] = BidiRun(0, 0, (short)0);
      //}
    }
  }
  /* split runs which are not consecutive in source text */
  int newI;
  for (i = oldRunCount - 1; i >= 0; i--) {
    newI = i + addedRuns;
    runLength = i == 0 ? runs[0].limit : runs[i].limit - runs[i - 1].limit;
    logicalStart = runs[i].start;
    indexOddBit = runs[i].level & 1;
    if (runLength < 2) {
      if (addedRuns > 0) {
        runs[newI].copyFrom(runs[i]);
      }
      logicalPos = visualMap[logicalStart];
      runs[newI].start = logicalPos;
      runs[newI].level = (short)(saveLevels[logicalPos] ^ indexOddBit);
      continue;
    }
    if (indexOddBit > 0) {
      start = logicalStart;
      limit = logicalStart + runLength - 1;
      step = 1;
    } else {
      start = logicalStart + runLength - 1;
      limit = logicalStart;
      step = -1;
    }
    for (j = start; j != limit; j += step) {
      index = visualMap[j];
      index1 = visualMap[j + step];
      if ((Bidi_Abs(index - index1) != 1) ||
          (saveLevels[index] != saveLevels[index1])) {
        logicalPos = Bidi_Min(visualMap[start], index);
        runs[newI].start = logicalPos;
        runs[newI].level = (short)(saveLevels[logicalPos] ^ indexOddBit);
        runs[newI].limit = runs[i].limit;
        runs[i].limit -= Bidi_Abs(j - start) + 1;
        insertRemove = runs[i].insertRemove & (LRM_AFTER | RLM_AFTER);
        runs[newI].insertRemove = insertRemove;
        runs[i].insertRemove &= ~insertRemove;
        start = j + step;
        addedRuns--;
        newI--;
      }
    }
    if (addedRuns > 0) {
      runs[newI].copyFrom(runs[i]);
    }
    logicalPos = Bidi_Min(visualMap[start], visualMap[limit]);
    runs[newI].start = logicalPos;
    runs[newI].level = (short)(saveLevels[logicalPos] ^ indexOddBit);
  }

  //    cleanup1:
  /* restore initial paraLevel */
  paraLevel ^= 1;
  //    cleanup2:
  /* restore real text */
  // text = (char*)parmText.c_str();
  text_ = parmText;
  // text_utf32 = UTFCnv::to_utf32(text);
  length = saveLength;
  originalLength = parmLength;
  direction = saveDirection;
  levels = saveLevels;
  trailingWSStart = saveTrailingWSStart;
  if (runCount > 1) {
    direction = MIXED;
  }
  //    cleanup3:
  reorderingMode = REORDER_RUNS_ONLY;
}

short Bidi::getLevelAt(int charIndex) {
  // verifyValidParaOrLine();
  verifyRange(charIndex, 0, length);
  return BidiLine::getLevelAt(this, charIndex);
}

std::vector<short> Bidi::getLevels() {
  // verifyValidParaOrLine();
  return BidiLine::getLevels(this);
}

BidiRun Bidi::getLogicalRun(int logicalPosition) {
  // verifyValidParaOrLine();
  verifyRange(logicalPosition, 0, length);
  return BidiLine::getLogicalRun(this, logicalPosition);
}

int Bidi::countRuns() {
  // verifyValidParaOrLine();
  BidiLine::getRuns(this);
  return runCount;
}

BidiRun Bidi::getVisualRun(int runIndex) {
  // verifyValidParaOrLine();
  BidiLine::getRuns(this);
  verifyRange(runIndex, 0, runCount);
  return BidiLine::getVisualRun(this, runIndex);
}

std::vector<uint32_t> Bidi::getLogicalMap() {
  /* countRuns() checks successful call to setPara/setLine */
  countRuns();
  if (length <= 0) {
    return std::vector<uint32_t>();
  }
  return BidiLine::getLogicalMap(this);
}

std::vector<uint32_t> Bidi::getVisualMap() {
  /* countRuns() checks successful call to setPara/setLine */
  countRuns();
  if (resultLength <= 0) {
    return std::vector<uint32_t>();
  }
  return BidiLine::getVisualMap(this);
}
