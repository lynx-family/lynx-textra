// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
package com.lynx.textra.demo;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import com.lynx.textra.TTText;
import java.io.File;
import java.io.FileOutputStream;
import java.util.Arrays;
import org.json.JSONArray;
import org.json.JSONObject;

/** C API example and reproducible C++/C API comparison in the existing demo. */
public class CapiDemoActivity extends Activity {
  static {
    System.loadLibrary("lynxtextra");
    System.loadLibrary("android_test");
  }
  static native float[] nativeRender(Canvas canvas, int caseId, boolean capi);
  private static native String nativeShapingBenchmark(int rounds, int workloadRotation);
  private static native String nativeFontPolicyTest();

  private static void save(Bitmap bitmap, File file) throws Exception {
    try (FileOutputStream stream = new FileOutputStream(file)) {
      if (!bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream))
        throw new Exception("PNG encoding failed");
    }
  }
  @Override
  public void onCreate(Bundle state) {
    super.onCreate(state);
    if (getIntent().getBooleanExtra("shaping_benchmark", false)
        || getIntent().getBooleanExtra("font_architecture_test", false)) {
      TextView label = new TextView(this);
      label.setText("Measuring shaping performance…");
      setContentView(label);
      TTText.Initial(false);
      new Thread(() -> {
        try {
          if (getIntent().getBooleanExtra("font_architecture_test", false)) {
            CapiFontArchitectureTest.run(this, new JSONArray(nativeFontPolicyTest()));
            runOnUiThread(() -> label.setText("Font architecture checks complete"));
            return;
          }
          String result = nativeShapingBenchmark(getIntent().getIntExtra("rounds", 21),
              getIntent().getIntExtra("workload_rotation", 0));
          File report = new File(getFilesDir(), "shaping-benchmark.json");
          try (FileOutputStream stream = new FileOutputStream(report)) {
            stream.write(result.getBytes("UTF-8"));
          }
          boolean ok = new JSONObject(result).optBoolean("results_equal", false);
          Log.i("TextraShapingBenchmark", "COMPLETE " + ok + " " + report);
          runOnUiThread(() -> label.setText("Shaping benchmark complete: " + ok));
        } catch (Exception | LinkageError error) {
          Log.e("TextraShapingBenchmark", "benchmark failed", error);
        }
      }, "textra-shaping-benchmark").start();
      return;
    }

    LinearLayout content = new LinearLayout(this);
    content.setOrientation(LinearLayout.VERTICAL);
    ScrollView scroll = new ScrollView(this);
    scroll.addView(content);
    setContentView(scroll);
    File output = new File(getFilesDir(), "capi-comparison");
    output.mkdirs();
    JSONArray results = new JSONArray();
    boolean all = true;
    String[] names = {"latin", "cjk", "wrapped", "rtl", "bold", "italic", "multiline", "fallback"};
    try {
      for (int i = 0; i < names.length; ++i) {
        Bitmap c = Bitmap.createBitmap(320, 200, Bitmap.Config.ARGB_8888);
        Bitmap cpp = Bitmap.createBitmap(320, 200, Bitmap.Config.ARGB_8888);
        Bitmap repeat = Bitmap.createBitmap(320, 200, Bitmap.Config.ARGB_8888);
        // Run the independent provider before initializing the legacy C++ manager.
        float[] cm = nativeRender(new Canvas(c), i, true);
        TTText.Initial(false);
        float[] old = nativeRender(new Canvas(cpp), i, false);
        float[] rm = nativeRender(new Canvas(repeat), i, true);
        int[] a = new int[320 * 200], b = new int[a.length], r = new int[a.length];
        cpp.getPixels(a, 0, 320, 0, 0, 320, 200);
        c.getPixels(b, 0, 320, 0, 0, 320, 200);
        repeat.getPixels(r, 0, 320, 0, 0, 320, 200);
        boolean metrics = true, nonempty = false;
        int differences = 0;
        for (int k = 0; k < 3; ++k)
          metrics &= Math.abs(cm[k] - old[k]) < 0.001f && Math.abs(cm[k] - rm[k]) < 0.001f;
        for (int k = 0; k < a.length; ++k) {
          if (a[k] != b[k])
            ++differences;
          nonempty |= b[k] != 0;
        }
        boolean pixels = differences == 0, repeated = Arrays.equals(b, r);
        JSONObject result = new JSONObject()
                                .put("case", names[i])
                                .put("pixels_equal", pixels)
                                .put("different_pixels", differences)
                                .put("metrics_equal", metrics)
                                .put("repeat_equal", repeated)
                                .put("nonempty", nonempty);
        results.put(result);
        all &= pixels && metrics && repeated && nonempty;
        save(cpp, new File(output, names[i] + "-cpp.png"));
        save(c, new File(output, names[i] + "-capi.png"));
        save(repeat, new File(output, names[i] + "-repeat.png"));
        TextView label = new TextView(this);
        label.setText(
            names[i] + ": " + (pixels && metrics && repeated && nonempty ? "PASS" : "FAIL"));
        content.addView(label);
        ImageView image = new ImageView(this);
        image.setImageBitmap(c);
        image.setAdjustViewBounds(true);
        image.setScaleType(ImageView.ScaleType.FIT_START);
        content.addView(image,
            new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
                (int) (200 * getResources().getDisplayMetrics().density)));
      }
    } catch (Exception error) {
      all = false;
      Log.e("TextraCapiDemo", "comparison failed", error);
      try {
        results.put(new JSONObject().put("error", error.toString()));
      } catch (Exception ignored) {
      }
    }
    try (FileOutputStream stream = new FileOutputStream(new File(output, "results.json"))) {
      stream.write(results.toString(2).getBytes("UTF-8"));
    } catch (Exception error) {
      all = false;
      Log.e("TextraCapiDemo", "saving report", error);
    }
    Log.i("TextraCapiDemo", "TEXTRA_CAPI_COMPARISON " + (all ? "PASS" : "FAIL") + " " + output);
  }
}
