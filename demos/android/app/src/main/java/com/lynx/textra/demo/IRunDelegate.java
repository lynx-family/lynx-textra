// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import com.lynx.textra.TTTextDefinition;
public interface IRunDelegate {
  public float getAscent();

  public float getDescent();

  public float getAdvance();

  public boolean hide();
  IRunDelegate resize(TTTextDefinition.Size size, float font_size, boolean is_inline);
  public void setIsFootnote(boolean is_footnote);
  public void updateFontMetrics(float ascent, float descent);
}
