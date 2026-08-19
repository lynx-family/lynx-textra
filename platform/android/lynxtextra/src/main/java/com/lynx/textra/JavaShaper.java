// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Paint;

public class JavaShaper {
  private static final int INITIAL_ADVANCES_CAPACITY = 32;
  private static final int MAX_REUSABLE_ADVANCES_CAPACITY = 256;

  public JavaShaper(JavaFontManager font_manager) {}

  public float[] OnShapeText(
      String content, JavaTypeface font, float text_size, boolean is_rtl, boolean italic) {
    int textLength = content.length();
    float[] result = obtainAdvances(textLength);
    mPaint.setTypeface(font.mTypeface);
    mPaint.setTextSize(text_size);
    mPaint.getTextWidths(content, result);
    return result;
  }

  public JavaShapeResult OnShapeTextNew(
      String content, JavaTypeface font, float text_size, boolean is_rtl, boolean italic) {
    return null;
  }

  private float[] obtainAdvances(int requiredCapacity) {
    if (requiredCapacity > MAX_REUSABLE_ADVANCES_CAPACITY) {
      return new float[requiredCapacity];
    }
    if (requiredCapacity > mAdvances.length) {
      int newCapacity = Math.min(
          MAX_REUSABLE_ADVANCES_CAPACITY, Math.max(requiredCapacity, mAdvances.length * 2));
      mAdvances = new float[newCapacity];
    }
    return mAdvances;
  }

  private final Paint mPaint = new Paint();
  private float[] mAdvances = new float[INITIAL_ADVANCES_CAPACITY];
}
