// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.perftest;

import android.content.Context;
import android.graphics.Canvas;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.ForegroundColorSpan;
import android.util.AttributeSet;
import android.view.View;

/**
 * TODO: document your custom view class.
 */
public class StaticLayoutView extends View {
  private StaticLayout staticLayout;
  private SpannableStringBuilder builder;
  private TextPaint paint = new TextPaint();

  public StaticLayoutView(Context context) {
    super(context);
    init(null, 0);
  }

  public StaticLayoutView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init(attrs, 0);
  }

  public StaticLayoutView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
    init(attrs, defStyle);
  }

  private void init(AttributeSet attrs, int defStyle) {}

  public void CreateParagraph() {
    builder = new SpannableStringBuilder();
  }

  public void AddText(String content, float text_size, int color) {
    int idx = builder.length();
    builder.append(content);
    builder.setSpan(new AbsoluteSizeSpan((int) text_size, true), idx, idx + content.length(),
        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    builder.setSpan(new ForegroundColorSpan(color), idx, idx + content.length(),
        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    builder.setSpan(
        new AntiAliasSpan(), idx, idx + content.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
  }

  public void LayoutContent() {
    //        paint.setAntiAlias(true);
    //        paint.setColor(Color.RED);
    //        paint.setTextSize(48);
    //        paint.setWordSpacing(30);
    staticLayout = StaticLayout.Builder.obtain(builder, 0, builder.length(), paint, 500).build();
  }

  public void DrawContent(Canvas canvas) {
    if (staticLayout == null) {
      return;
    }
    staticLayout.draw(canvas);
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    DrawContent(canvas);
  }
}
