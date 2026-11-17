// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

import java.util.HashMap;
import java.util.Map;
import org.junit.Test;

public class FontFeatureCacheKeyTest {
  private JavaTypeface.FontKey key(int sourceIndex, String features) {
    JavaTypeface.FontKey key = new JavaTypeface.FontKey();
    key.mFontFamily = "sans-serif";
    key.mSourceIndex = sourceIndex;
    key.mFontFeatureSettings = features;
    return key;
  }

  @Test
  public void featureValuesHaveIndependentCacheEntries() {
    Map<JavaTypeface.FontKey, Integer> cache = new HashMap<>();
    cache.put(key(1, "'liga' 0"), 10);
    cache.put(key(1, "'liga' 1"), 20);
    cache.put(key(1, "'cv01' 2"), 30);
    cache.put(key(1, ""), 40);
    assertEquals(4, cache.size());
    assertEquals(Integer.valueOf(10), cache.get(key(1, "'liga' 0")));
    assertEquals(Integer.valueOf(20), cache.get(key(1, "'liga' 1")));
    assertEquals(Integer.valueOf(30), cache.get(key(1, "'cv01' 2")));
    assertEquals(Integer.valueOf(40), cache.get(key(1, "")));
  }

  @Test
  public void registeredFontsWithTheSameFamilyStayIndependent() {
    assertNotEquals(key(1, "'liga' 1"), key(2, "'liga' 1"));
    assertEquals(key(1, "'liga' 1"), key(1, "'liga' 1"));
  }
}
