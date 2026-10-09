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

  public static final class LayoutResult {
    public static final int NORMAL = 0;
    public static final int RELAYOUT_PAGE = 1;
    public static final int RELAYOUT_LINE = 2;
    public static final int BREAK_PAGE = 3;
    public static final int BREAK_COLUMN = 4;
    public static final int PARAGRAPH_END = 5;

    private LayoutResult() {}
  }

  public static final class FontWeight {
    public static final int UNDEFINED = 0;
    public static final int THIN_100 = 1;
    public static final int EXTRA_LIGHT_200 = 2;
    public static final int LIGHT_300 = 3;
    public static final int NORMAL_400 = 4;
    public static final int MEDIUM_500 = 5;
    public static final int SEMI_BOLD_600 = 6;
    public static final int BOLD_700 = 7;
    public static final int EXTRA_BOLD_800 = 8;
    public static final int BLACK_900 = 9;

    private FontWeight() {}
  }

  public static final class CharacterVerticalAlign {
    public static final int BASE_LINE = 0;
    public static final int SUPER_SCRIPT = 1;
    public static final int SUB_SCRIPT = 2;
    public static final int TOP = 3;
    public static final int BOTTOM = 4;
    public static final int MIDDLE = 5;

    private CharacterVerticalAlign() {}
  }

  public static final class ParagraphHorizontalAlign {
    public static final int LEFT = 0;
    public static final int CENTER = 1;
    public static final int RIGHT = 2;
    public static final int JUSTIFY = 3;
    public static final int DISTRIBUTED = 4;

    private ParagraphHorizontalAlign() {}
  }

  public static final class ParagraphVerticalAlign {
    public static final int TOP = 0;
    public static final int CENTER = 1;
    public static final int BASELINE = 2;
    public static final int BOTTOM = 3;

    private ParagraphVerticalAlign() {}
  }

  public static final class DecorationType {
    public static final int NONE = 0;
    public static final int UNDER_LINE = 1;

    private DecorationType() {}
  }

  public static final class Slant {
    public static final int UP_RIGHT_SLANT = 0;
    public static final int ITALIC_SLANT = 1;
    public static final int OBLIQUE_SLANT = 2;

    private Slant() {}
  }

  public static final class LinkStyle {
    public static final int NONE = 0;
    public static final int UNDERLINE = 1;

    private LinkStyle() {}
  }

  public static final class ThemeColorType {
    public static final int NORMAL = 0;
    public static final int LINK = 1;
    public static final int BACKGROUND = 2;
    public static final int BLOCK = 3;
    public static final int FOOTNOTE = 4;
    public static final int PRESSED_LINK = 5;

    private ThemeColorType() {}
  }

  public static final class CanvasOp {
    public static final int START_PAINT = 0;
    public static final int END_PAINT = 1;
    public static final int SAVE = 2;
    public static final int RESTORE = 3;
    public static final int TRANSLATE = 4;
    public static final int SCALE = 5;
    public static final int ROTATE = 6;
    public static final int SKEW = 7;
    public static final int CLIP_RECT = 8;
    public static final int CLEAR = 9;
    public static final int CLEAR_RECT = 10;
    public static final int FILL_RECT = 11;
    public static final int DRAW_COLOR = 12;
    public static final int DRAW_LINE = 13;
    public static final int DRAW_RECT = 14;
    public static final int DRAW_OVAL = 15;
    public static final int DRAW_CIRCLE = 16;
    public static final int DRAW_ARC = 17;
    public static final int DRAW_PATH = 18;
    public static final int DRAW_ARC_TO = 19;
    public static final int DRAW_TEXT = 20;
    public static final int DRAW_GLYPHS = 21;
    public static final int DRAW_RUN_DELEGATE = 22;
    public static final int DRAW_IMAGE = 23;
    public static final int DRAW_IMAGE_RECT = 24;
    public static final int DRAW_BACKGROUND_DELEGATE = 25;
    public static final int DRAW_BLOCK_REGION = 26;
    public static final int DRAW_ROUND_RECT = 27;

    private CanvasOp() {}
  }

  public static final class PathType {
    public static final int LINES = 0;
    public static final int ARC = 1;
    public static final int BEZIER = 2;
    public static final int MOVE_TO = 3;
    public static final int MULTI_PATH = 4;

    private PathType() {}
  }

  public static int GetFontWeight(int ordinal) {
    if (ordinal < 0 || ordinal > FontWeight.BLACK_900)
      return FontWeight.NORMAL_400;
    return ordinal;
  }

  public static int GetLinkStyle(int ordinal) {
    if (ordinal < 0 || ordinal > LinkStyle.UNDERLINE)
      return LinkStyle.NONE;
    return ordinal;
  }

  public static int GetThemeColorType(int ordinal) {
    if (ordinal < 0 || ordinal > ThemeColorType.PRESSED_LINK)
      return ThemeColorType.NORMAL;
    return ordinal;
  }
}
