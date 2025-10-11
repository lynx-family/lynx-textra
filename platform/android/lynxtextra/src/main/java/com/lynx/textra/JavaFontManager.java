// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Typeface;
import android.graphics.fonts.Font;
import java.lang.reflect.Type;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicInteger;

public class JavaFontManager {
  private final long mNativeHandler;
  private final AtomicInteger mCount = new AtomicInteger(0);
  private final LinkedHashMap<JavaTypeface.FontKey, JavaTypeface> mFontMap = new LinkedHashMap<>();
  private final LinkedList<JavaTypeface> mFontList = new LinkedList<>();

  public JavaFontManager(long handler) {
    mNativeHandler = handler;
  }

  public long GetNativeHandler() {
    return mNativeHandler;
  }

  public synchronized JavaTypeface CreateOrRegisterTypeface(
      Typeface typeface, String families, int font_weight, boolean italic) {
    JavaTypeface.FontKey key = new JavaTypeface.FontKey();
    if (families != null && !families.isEmpty()) {
      key.mFontFamily = families;
    }
    key.mFontWeight = font_weight;
    key.mItalic = italic;
    JavaTypeface cached_typeface = mFontMap.get(key);
    if (cached_typeface == null) {
      cached_typeface = RegisterTypeface(typeface, key, CreateNativeTypeface(mNativeHandler));
    } else if (cached_typeface.mTypeface != typeface) {
      mFontList.add(cached_typeface);
      cached_typeface = RegisterTypeface(typeface, key, CreateNativeTypeface(mNativeHandler));
    }
    return cached_typeface;
  }

  public long onMatchTypefaceIndex(long index) {
    return GetTypefaceByIndex((int) index).mNativeHandler;
  }
  public long onMatchFamilyStyle(
      String families, int font_weight, boolean is_italic, long typeface_handler) {
    JavaTypeface.FontKey key = new JavaTypeface.FontKey();
    if (families != null && !families.isEmpty()) {
      key.mFontFamily = families;
    }
    key.mFontWeight = font_weight;
    key.mItalic = is_italic;
    JavaTypeface cached_typeface = mFontMap.get(key);
    if (cached_typeface != null) {
      return cached_typeface.mNativeHandler;
    }

    key.mFontFamily = "";
    cached_typeface = mFontMap.get(key);
    if (cached_typeface != null) {
      return cached_typeface.mNativeHandler;
    }

    int style = Typeface.NORMAL;
    if (font_weight >= 700 && is_italic) {
      style = Typeface.BOLD_ITALIC;
    } else if (font_weight >= 700) {
      style = Typeface.BOLD;
    } else if (is_italic) {
      style = Typeface.ITALIC;
    }

    try {
      Typeface tf = Typeface.create(Typeface.DEFAULT, style);
      if (tf == null)
        tf = Typeface.DEFAULT;

      cached_typeface = RegisterTypeface(tf, key, typeface_handler);
      if (cached_typeface.mNativeHandler == 0) {
        cached_typeface = null;
      }
    } catch (Exception e) {
      cached_typeface = null;
    }
    if (cached_typeface != null) {
      mFontMap.put(key, cached_typeface);
    } else {
      if (!mFontMap.isEmpty()) {
        cached_typeface = mFontMap.values().iterator().next();
      } else if (!mFontList.isEmpty()) {
        cached_typeface = mFontList.getFirst();
      }
    }

    return cached_typeface != null ? cached_typeface.mNativeHandler : mNativeHandler;
  }

  private synchronized JavaTypeface RegisterTypeface(
      Typeface typeface, JavaTypeface.FontKey key, long typeface_handler) {
    int index = mCount.incrementAndGet();
    return new JavaTypeface(index, typeface, key, typeface_handler);
  }

  public synchronized JavaTypeface RegisterShapeFont(Font font, JavaTypeface.FontKey key) {
    JavaTypeface java_typeface = mFontMap.get(key);
    if (java_typeface == null) {
      int index = mCount.incrementAndGet();
      java_typeface = new JavaTypeface(index, font, key, CreateNativeTypeface(mNativeHandler));
      mFontMap.put(key, java_typeface);
    }
    return java_typeface;
  }

  public JavaTypeface GetTypefaceByIndex(int index) {
    for (JavaTypeface java_typeface : mFontMap.values()) {
      if (java_typeface.mIndex == index) {
        return java_typeface;
      }
    }
    for (JavaTypeface java_typeface : mFontList) {
      if (java_typeface.mIndex == index) {
        return java_typeface;
      }
    }
    return null;
  }

  private native void BindNativeInstance(long nativeHandler, JavaFontManager java_instance);

  private native long CreateNativeTypeface(long nativeHandler);
}
