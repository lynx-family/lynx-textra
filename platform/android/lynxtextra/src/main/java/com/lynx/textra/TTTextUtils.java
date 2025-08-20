// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

public class TTTextUtils {
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
}
