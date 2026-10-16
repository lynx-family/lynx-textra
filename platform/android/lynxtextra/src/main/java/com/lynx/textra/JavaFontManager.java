// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.annotation.TargetApi;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.fonts.Font;
import android.graphics.text.LineBreaker;
import android.graphics.text.MeasuredText;
import android.os.Build;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class JavaFontManager {
  private final long mNativeHandler;
  private final ConcurrentHashMap<JavaTypeface.FontKey, JavaTypeface> mFontMap =
      new ConcurrentHashMap<>();
  private final ConcurrentHashMap<Integer, JavaTypeface> mTypefaceByIndex =
      new ConcurrentHashMap<>();
  private final AtomicInteger mCount = new AtomicInteger();

  public JavaFontManager(long handler) {
    mNativeHandler = handler;
  }

  public long GetNativeHandler() {
    return mNativeHandler;
  }

  public JavaTypeface CreateOrRegisterTypeface(
      Typeface typeface, String families, int font_weight, boolean italic) {
    JavaTypeface.FontKey key = CreateFontKey(families, font_weight, italic);
    JavaTypeface cached_typeface = mFontMap.get(key);
    if (cached_typeface != null && cached_typeface.mTypeface == typeface) {
      return cached_typeface;
    }

    JavaTypeface candidate = RegisterTypeface(typeface, key, CreateNativeTypeface(mNativeHandler));
    mTypefaceByIndex.put(candidate.mIndex, candidate);
    while (true) {
      cached_typeface = mFontMap.get(key);
      if (cached_typeface != null && cached_typeface.mTypeface == typeface) {
        mTypefaceByIndex.remove(candidate.mIndex, candidate);
        return cached_typeface;
      }
      if (cached_typeface == null) {
        if (mFontMap.putIfAbsent(key, candidate) == null) {
          return candidate;
        }
      } else if (mFontMap.replace(key, cached_typeface, candidate)) {
        return candidate;
      }
    }
  }

  public long onMatchTypefaceIndex(long index) {
    return GetTypefaceByIndex((int) index).mNativeHandler;
  }
  public long onMatchFamilyStyle(
      String families, int font_weight, boolean is_italic, long typeface_handler) {
    JavaTypeface.FontKey requested_key = CreateFontKey(families, font_weight, is_italic);
    JavaTypeface.FontKey fallback_key = requested_key;
    JavaTypeface cached_typeface = mFontMap.get(requested_key);
    if (cached_typeface != null) {
      return cached_typeface.mNativeHandler;
    }

    if (!requested_key.mFontFamily.isEmpty()) {
      fallback_key = CreateFontKey("", font_weight, is_italic);
      cached_typeface = mFontMap.get(fallback_key);
      if (cached_typeface != null) {
        JavaTypeface requested_typeface = mFontMap.get(requested_key);
        if (requested_typeface != null) {
          return requested_typeface.mNativeHandler;
        }
        return cached_typeface.mNativeHandler;
      }
    }

    int style = Typeface.NORMAL;
    if (font_weight >= 700 && is_italic) {
      style = Typeface.BOLD_ITALIC;
    } else if (font_weight >= 700) {
      style = Typeface.BOLD;
    } else if (is_italic) {
      style = Typeface.ITALIC;
    }

    JavaTypeface candidate =
        RegisterTypeface(Typeface.create(Typeface.DEFAULT, style), fallback_key, typeface_handler);
    mTypefaceByIndex.put(candidate.mIndex, candidate);
    cached_typeface = mFontMap.putIfAbsent(fallback_key, candidate);
    if (cached_typeface != null) {
      mTypefaceByIndex.remove(candidate.mIndex, candidate);
      return cached_typeface.mNativeHandler;
    }
    return candidate.mNativeHandler;
  }

  private JavaTypeface RegisterTypeface(
      Typeface typeface, JavaTypeface.FontKey key, long typeface_handler) {
    return new JavaTypeface(mCount.incrementAndGet(), typeface, key, typeface_handler);
  }

  public JavaTypeface RegisterShapeFont(Font font, JavaTypeface.FontKey key) {
    JavaTypeface java_typeface = mFontMap.get(key);
    if (java_typeface != null) {
      return java_typeface;
    }

    JavaTypeface candidate =
        new JavaTypeface(mCount.incrementAndGet(), font, key, CreateNativeTypeface(mNativeHandler));
    mTypefaceByIndex.put(candidate.mIndex, candidate);
    java_typeface = mFontMap.putIfAbsent(key, candidate);
    if (java_typeface != null) {
      mTypefaceByIndex.remove(candidate.mIndex, candidate);
      return java_typeface;
    }
    return candidate;
  }

  public JavaTypeface GetTypefaceByIndex(int index) {
    return mTypefaceByIndex.get(index);
  }

  public void InitDefaultFallbackFonts() {
    if (Build.VERSION.SDK_INT >= 29) {
      float[] metrics = ChineseFallbackMetrics.Precompute();
      BindDefaultFallbackMetrics(mNativeHandler, metrics[0], metrics[1], metrics[2], metrics[3]);
    }
  }

  private static JavaTypeface.FontKey CreateFontKey(
      String families, int font_weight, boolean italic) {
    JavaTypeface.FontKey key = new JavaTypeface.FontKey();
    if (families != null && !families.isEmpty()) {
      key.mFontFamily = families;
    }
    key.mFontWeight = font_weight;
    key.mItalic = italic;
    return key;
  }

  @TargetApi(29)
  private static class ChineseFallbackMetrics {
    private static final float TEXT_SIZE = 24;

    static float[] Precompute() {
      Paint paint = new Paint();
      paint.setTypeface(Typeface.DEFAULT);
      paint.setTextSize(TEXT_SIZE);
      LineBreaker breaker = new LineBreaker.Builder()
                                .setBreakStrategy(LineBreaker.BREAK_STRATEGY_SIMPLE)
                                .setHyphenationFrequency(LineBreaker.HYPHENATION_FREQUENCY_NONE)
                                .build();
      LineBreaker.ParagraphConstraints constraints = new LineBreaker.ParagraphConstraints();
      constraints.setWidth(1024);
      float[] metrics = new float[4];
      // Supplementary Han can use a different fallback font from common Han.
      Measure("\u4E2D", paint, breaker, constraints, metrics, 0);
      Measure("\uD842\uDFB7", paint, breaker, constraints, metrics, 2);
      return metrics;
    }

    private static void Measure(String sample, Paint paint, LineBreaker breaker,
        LineBreaker.ParagraphConstraints constraints, float[] metrics, int offset) {
      char[] text = sample.toCharArray();
      MeasuredText measured = new MeasuredText.Builder(text)
                                  .setComputeHyphenation(false)
                                  .setComputeLayout(true)
                                  .appendStyleRun(paint, text.length, false)
                                  .build();
      LineBreaker.Result result = breaker.computeLineBreaks(measured, constraints, 0);
      metrics[offset] = result.getLineAscent(0) / TEXT_SIZE;
      metrics[offset + 1] = result.getLineDescent(0) / TEXT_SIZE;
    }
  }

  private native void BindNativeInstance(long nativeHandler, JavaFontManager java_instance);

  private native long CreateNativeTypeface(long nativeHandler);

  private native void BindDefaultFallbackMetrics(long nativeHandler, float ascent, float descent,
      float supplementaryAscent, float supplementaryDescent);
}
