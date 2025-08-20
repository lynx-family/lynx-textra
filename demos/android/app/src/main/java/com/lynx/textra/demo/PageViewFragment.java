// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

public class PageViewFragment extends Fragment {
  private static final String ARG_TEST_ID = "test_id";
  private static final String ARG_JSON_BYTES = "json_bytes";

  public static PageViewFragment newInstance(int testId) {
    PageViewFragment fragment = new PageViewFragment();
    Bundle args = new Bundle();
    args.putInt(ARG_TEST_ID, testId);
    fragment.setArguments(args);
    return fragment;
  }

  public static PageViewFragment newInstance(byte[] jsonBytes) {
    PageViewFragment fragment = new PageViewFragment();
    Bundle args = new Bundle();
    args.putByteArray(ARG_JSON_BYTES, jsonBytes);
    fragment.setArguments(args);
    return fragment;
  }

  @Nullable
  @Override
  public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
      @Nullable Bundle savedInstanceState) {
    PageView pageView = new PageView(requireContext());

    Bundle args = getArguments();
    if (args != null) {
      if (args.containsKey(ARG_JSON_BYTES)) {
        byte[] jsonBytes = args.getByteArray(ARG_JSON_BYTES);
        pageView.setJsonTestCase(jsonBytes);
      } else if (args.containsKey(ARG_TEST_ID)) {
        int testId = args.getInt(ARG_TEST_ID);
        pageView.setCppTestCase(testId);
      }
    }

    return pageView;
  }
}
