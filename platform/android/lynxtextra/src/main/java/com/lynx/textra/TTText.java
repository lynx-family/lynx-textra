// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

public class TTText {
  private static boolean sInitialized = false;
  private static boolean sSelfRenderingPreloaded = false;

  public static void Initial() {
    Initial(false);
  }

  public static synchronized void Initial(boolean load_library) {
    if (sInitialized) {
      return;
    }
    if (load_library) {
      System.loadLibrary("lynxtextra");
    }
    nativeInitialCache();
    mFontManager = nativeGetDefaultFontManager();
    sInitialized = true;
  }

  public static boolean PreloadSelfRendering() {
    return PreloadSelfRendering(false);
  }

  public static synchronized boolean PreloadSelfRendering(boolean load_library) {
    if (sSelfRenderingPreloaded) {
      return true;
    }
    if (load_library) {
      System.loadLibrary("lynxtextra");
    }
    sSelfRenderingPreloaded = nativePreloadSelfRendering();
    return sSelfRenderingPreloaded;
  }

  private native static void nativeInitialCache();

  private native static boolean nativePreloadSelfRendering();

  private native static JavaFontManager nativeGetDefaultFontManager();

  public static JavaFontManager mFontManager = null;
}
