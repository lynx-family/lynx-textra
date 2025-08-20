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
import com.lynx.textra.JavaCanvasHelper;
import com.lynx.textra.JavaFontManager;

public class TTTextView extends View {
  private byte[] drawer_buffer_;
  private long manager_handler = 0;
  private long region_handler = 0;
  private JavaFontManager font_manager = null;
  JavaCanvasHelper canvas_helper = null;
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
    font_manager = nativeGetFontManager(manager_handler);
    canvas_helper = new JavaCanvasHelper(font_manager);
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
    drawer_buffer_ = nativeDrawContent(canvas, manager_handler, region_handler);
    if (drawer_buffer_ == null)
      return;
    canvas_helper.drawBuffer(canvas, drawer_buffer_);
  }

  @Override
  protected void onDraw(@NonNull Canvas canvas) {
    DrawContent(canvas);
  }

  native long nativeCreateManager();

  native JavaFontManager nativeGetFontManager(long manager_handler);

  native void nativeCreateParagraph(long manager_handler);

  native void nativeAddText(long manager_handler, String content, float font_size, int color);

  native void nativeDestroyManager(long manager_handler);

  native long nativeLayoutContent(long handler, float height);

  native byte[] nativeDrawContent(Canvas canvas, long manager_handler, long region_handler);
}
