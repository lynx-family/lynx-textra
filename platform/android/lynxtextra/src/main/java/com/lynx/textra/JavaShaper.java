// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Paint;
import java.util.HashMap;

public class JavaShaper {
  private HashMap<Integer, JavaTypeface> mFontMap = new HashMap<>(10);

  public JavaShaper(JavaFontManager font_manager) {
    mFontManager = font_manager;
  }

  public float[] OnShapeText(
      String content, JavaTypeface font, float text_size, boolean is_rtl, boolean italic) {
    float[] result = new float[content.length()];
    mPaint.setTypeface(font.mTypeface);
    mPaint.setTextSize(TTTextUtils.Dp2Px(text_size));
    mPaint.getTextWidths(content, result);
    for (int k = 0; k < content.length(); k++) {
      result[k] = TTTextUtils.Px2Dp(result[k]);
    }
    font.GetFontMetrics(text_size);
    return result;
  }

  public JavaShapeResult OnShapeTextNew(
      String content, JavaTypeface font, float text_size, boolean is_rtl, boolean italic) {
    return null;
  }

  private JavaFontManager mFontManager = null;
  private BBufferOutputStream bs = null;
  private Paint mPaint = new Paint();
}
