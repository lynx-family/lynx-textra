// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.perftest;

import android.text.TextPaint;
import android.text.style.CharacterStyle;
import android.text.style.UpdateAppearance;

public class AntiAliasSpan extends CharacterStyle implements UpdateAppearance {
  @Override
  public void updateDrawState(TextPaint tp) {
    tp.setAntiAlias(true);
  }
}
