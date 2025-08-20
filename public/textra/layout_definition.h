// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

/**
 * @brief A centralized header to define enum classes used across text styles,
 * paragraph styles, and layout process.
 *
 * The defined enum classes include:
 * - Text styling (decoration types, colors, font attributes)
 * - Paragraph formatting (alignment, tab stops, line breaks)
 * - Layout process (run types, boundary types, overflow wrap)
 * - Directional properties (writing direction, alignment)
 */

#ifndef PUBLIC_TEXTRA_LAYOUT_DEFINITION_H_
#define PUBLIC_TEXTRA_LAYOUT_DEFINITION_H_

#include <textra/macro.h>

#include <cstdint>

namespace ttoffice {
namespace tttext {
using AttrType = uint32_t;
constexpr float LAYOUT_MAX_UNITS = 10e5;
constexpr float LAYOUT_MIN_UNITS = -10e5;
// #ifdef TTTEXT_DEBUG
// #define CHECK_ATTRTYPE
// #endif
enum class AttributeType : AttrType {
  kLayoutAttrStart = 0,
  kFontDescriptor = kLayoutAttrStart,
  kTextSize,
  kTextScale,
  kVerticalAlignment,
  kWordSpacing,
  kLetterSpacing,
  kLayoutAttrEnd,
  kStyleManagerAttrStart = kLayoutAttrEnd,
  kForegroundColor = kStyleManagerAttrStart,
  kBackgroundColor,
  kDecorationColor,
  kDecorationType,
  kDecorationStyle,
  //  kDecorationMode,
  kDecorationThicknessMultiplier,
  kBold,
  kItalic,
  kTextShadowList,
  kForegroundPainter,
  kBackgroundPainter,
  kWordBreak,
  kBaselineOffset,
  kStyleManagerAttrEnd,
  kMaxAttrType = kStyleManagerAttrEnd,
  kExtraAttrStart = 128,
  kExtraBaselineOffset
};

enum class LayoutResult : uint8_t {
  kNormal,
  kRelayoutPage,
  kRelayoutLine,
  kBreakPage,
  kBreakColumn,
  kBreakLine,
};
enum class ParagraphHorizontalAlignment : uint8_t {
  kLeft,
  kCenter,
  kRight,
  kJustify,
  kDistributed,
};
enum class ParagraphVerticalAlignment : uint8_t {
  kTop,
  kCenter,
  // kBaseline,
  kBottom
};
enum class CharacterVerticalAlignment : uint8_t {
  kTop,
  kSuperScript,
  kMiddle,
  kBaseLine,
  kSubScript,
  kBottom,
};
enum class ScriptType : uint8_t { kNormal, kSuperScript, kSubScript };
enum class TabLeaderType : uint8_t {
  kNone,
  kDot,
  kHeavy,
  kHyphen,
  kMiddleDot,
  kUnderScore
};
enum class TabStopType : uint8_t {
  kClear,
  kStart,
  kBar,
  kCenter,
  kDecimal,
  kEnd,
  kNum
};
enum class RulerType : uint8_t { kAtLeast, kAuto, kExact };
enum class LineBreakType : uint8_t {
  kNotDefined,
  kNoBreak,
  kAllowBreak,
  kBreakLine,
  kBreakColumn,
  kBreakPage,
};
enum class WordBreakType : uint8_t {
  kNormal,
  kBreakAll,
  kKeepAll,
};
enum class OverflowWrap : uint8_t { kNormal, kAnywhere, kBreakWord };
enum class RunType : uint8_t {
  kTextRun = 0,
  kGhostRun = 1,
  kSpaceRun = 2,
  kTabRun = 3,  // tab \t
  kInlineObject = 1 << 2,
  kFloatObject = kInlineObject + 1,
  kBlockStart = 1 << 3,
  kBlockEnd = kBlockStart + 1,
  kControlRun = 1 << 7,
  kCRRun = kControlRun + 2,    // \n
  kFFRun = kControlRun + 3,    // \f
  kLFRun = kControlRun + 4,    // \r
  kNLFRun = kControlRun + 5,   // \u2028
  kCRLFRun = kControlRun + 6,  // \r\n
};
enum class WriteDirection : uint8_t {
  kAuto = 0,
  kLTR = 1,
  kRTL = 2,
  kTTB = 3,
  kBTT = 4,
};
enum class LayoutMode : uint8_t { kIndefinite, kDefinite, kAtMost };
enum class ThemeCategory : uint8_t {
  kNormal = 0,
  kLink = 1,
  kBackground = 2,
  kBorder = 3,
  kFootnote = 4
};
enum class DecorationType : uint8_t {
  kNone = 0x0,
  kUnderLine = 0x1,
  kOverline = 0x2,
  kLineThrough = 0x4,
};
enum class DecorationMode { kGaps, kThrough };
enum class ColorType : uint8_t { kUndefined, kARGB, kTheme };
enum class LineType : uint8_t {
  kNone,
  kHidden,
  kSolid,
  kDotted,
  kDashed,
  kDouble,
  kInset,
  kOutset,
  kGroove,
  kRidge,
  kWavy
};
enum class FillType : uint8_t { kNone, kSolid };
enum class BoundaryType : uint8_t {
  kNone,
  kGraphme,
  kWord,
  kLineBreakable,
  kMustLineBreak,
  kParagraph
};
enum LineBreakStrategy : uint8_t {
  kAvoidBreakAroundPunctuation = 1,
  kLineBreakStrategyDefault = 0xff,
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_LAYOUT_DEFINITION_H_
