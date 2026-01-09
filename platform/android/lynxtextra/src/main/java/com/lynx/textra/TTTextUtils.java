// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.content.Context;

public class TTTextUtils {
  public static Context context;

  public static void SetDpi(float dpi) {
    synchronized (TTTextUtils.class) {
      density_ = dpi / 160.f;
    }
  }

  public static float Dp2Px(float dp) {
    return dp * density_;
  }

  public static float Px2Dp(float px) {
    return px / density_;
  }

  public static float Dp2Px(float dp, float dpi) {
    return dp * dpi / 160.f;
  }

  public static float Px2Dp(float px, float dpi) {
    return px * 160.f / dpi;
  }

  public static float density_ = 1;

  public static byte[] SystemFontStyleAdjust(float font_size, short font_weight) {
    BBufferOutputStream bout = new BBufferOutputStream(6);
    bout.writeFloat(font_size);

    if (context != null) {
      PlatformFontCaps strategy = PlatformFontCaps.Factory.getStrategy(context);
      short new_weight = strategy.adjust(font_weight);
      bout.writeShort(new_weight);
    } else {
      bout.writeShort(font_weight);
    }

    return bout.getByteArray();
  }

  public static String SystemDefaultFamilyName() {
    if (context == null)
      return "sans-serif";
    PlatformFontCaps strategy = PlatformFontCaps.Factory.getStrategy(context);
    return strategy.defaultFamilyName();
  }

  public native static int nativeGetSystemPropInt(String key);
}
