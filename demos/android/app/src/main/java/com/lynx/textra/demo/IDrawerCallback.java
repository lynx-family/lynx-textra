// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.graphics.Rect;
import com.lynx.textra.TTTextDefinition;

public interface IDrawerCallback {
  int fetchThemeColor(TTTextDefinition.ThemeColorType type, String extra);

  void drawRunDelegate(IRunDelegate delegate, Rect rect);

  void drawBackgroundDelegate(IRunDelegate delegate);
}
