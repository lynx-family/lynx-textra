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
import com.lynx.textra.TTTextUtils;

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

    TTTextUtils.context = getApplicationContext();

    // Adjust the layout to place the TextView closer to the custom-rendered view for easier
    // comparison.
    LinearLayout rootLayout = new LinearLayout(this);
    rootLayout.setOrientation(LinearLayout.VERTICAL);

    LinearLayout.LayoutParams rootParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
    rootLayout.setLayoutParams(rootParams);
    TextView textView = new TextView(this);
    textView.setText("第11章, test font weight, 😄🇨🇳");
    textView.setTextColor(Color.BLACK);
    textView.setTextSize(60.f / metrics.density);
    //    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
    //      textView.setFontVariationSettings("'wght' 600");
    //    }

    LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    textView.setLayoutParams(textParams);

    mView = new SkityGLView(this, new SkityGLRender());

    LinearLayout.LayoutParams glParams =
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1.0f);
    mView.setLayoutParams(glParams);

    rootLayout.addView(textView);
    rootLayout.addView(mView);

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
