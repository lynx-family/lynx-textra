// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
package com.lynx.textra.demo;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.util.Log;
import java.io.File;
import java.io.FileOutputStream;
import java.util.Arrays;
import org.json.JSONArray;
import org.json.JSONObject;

final class CapiFontArchitectureTest {
  static void run(Activity activity, JSONArray checks) throws Exception {
    File output = new File(activity.getFilesDir(), "font-architecture");
    output.mkdirs();
    String[] names = {"latin", "cjk", "wrapped", "rtl", "bold", "italic", "multiline", "fallback"};
    for (int mode = 1; mode <= 2; ++mode)
      for (int i = 0; i < 8; ++i) {
        int sample = 8 * mode + i;
        Bitmap a = Bitmap.createBitmap(320, 200, Bitmap.Config.ARGB_8888),
               b = Bitmap.createBitmap(320, 200, Bitmap.Config.ARGB_8888);
        float[] x = CapiDemoActivity.nativeRender(new Canvas(a), sample, false);
        float[] y = CapiDemoActivity.nativeRender(new Canvas(b), sample, true);
        int[] pa = new int[320 * 200], pb = new int[pa.length];
        a.getPixels(pa, 0, 320, 0, 0, 320, 200);
        b.getPixels(pb, 0, 320, 0, 0, 320, 200);
        boolean metrics = true;
        for (int j = 0; j < 3; ++j) metrics &= Math.abs(x[j] - y[j]) < 0.001;
        boolean equal = Arrays.equals(pa, pb), nonempty = false;
        for (int pixel : pb) nonempty |= pixel != 0;
        String name = names[i] + (mode == 1 ? "-round-24" : "-round-24.5");
        checks.put(new JSONObject()
                       .put("name", name)
                       .put("passed", equal && metrics && nonempty)
                       .put("pixels_equal", equal)
                       .put("metrics_equal", metrics));
        try (FileOutputStream stream = new FileOutputStream(new File(output, name + "-cpp.png"))) {
          a.compress(Bitmap.CompressFormat.PNG, 100, stream);
        }
        try (FileOutputStream stream = new FileOutputStream(new File(output, name + "-capi.png"))) {
          b.compress(Bitmap.CompressFormat.PNG, 100, stream);
        }
      }
    boolean passed = true;
    for (int i = 0; i < checks.length(); ++i)
      passed &= checks.getJSONObject(i).getBoolean("passed");
    try (FileOutputStream stream = new FileOutputStream(new File(output, "results.json"))) {
      stream.write(checks.toString(2).getBytes("UTF-8"));
    }
    Log.i("TextraFontArchitecture", "COMPLETE " + passed + " " + output);
  }
}
