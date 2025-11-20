package com.lynx.textra.skitydemo;

import android.content.Context;
import android.content.res.AssetManager;

public class SkityGLRender extends SkityRender {
  @Override
  protected long onInit(int width, int height, int density, Context context) {
    return nativeCreateGLRender(width, height, density, context.getAssets());
  }

  private native long nativeCreateGLRender(
      int width, int height, int density, AssetManager assetManager);
}
