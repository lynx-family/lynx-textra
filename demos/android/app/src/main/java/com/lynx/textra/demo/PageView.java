// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;
import androidx.annotation.Nullable;
import com.lynx.textra.JavaCanvasHelper;
import com.lynx.textra.JavaFontManager;
import com.lynx.textra.TTTextDefinition;
import com.lynx.textra.TTTextUtils;

public class PageView extends View implements IDrawerCallback {
  private JavaDrawerCallback drawer;
  private JavaFontManager font_manager = null;
  private byte[] mDrawBuffer;
  private final float x_ = getContext().getResources().getDisplayMetrics().widthPixels * 0.01f;
  private final float y_ = getContext().getResources().getDisplayMetrics().heightPixels * 0.01f;
  private final float width_ = getContext().getResources().getDisplayMetrics().widthPixels * 0.98f;
  private final float height_ =
      getContext().getResources().getDisplayMetrics().heightPixels * 0.95f;
  MainActivity activity;

  @SuppressLint("ClickableViewAccessibility")
  public PageView(Context context) {
    super(context);
    activity = (MainActivity) context;
    drawer = new JavaDrawerCallback(this);
    Initial();
  }

  public PageView(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
  }

  public PageView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
  }

  @SuppressLint("DrawAllocation")
  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    if (mDrawBuffer == null)
      return;
    canvas.save();
    canvas.translate(x_, y_);
    canvas.drawColor(Color.WHITE);
    JavaCanvasHelper helper = new JavaCanvasHelper(font_manager);
    helper.drawBuffer(canvas, mDrawBuffer);
    canvas.restore();
  }

  @SuppressLint("ClickableViewAccessibility")
  private void Initial() {
    font_manager = nativeCreateFontManager();
    AssetManager assetManager = getContext().getAssets();
    Typeface typeface = Typeface.createFromAsset(
        getContext().getAssets(), "Inter/Inter-VariableFont_opsz,wght.ttf");
    font_manager.CreateOrRegisterTypeface(typeface, "Inter", 400, false);
    typeface =
        Typeface.createFromAsset(getContext().getAssets(), "Noto_Sans_CJK/NotoSansCJK-Regular.ttc");
    font_manager.CreateOrRegisterTypeface(typeface, "NotoSansCJK", 400, false);
  }

  private native JavaFontManager nativeCreateFontManager();

  /**
   * Get C++ test case list
   */
  public static native TestCaseListFragment.CppTestItem[] getCppTestList();

  /**
   * Set C++ test case and update draw buffer
   */
  public void setCppTestCase(int case_id) {
    mDrawBuffer =
        drawCppTestCase(font_manager.GetNativeHandler(), case_id, TTTextUtils.Px2Dp(width_));
    invalidate();
  }

  /**
   * Set JSON test case and update draw buffer
   */
  public void setJsonTestCase(byte[] jsonBytes) {
    mDrawBuffer =
        drawJsonTestCase(font_manager.GetNativeHandler(), jsonBytes, TTTextUtils.Px2Dp(width_));
    invalidate();
  }

  private native byte[] drawCppTestCase(long font_manager_handler, int case_id, float width);

  private native byte[] drawJsonTestCase(long font_manager_handler, byte[] jsonBytes, float width);

  @Override
  public int fetchThemeColor(TTTextDefinition.ThemeColorType type, String extra) {
    return 0;
  }

  @Override
  public void drawRunDelegate(IRunDelegate delegate, Rect rect) {}

  @Override
  public void drawBackgroundDelegate(IRunDelegate delegate) {}
}
