package com.lynx.textra.skitydemo;

import android.content.Context;

public abstract class SkityRender {
  private long mNativeHandle = 0;

  public void init(int width, int height, int density, Context context) {
    mNativeHandle = onInit(width, height, density, context);
  }

  public void draw() {
    nativeDraw(mNativeHandle);
  }

  public void destroy() {
    nativeDestroy(mNativeHandle);
  }

  protected abstract long onInit(int width, int height, int density, Context context);

  private native void nativeLayout(long handler);
  private native void nativeDraw(long handler);

  private native void nativeDestroy(long handler);
}
