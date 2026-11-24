// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.textra.perf;

import android.app.Activity;
import android.os.Bundle;
import com.lynx.textra.TTText;
import com.lynx.textra.TTTextUtils;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public final class PerfActivity extends Activity {
  private volatile boolean running;

  private static byte[] read(InputStream stream) throws Exception {
    try (InputStream input = stream; ByteArrayOutputStream output = new ByteArrayOutputStream()) {
      byte[] buffer = new byte[8192];
      int count;
      while ((count = input.read(buffer)) != -1) output.write(buffer, 0, count);
      return output.toByteArray();
    }
  }

  @Override
  public void onCreate(Bundle state) {
    super.onCreate(state);
    System.loadLibrary("perf_host");
    TTTextUtils.context = getApplicationContext();
    TTText.Initial(false);
    running = true;
    new Thread(() -> {
      File request = new File(getFilesDir(), "request.json");
      File response = new File(getFilesDir(), "response.json");
      try {
        byte[] metadata = read(getAssets().open("metadata.json"));
        while (running) {
          if (!request.isFile()) {
            Thread.sleep(10);
            continue;
          }
          byte[] input = read(new FileInputStream(request));
          if (!request.delete())
            throw new IllegalStateException("Cannot consume request");
          String result = nativeRun(input, metadata);
          if (result == null)
            throw new IllegalStateException("Native benchmark failed");
          File temporary = new File(getFilesDir(), "response.tmp");
          try (FileOutputStream output = new FileOutputStream(temporary)) {
            output.write(result.getBytes(StandardCharsets.UTF_8));
          }
          if (!temporary.renameTo(response))
            throw new IllegalStateException("Cannot publish result");
        }
      } catch (Exception error) {
        android.util.Log.e("TextraPerf", "Benchmark host failed", error);
      }
    }, "TextraPerf").start();
  }

  @Override
  public void onDestroy() {
    running = false;
    super.onDestroy();
  }

  private static native String nativeRun(byte[] request, byte[] metadata);
}
