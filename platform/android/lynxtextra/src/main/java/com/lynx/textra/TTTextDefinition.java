// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

public class TTTextDefinition {
  public static class Size {
    public Size(float w, float h) {
      width = w;
      height = h;
    }

    public float width;
    public float height;
  }

  public static class LayoutPosition {
    public LayoutPosition() {}

    public LayoutPosition(int run_index, int char_index) {
      run_index_ = run_index;
      char_index_ = char_index;
    }

    public int run_index_ = 0;
    public int char_index_ = 0;
  }

  public static class FontFace {
    public String font_family;
    public String[] resource;
  }

  public enum LayoutResult {
    kNormal,
    kRelayoutPage,
    kRelayoutLine,
    kBreakPage,
    kBreakColumn,
    kParagraphEnd
  }

  public enum FontWeight {
    kUndefined,
    kThin_100,
    kExtraLight_200,
    kLight_300,
    kNormal_400,
    kMedium_500,
    kSemiBold_600,
    kBold_700,
    kExtraBold_800,
    kBlack_900,
  }

  public enum CharacterVerticalAlign {
    kBaseLine,
    kSuperScript,
    kSubScript,
    kTop,
    kBottom,
    kMiddle,
  }

  public enum ParagraphHorizontalAlign {
    kLeft,
    kCenter,
    kRight,
    kJustify,
    kDistributed,
  }

  public enum ParagraphVerticalAlign { kTop, kCenter, kBaseline, kBottom }

  public enum DecorationType { kNone, kUnderLine }

  public enum Slant { kUpRightSlant, kItalicSlant, kObliqueSlant }

  public enum LinkStyle { kNone, kUnderline }

  public enum ThemeColorType { kNormal, kLink, kBackground, kBlock, kFootnote, kPressedLink }

  public enum CanvasOp {
    kStartPaint,
    kEndPaint,
    kSave,
    kRestore,
    kTranslate,
    kScale,
    kRotate,
    kSkew,
    kClipRect,
    kClear,
    kClearRect,
    kFillRect,
    kDrawColor,
    kDrawLine,
    kDrawRect,
    kDrawOval,
    kDrawCircle,
    kDrawArc,
    kDrawPath,
    kDrawArcTo,
    kDrawText,
    kDrawGlyphs,
    kDrawRunDelegate,
    kDrawImage,
    kDrawImageRect,
    kDrawBackgroundDelegate,
    kDrawBlockRegion,
    kDrawRoundRect,
  }

  public enum PathType { kLines, kArc, kBezier, kMoveTo, kMultiPath }

  public static FontWeight GetFontWeight(int ordinal) {
    if (ordinal < 0 || ordinal >= weight_ordinal_.length)
      return FontWeight.kNormal_400;
    return weight_ordinal_[ordinal];
  }

  //    public static FontStyle GetFontStyle(int ordinal) {
  //        if (ordinal < 0 || ordinal >= style_ordinal_.length)
  //            return FontStyle.kNormal;
  //        return style_ordinal_[ordinal];
  //    }

  public static LinkStyle GetLinkStyle(int ordinal) {
    if (ordinal < 0 || ordinal >= link_ordinal_.length)
      return LinkStyle.kNone;
    return link_ordinal_[ordinal];
  }

  public static ThemeColorType GetThemeColorType(int ordinal) {
    if (ordinal < 0 || ordinal >= theme_color_ordinal.length)
      return ThemeColorType.kNormal;
    return theme_color_ordinal[ordinal];
  }

  public static CanvasOp GetCanvasOp(int ordinal) {
    return canvas_ordinal_[ordinal];
  }

  public static PathType GetPathType(int ordinal) {
    return path_ordinal_[ordinal];
  }

  public static CharacterVerticalAlign GetCharacterVerticalAlign(int ordinal) {
    return character_vertical_ordinal_[ordinal];
  }

  public static ParagraphHorizontalAlign GetParagraphHorizontalAlign(int ordinal) {
    return paragraph_horizon_ordinal_[ordinal];
  }

  public static ParagraphVerticalAlign GetParagraphVerticalAlign(int ordinal) {
    return paragraph_vertical_ordinal_[ordinal];
  }

  public static LayoutResult GetLayoutResult(int ordinal) {
    return layout_result_ordinal_[ordinal];
  }

  private static final FontWeight[] weight_ordinal_ = FontWeight.values();
  //    private static final FontStyle[] style_ordinal_ = FontStyle.values();
  private static final LinkStyle[] link_ordinal_ = LinkStyle.values();
  private static final ThemeColorType[] theme_color_ordinal = ThemeColorType.values();
  private static final CanvasOp[] canvas_ordinal_ = CanvasOp.values();
  private static final PathType[] path_ordinal_ = PathType.values();
  private static final CharacterVerticalAlign[] character_vertical_ordinal_ =
      CharacterVerticalAlign.values();
  private static final ParagraphHorizontalAlign[] paragraph_horizon_ordinal_ =
      ParagraphHorizontalAlign.values();
  private static final ParagraphVerticalAlign[] paragraph_vertical_ordinal_ =
      ParagraphVerticalAlign.values();
  private static final LayoutResult[] layout_result_ordinal_ = LayoutResult.values();
}
