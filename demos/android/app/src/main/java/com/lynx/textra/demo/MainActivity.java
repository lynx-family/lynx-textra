// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.MenuItem;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;
import com.lynx.textra.TTText;
import com.lynx.textra.TTTextUtils;

public class MainActivity extends AppCompatActivity {
  static {
    TTText.Initial(true);
    System.loadLibrary("android_test");
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    // Set ActionBar
    getSupportActionBar().setTitle("LynxTextra Demo");

    // Initialize DPI
    DisplayMetrics metrics = new DisplayMetrics();
    getWindowManager().getDefaultDisplay().getMetrics(metrics);
    TTTextUtils.SetDpi(metrics.densityDpi / 2);

    // If first creation, show list Fragment
    if (savedInstanceState == null) {
      TestCaseListFragment listFragment = new TestCaseListFragment();
      getSupportFragmentManager().beginTransaction().add(R.id.content_frame, listFragment).commit();
    }
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    if (item.getItemId() == android.R.id.home) {
      popBackStack();
      return true;
    }
    return super.onOptionsItemSelected(item);
  }

  @Override
  public void onBackPressed() {
    FragmentManager fragmentManager = getSupportFragmentManager();
    if (fragmentManager.getBackStackEntryCount() > 0) {
      popBackStack();
    } else {
      super.onBackPressed();
    }
  }

  private void popBackStack() {
    FragmentManager fragmentManager = getSupportFragmentManager();
    if (fragmentManager.getBackStackEntryCount() > 0) {
      fragmentManager.popBackStack();
      getSupportActionBar().setTitle("LynxTextra Demo");
      getSupportActionBar().setDisplayHomeAsUpEnabled(false);
    }
  }
}
