// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.graphics.Rect;
import androidx.annotation.Keep;
import com.lynx.textra.TTTextDefinition;
import java.io.ByteArrayInputStream;
import java.io.IOException;
@Keep
public class JavaDrawerCallback {
  private final IDrawerCallback callback_;
  private final long instance_;

  public JavaDrawerCallback(IDrawerCallback callback) {
    callback_ = callback;
    instance_ = nativeCreateDrawerCallback();
  }

  @Override
  protected void finalize() throws Throwable {
    super.finalize();
    nativeDestroyDrawerCallback(instance_);
  }

  public int FetchThemeColor(byte[] input) {
    ByteArrayInputStream inputStream = new ByteArrayInputStream(input);
    CBufferInputStream cBufferInputStream = new CBufferInputStream(inputStream);
    try {
      int type = cBufferInputStream.readInt();
      String extra = cBufferInputStream.ReadCString();
      return callback_.fetchThemeColor(TTTextDefinition.GetThemeColorType(type), extra);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return 0;
  }

  public void drawRunDelegate(IRunDelegate delegate, Rect rect) {
    if (delegate == null) {
      return;
    }
    callback_.drawRunDelegate(delegate, rect);
  }

  public void drawBackgroundDelegate(IRunDelegate delegate) {
    if (delegate == null) {
      return;
    }
    callback_.drawBackgroundDelegate(delegate);
  }

  public long GetInstance() {
    return instance_;
  }

  private native long nativeCreateDrawerCallback();

  private native void nativeDestroyDrawerCallback(long instance);
}
