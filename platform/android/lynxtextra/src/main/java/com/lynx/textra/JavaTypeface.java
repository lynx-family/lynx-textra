// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.fonts.Font;
import java.util.Objects;

public class JavaTypeface {
  public static class FontKey {
    String mFontFamily = "";
    int mFontWeight = 400;
    boolean mItalic = false;

    @Override
    public boolean equals(Object o) {
      if (o == null || getClass() != o.getClass())
        return false;
      FontKey fontKey = (FontKey) o;
      return mFontWeight == fontKey.mFontWeight && mItalic == fontKey.mItalic
          && Objects.equals(mFontFamily, fontKey.mFontFamily);
    }

    @Override
    public int hashCode() {
      return Objects.hash(mFontFamily, mFontWeight, mItalic);
    }
  }

  public FontKey mFontKey;
  public Typeface mTypeface = null;
  public Font mFont = null;
  private final Paint.FontMetrics mFontMetrics = new Paint.FontMetrics();
  private float mTextSize = 0;

  public int mIndex = -1;

  public long mNativeHandler = 0;

  private float dpi = 1;

  public JavaTypeface(int index, Typeface typeface, FontKey fontKey, long nativeHandler) {
    mIndex = index;
    mTypeface = typeface;
    mFontKey = fontKey;
    mNativeHandler = nativeHandler;
    GetFontMetrics(24);
    BindNativeHandler(nativeHandler, this, mIndex);
  }

  public JavaTypeface(int index, Font typeface, FontKey fontKey, long nativeHandler) {
    mIndex = index;
    mFont = typeface;
    mFontKey = fontKey;
    mNativeHandler = nativeHandler;
    BindNativeHandler(nativeHandler, this, mIndex);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o)
      return true;
    if (o == null || getClass() != o.getClass())
      return false;
    JavaTypeface that = (JavaTypeface) o;
    return mFontKey == that.mFontKey;
  }

  @Override
  public int hashCode() {
    return Objects.hash(mFontKey);
  }

  public float[] GetFontMetrics(float text_size) {
    float[] font_metrics = new float[2];
    if (mTextSize == 0 || dpi != TTTextUtils.density_) {
      TTText.mPainter.setTypeface(mTypeface);
      TTText.mPainter.setTextSize(TTTextUtils.Dp2Px(text_size));
      TTText.mPainter.getFontMetrics(mFontMetrics);
      dpi = TTTextUtils.density_;
    } else {
      float scale = text_size / mTextSize;
      mFontMetrics.ascent *= scale;
      mFontMetrics.descent *= scale;
    }
    mTextSize = text_size;
    font_metrics[0] = TTTextUtils.Px2Dp(mFontMetrics.ascent);
    font_metrics[1] = TTTextUtils.Px2Dp(mFontMetrics.descent);
    return font_metrics;
  }

  public float[] GetTextBounds(char[] text, float text_size) {
    Paint paint = TTText.mPainter;
    Rect rect = new Rect();
    paint.setTypeface(mTypeface);
    paint.setTextSize(text_size);
    paint.getTextBounds(text, 0, text.length, rect);
    float[] ret = new float[4];
    ret[0] = rect.left;
    ret[1] = rect.top;
    ret[2] = rect.right;
    ret[3] = rect.bottom;
    return ret;
  }

  public native void BindNativeHandler(long nativeHandler, JavaTypeface javaTypeface, int index);
}
