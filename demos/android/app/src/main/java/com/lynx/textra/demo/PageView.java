// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;
import androidx.annotation.Nullable;
import com.lynx.textra.JavaFontManager;
import com.lynx.textra.TTText;

public class PageView extends View implements IDrawerCallback {
  private JavaDrawerCallback drawer;
  private JavaFontManager font_manager = null;
  private Bitmap renderedPage;
  private final float x_ = getContext().getResources().getDisplayMetrics().widthPixels * 0.01f;
  private final float y_ = getContext().getResources().getDisplayMetrics().heightPixels * 0.01f;
  private final float width_px_ =
      getContext().getResources().getDisplayMetrics().widthPixels * 0.98f;
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
    if (renderedPage == null)
      return;
    canvas.save();
    canvas.translate(x_, y_);
    canvas.drawColor(Color.WHITE);
    canvas.drawBitmap(renderedPage, 0, 0, null);
    canvas.restore();
  }

  @SuppressLint("ClickableViewAccessibility")
  private void Initial() {
    font_manager = TTText.mFontManager;
    AssetManager assetManager = getContext().getAssets();
    Typeface typeface = Typeface.createFromAsset(
        getContext().getAssets(), "Inter/Inter-VariableFont_opsz,wght.ttf");
    font_manager.CreateOrRegisterTypeface(typeface, "Inter", 400, false);
    typeface =
        Typeface.createFromAsset(getContext().getAssets(), "Noto_Sans_CJK/NotoSansCJK-Regular.ttc");
    font_manager.CreateOrRegisterTypeface(typeface, "NotoSansCJK", 400, false);
  }

  /**
   * Get C++ test case list
   */
  public static native TestCaseListFragment.CppTestItem[] getCppTestList();

  /**
   * Set C++ test case and update rendered page
   */
  public void setCppTestCase(int case_id) {
    Bitmap page = createPage();
    drawCppTestCase(new Canvas(page), case_id, width_px_);
    renderedPage = page;
    invalidate();
  }

  /**
   * Set JSON test case and update rendered page
   */
  public void setJsonTestCase(byte[] jsonBytes) {
    Bitmap page = createPage();
    drawJsonTestCase(new Canvas(page), jsonBytes, width_px_);
    renderedPage = page;
    invalidate();
  }

  private Bitmap createPage() {
    return Bitmap.createBitmap(Math.max(1, (int) Math.ceil(width_px_)),
        Math.max(1, (int) Math.ceil(height_)), Bitmap.Config.ARGB_8888);
  }

  private native void drawCppTestCase(Canvas canvas, int case_id, float width_px);

  private native void drawJsonTestCase(Canvas canvas, byte[] jsonBytes, float width_px);

  @Override
  public int fetchThemeColor(int type, String extra) {
    return 0;
  }

  @Override
  public void drawRunDelegate(IRunDelegate delegate, Rect rect) {}

  @Override
  public void drawBackgroundDelegate(IRunDelegate delegate) {}
}
