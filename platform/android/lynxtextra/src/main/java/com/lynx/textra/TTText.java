// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Paint;

public class TTText {
  public static void Initial() {
    Initial(false);
  }

  public static void Initial(boolean load_library) {
    if (load_library) {
      System.loadLibrary("lynxtextra");
    }
    nativeInitialCache();
    mFontManager = nativeGetDefaultFontManager();
  }

  private native static void nativeInitialCache();

  private native static JavaFontManager nativeGetDefaultFontManager();

  public static Paint mPainter = new Paint();
  public static JavaFontManager mFontManager = null;
}
