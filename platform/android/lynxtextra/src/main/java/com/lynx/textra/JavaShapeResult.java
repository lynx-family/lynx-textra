// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

class JavaShapeResult {
  public JavaShapeResult(int glyph_count) {
    glyphs_ = new short[glyph_count];
    advance_ = new float[glyph_count];
    position_x_ = new float[glyph_count];
    position_y_ = new float[glyph_count];
    typeface_instance_ = new long[glyph_count];
    glyph_count_ = glyph_count;
  }

  public int glyph_count_;
  public short[] glyphs_;
  public float[] advance_;
  public float[] position_x_;
  public float[] position_y_;
  public long[] typeface_instance_;
}
