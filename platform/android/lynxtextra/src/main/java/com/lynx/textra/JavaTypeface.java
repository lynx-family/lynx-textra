// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.fonts.Font;
import android.os.Build;
import java.util.Objects;

public class JavaTypeface {
  private static final ThreadLocal<Paint> sPainter = new ThreadLocal<Paint>() {
    @Override
    protected Paint initialValue() {
      return new Paint();
    }
  };

  public static class FontKey {
    String mFontFamily = "";
    int mFontWeight = 400;
    boolean mItalic = false;
    String mFontFeatureSettings = "";
    int mSourceIndex = 0;

    @Override
    public boolean equals(Object o) {
      if (o == null || getClass() != o.getClass())
        return false;
      FontKey fontKey = (FontKey) o;
      return mFontWeight == fontKey.mFontWeight && mItalic == fontKey.mItalic
          && mSourceIndex == fontKey.mSourceIndex
          && Objects.equals(mFontFamily, fontKey.mFontFamily)
          && Objects.equals(mFontFeatureSettings, fontKey.mFontFeatureSettings);
    }

    @Override
    public int hashCode() {
      int result = mFontFamily.hashCode();
      result = 31 * result + mFontWeight;
      result = 31 * result + (mItalic ? 1 : 0);
      result = 31 * result + mSourceIndex;
      return 31 * result + mFontFeatureSettings.hashCode();
    }
  }

  public FontKey mFontKey;
  public Typeface mTypeface = null;
  public Font mFont = null;
  private final Paint.FontMetrics mFontMetrics = new Paint.FontMetrics();
  private float mTextSize = 0;

  public int mIndex = -1;

  public long mNativeHandler = 0;

  public JavaTypeface(int index, Typeface typeface, FontKey fontKey, long nativeHandler) {
    mIndex = index;
    mTypeface = typeface;
    mFontKey = fontKey;
    mNativeHandler = nativeHandler;
    InitFontMetrics(24);
    if (nativeHandler != 0) {
      BindNativeHandler(nativeHandler, this, mIndex, mFontMetrics.ascent / mTextSize,
          mFontMetrics.descent / mTextSize);
    }
  }

  public JavaTypeface(int index, Font typeface, FontKey fontKey, long nativeHandler) {
    mIndex = index;
    mFont = typeface;
    mFontKey = fontKey;
    mNativeHandler = nativeHandler;
    InitFontMetrics(24);
    if (nativeHandler != 0) {
      BindNativeHandler(nativeHandler, this, mIndex, mFontMetrics.ascent / mTextSize,
          mFontMetrics.descent / mTextSize);
    }
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

  private void InitFontMetrics(float text_size) {
    Paint paint = sPainter.get();
    paint.setTypeface(mTypeface);
    paint.setTextSize(text_size);
    paint.getFontMetrics(mFontMetrics);
    mTextSize = text_size;
  }

  public float[] GetFontMetrics(float text_size) {
    float scale = text_size / mTextSize;
    return new float[] {mFontMetrics.ascent * scale, mFontMetrics.descent * scale};
  }

  public float[] GetTextBounds(char[] text, float text_size) {
    Paint paint = sPainter.get();
    ApplyFontFeatureSettings(paint);
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

  public void ApplyFontFeatureSettings(Paint paint) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      paint.setFontFeatureSettings(
          mFontKey.mFontFeatureSettings.isEmpty() ? null : mFontKey.mFontFeatureSettings);
    }
  }

  public native void BindNativeHandler(
      long nativeHandler, JavaTypeface javaTypeface, int index, float ascent, float descent);
}
