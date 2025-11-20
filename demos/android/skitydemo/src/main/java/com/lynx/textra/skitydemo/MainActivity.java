// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.skitydemo;

import android.graphics.Color;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import com.lynx.textra.TTText;

public class MainActivity extends AppCompatActivity {
  static {
    TTText.Initial(true);
    System.loadLibrary("skity_demo");
  }
  private SkityGLView mView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    DisplayMetrics metrics = getResources().getDisplayMetrics();

    // 创建根布局 LinearLayout
    LinearLayout rootLayout = new LinearLayout(this);
    rootLayout.setOrientation(LinearLayout.VERTICAL); // 垂直方向

    // 设置 LayoutParams，全屏
    LinearLayout.LayoutParams rootParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
    rootLayout.setLayoutParams(rootParams);

    // 创建 TextView
    TextView textView = new TextView(this);
    textView.setText("测试字重, test font weight, 😄🇨🇳");
    textView.setBackgroundColor(Color.LTGRAY);
    textView.setTextColor(Color.BLACK);
    textView.setTextSize(60.f / metrics.density);
    //    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
    //      textView.setFontVariationSettings("'wght' 600");
    //    }

    // TextView 的 LayoutParams
    LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    textView.setLayoutParams(textParams);

    mView = new SkityGLView(this, new SkityGLRender());

    // GLView LayoutParams，让它占剩余空间
    LinearLayout.LayoutParams glParams =
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0,
            1.0f // weight = 1，占剩余空间
        );
    mView.setLayoutParams(glParams);

    // 添加到根布局
    rootLayout.addView(textView);
    rootLayout.addView(mView);

    // 设置 Activity 的内容视图
    setContentView(rootLayout);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mView.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mView.onResume();
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    mView.onDestroy();
  }
}
