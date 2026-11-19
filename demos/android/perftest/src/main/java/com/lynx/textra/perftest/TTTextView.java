// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.perftest;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class TTTextView extends View {
  private long manager_handler = 0;
  private long region_handler = 0;
  private String content;
  private Rect clip_rect = new Rect();

  public TTTextView(Context context) {
    super(context);
    init();
  }

  public TTTextView(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  public TTTextView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    init();
  }

  public TTTextView(
      Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
    init();
  }

  private void init() {
    if (manager_handler == 0) {
      manager_handler = nativeCreateManager();
    }
  }

  public void CreateParagraph() {
    nativeCreateParagraph(manager_handler);
  }

  public void AddText(String content, float text_size, int color) {
    nativeAddText(manager_handler, content, text_size, color);
  }

  public void LayoutContent() {
    region_handler = nativeLayoutContent(manager_handler, getHeight());
  }

  public void DrawContent(Canvas canvas) {
    if (region_handler == 0)
      return;
    nativeDrawContent(canvas, manager_handler, region_handler);
  }

  @Override
  protected void onDraw(@NonNull Canvas canvas) {
    DrawContent(canvas);
  }

  native long nativeCreateManager();

  native void nativeCreateParagraph(long manager_handler);

  native void nativeAddText(long manager_handler, String content, float font_size, int color);

  native void nativeDestroyManager(long manager_handler);

  native long nativeLayoutContent(long handler, float height);

  native void nativeDrawContent(Canvas canvas, long manager_handler, long region_handler);
}
