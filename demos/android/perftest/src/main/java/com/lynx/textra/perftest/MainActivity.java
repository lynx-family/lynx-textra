// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.perftest;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import com.lynx.textra.TTText;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Random;

public class MainActivity extends AppCompatActivity {
  static {
    TTText.Initial(true);
    System.loadLibrary("perf_test");
  }

  private String test_content;
  private TTTextView tttext_view;
  private StaticLayoutView static_layout_view;
  private Switch engine_switch;
  private Switch text_switch;
  private Switch random_fontsize_switch;
  private Switch random_color_switch;
  private TextView logview;

  double tttext_layout_cost = 0;
  double tttext_draw_cost = 0;
  double sl_layout_cost = 0;
  double sl_draw_cost = 0;
  private Random random = new Random();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    Button button = findViewById(R.id.start_test);
    button.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        StartTest(1000, 100);
      }
    });
    tttext_view = findViewById(R.id.TTTextView);
    static_layout_view = findViewById(R.id.staticLayoutView);
    logview = findViewById(R.id.logview);
    logview.setTextColor(Color.BLACK);
    engine_switch = findViewById(R.id.engine_switch);
    engine_switch.setTextOn("LynxTextra");
    engine_switch.setTextOff("StaticLayout");
    text_switch = findViewById(R.id.text_switch);
    text_switch.setTextOff("Chinese Test");
    text_switch.setTextOn("English Test");
    random_fontsize_switch = findViewById(R.id.random_fontsize_switch);
    random_color_switch = findViewById(R.id.random_color_switch);
  }

  public String readAssetFile(Context context, String filename) {
    AssetManager assetManager = context.getAssets();
    StringBuilder stringBuilder = new StringBuilder();

    try (InputStream inputStream = assetManager.open(filename);
         BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
      String line;
      while ((line = reader.readLine()) != null) {
        stringBuilder.append(line).append("\n");
      }
    } catch (IOException e) {
      e.printStackTrace();
    }

    return stringBuilder.toString();
  }

  public void StartTest(int test_count, int test_max_char) {
    Log.e("PerfTest", "test_count:" + test_count + ", max_char_count:" + test_max_char);
    if (text_switch.isChecked()) {
      test_content = readAssetFile(getApplicationContext(), "english.txt");
    } else {
      test_content = readAssetFile(getApplicationContext(), "hlm.txt");
    }
    Bitmap bitmap = Bitmap.createBitmap(500, 500, Bitmap.Config.ARGB_8888);
    Canvas canvas = new Canvas(bitmap);
    InitTestContent("Hello, testing...");
    TestTTText("Hello, testing...", canvas);
    TestStaticLayout("Hello, testing...", canvas);
    tttext_layout_cost = 0;
    tttext_draw_cost = 0;
    sl_layout_cost = 0;
    sl_draw_cost = 0;
    int total_char = 0;
    for (int k = 0; k < test_count; k++) {
      canvas.drawColor(Color.WHITE);
      int content_start = random.nextInt(test_content.length() - 1);
      int content_len =
          random.nextInt(Math.min(test_max_char - 1, test_content.length() - content_start - 1))
          + 1;
      String test_case = test_content.substring(content_start, content_start + content_len);
      InitTestContent(test_case);
      if (engine_switch.isChecked()) {
        TestTTText(test_case, canvas);
      } else {
        TestStaticLayout(test_case, canvas);
      }
      total_char += content_len;
      tttext_view.postInvalidate();
      static_layout_view.postInvalidate();
    }
    PrintSummary(test_count, total_char);
  }

  private void InitTestContent(String content) {
    static_layout_view.CreateParagraph();
    tttext_view.CreateParagraph();
    int font_size_delta = 24;
    int r = 0, g = 255, b = 0;
    if (random_color_switch.isChecked() || random_fontsize_switch.isChecked()) {
      for (int k = 0; k < content.length(); k++) {
        if (random_fontsize_switch.isChecked()) {
          font_size_delta = random.nextInt(24);
        }
        if (random_color_switch.isChecked()) {
          r = random.nextInt(256);
          g = random.nextInt(256);
          b = random.nextInt(256);
        }
        String ch = content.substring(k, k + 1);
        if (!ch.equals("\n") && !ch.equals("\r")) {
          static_layout_view.AddText(
              ch, 24 + font_size_delta, 0xff000000 | (r << 16) | (g << 8) | b);
          tttext_view.AddText(ch, 24 + font_size_delta, 0xff000000 | (r << 16) | (g << 8) | b);
        }
      }
    } else {
      static_layout_view.AddText(
          content, 24 + font_size_delta, 0xff000000 | (r << 16) | (g << 8) | b);
      tttext_view.AddText(content, 24 + font_size_delta, 0xff000000 | (r << 16) | (g << 8) | b);
    }
  }

  private float ns2ms(float nano_sec, int char_count) {
    return nano_sec / 1000.f / char_count;
  }

  private void TestTTText(String content, Canvas canvas) {
    long start = System.nanoTime();
    tttext_view.LayoutContent();
    long end = System.nanoTime();
    tttext_layout_cost += ns2ms(end - start, content.length());
    start = System.nanoTime();
    tttext_view.DrawContent(canvas);
    end = System.nanoTime();
    tttext_draw_cost += ns2ms(end - start, content.length());
  }

  private void TestStaticLayout(String content, Canvas canvas) {
    long start = System.nanoTime();
    static_layout_view.LayoutContent();
    long end = System.nanoTime();
    sl_layout_cost += ns2ms(end - start, content.length());
    start = System.nanoTime();
    static_layout_view.DrawContent(canvas);
    end = System.nanoTime();
    sl_draw_cost += ns2ms(end - start, content.length());
  }

  private void PrintSummary(int test_count, int total_char) {
    String log_text;
    if (engine_switch.isChecked()) {
      log_text =
          String.format("LynxTextra\n\tlayout:%10.4f(ms/1000chars)\n\tdraw:%10.4f(ms/1000chars)\n",
              tttext_layout_cost / test_count, tttext_draw_cost / test_count);
    } else {
      log_text =
          String.format("Static\n\tlayout:%10.4f(ms/1000chars)\n\tdraw:%10.4f(ms/1000chars)\n",
              sl_layout_cost / test_count, sl_draw_cost / test_count);
    }
    logview.append(log_text);

    Log.e("PerfTest",
        "LynxTextra layout:" + tttext_layout_cost / test_count + "(ms/1000chars), draw:"
            + tttext_draw_cost / test_count + "(ms/1000chars), total chars:" + total_char);
    Log.e("PerfTest",
        "Static layout:" + sl_layout_cost / test_count + "(ms/1000chars), draw:"
            + sl_draw_cost / test_count + "(ms/1000chars), total chars:" + total_char);
  }
}
