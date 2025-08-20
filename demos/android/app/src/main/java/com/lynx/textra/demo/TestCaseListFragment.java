// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra.demo;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentTransaction;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class TestCaseListFragment extends Fragment {
  private JsonAssetManager jsonDataManager;
  private List<TestItem> testItems;

  public static abstract class TestItem {}
  public static class CppTestItem extends TestItem {
    public final int id;
    public final String name;
    public CppTestItem(int id, String name) {
      this.id = id;
      this.name = name;
    }
    @Override
    public String toString() {
      return String.format("%d: %s (cpp)", id, name);
    }
  }
  public static class JsonTestItem extends TestItem {
    public final int id;
    public final String fileName;
    public JsonTestItem(int id, String fileName) {
      this.fileName = fileName;
      this.id = id;
    }
    @Override
    public String toString() {
      return String.format("%d: %s", id, fileName);
    }
  }

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    jsonDataManager = new JsonAssetManager(requireContext());
  }

  @Nullable
  @Override
  public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
      @Nullable Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_test_case_list, container, false);
  }

  @Override
  public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    initializeTestItems();

    // Initialize Test Case List View
    ListView listView = view.findViewById(R.id.test_case_list);

    ArrayAdapter<TestItem> adapter =
        new ArrayAdapter<>(requireContext(), android.R.layout.simple_list_item_1, testItems);
    listView.setAdapter(adapter);

    listView.setOnItemClickListener((parent, view1, position, id) -> {
      TestItem selectedItem = testItems.get(position);
      handleClicked(selectedItem);
    });
  }

  private void initializeTestItems() {
    testItems = new ArrayList<>();

    TestCaseListFragment.CppTestItem[] testCases = PageView.getCppTestList();
    for (TestCaseListFragment.CppTestItem testCase : testCases) {
      testItems.add(testCase);
    }

    List<String> jsonFiles = jsonDataManager.getFiles();
    for (int i = 0; i < jsonFiles.size(); i++) {
      String fileName = jsonFiles.get(i);
      JsonTestItem jsonTestItem = new JsonTestItem(i, fileName);
      testItems.add(jsonTestItem);
    }
  }

  private void handleClicked(TestItem testItem) {
    if (testItem instanceof CppTestItem) {
      handleCppTestClicked((CppTestItem) testItem);
    } else if (testItem instanceof JsonTestItem) {
      handleJsonTestClicked((JsonTestItem) testItem);
    }
  }

  private void handleCppTestClicked(CppTestItem cppTestItem) {
    String title = String.format("%d: %s", cppTestItem.id, cppTestItem.name);
    navigateToPageView(PageViewFragment.newInstance(cppTestItem.id), title);
  }

  private void handleJsonTestClicked(JsonTestItem jsonTestItem) {
    byte[] jsonBytes = null;
    try {
      jsonBytes = jsonDataManager.readFile(jsonTestItem.fileName);
    } catch (IOException e) {
      android.util.Log.w(
          "TestCaseListFragment", "Error handling JSON test: " + jsonTestItem.fileName, e);
      return;
    }
    navigateToPageView(PageViewFragment.newInstance(jsonBytes), jsonTestItem.fileName);
  }

  private void navigateToPageView(PageViewFragment pageViewFragment, String title) {
    requireActivity()
        .getSupportFragmentManager()
        .beginTransaction()
        .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
        .replace(R.id.content_frame, pageViewFragment)
        .addToBackStack(null)
        .commit();

    FragmentActivity activity = requireActivity();
    if (activity instanceof AppCompatActivity) {
      AppCompatActivity appCompatActivity = (AppCompatActivity) activity;
      appCompatActivity.getSupportActionBar().setTitle(title);
      appCompatActivity.getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    }
  }
}
