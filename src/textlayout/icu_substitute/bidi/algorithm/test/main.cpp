// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bidi_wrapper.h"

std::vector<std::string> readFile(std::string txt_path) {
  std::vector<std::string> contents;
  std::ifstream fin(txt_path);
  std::string s;
  while (getline(fin, s)) {
    contents.push_back(s);
  }
  return contents;
}

std::vector<std::string> decodeTestFile(std::vector<std::string>& test_file) {
  std::vector<std::string> vec;
  for (auto line : test_file) {
    std::string str = "";
    for (auto ch : line) {
      if (ch == '/') {
        continue;
      }
      str += ch;
    }
    vec.push_back(str);
  }
  return vec;
}

std::vector<std::vector<int>> decodeGroundTruth(
    std::vector<std::string>& ground_truth) {
  std::vector<std::vector<int>> vec;

  for (auto line : ground_truth) {
    std::string curr = "";
    std::vector<int> v;
    for (auto ch : line) {
      if (ch == ' ') {
        if (curr == "x") {
          v.push_back(-1);
          curr = "";
        } else if (curr.length() != 0) {
          v.push_back(stoi(curr));
          curr = "";
        }
      } else {
        curr += ch;
      }
    }
    if (curr.length() != 0) {
      if (curr == "x") {
        v.push_back(-1);
      } else {
        v.push_back(stoi(curr));
      }
    }
    vec.push_back(v);
  }
  return vec;
}

bool compareVecs(std::vector<int>& vec1, std::vector<uint32_t>& vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }
  for (int i = 0; i < vec1.size(); i++) {
    if (vec1[i] < 0) {
      continue;
    }
    if (vec1[i] != vec2[i]) {
      return false;
    }
  }
  return true;
}

bool compareVecs(std::vector<int>& vec1, std::vector<int32_t>& vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }
  for (int i = 0; i < vec1.size(); i++) {
    if (vec1[i] != vec2[i]) {
      return false;
    }
  }
  return true;
}

std::vector<bool> compare2GT(std::vector<int>& gtlvl, std::vector<int>& gtorder,
                             std::vector<short>& bdlvl,
                             std::vector<uint32_t>& bdorder) {
  std::vector<bool> x_pos;
  for (int i = 0; i < gtlvl.size(); i++) {
    if (gtlvl[i] != -1) {
      x_pos.push_back(false);
    } else {
      x_pos.push_back(true);
    }
  }
  bool lvl_cmp = true;
  for (int i = 0; i < bdlvl.size(); i++) {
    if (x_pos[i]) {
      continue;
    }
    if (gtlvl[i] != bdlvl[i]) {
      lvl_cmp = false;
      break;
    }
  }
  bool order_cmp = true;
  int index = 0;
  for (int i = 0; i < bdorder.size(); i++) {
    if (x_pos[bdorder[i]]) {
      continue;
    }
    if (gtorder[index++] != bdorder[i]) {
      order_cmp = false;
      break;
    }
  }
  return {lvl_cmp, order_cmp};
}

struct params {
  std::string test;
  std::string levels;
  std::string reorder;
  short bitset;
  params(std::string t, std::string l, std::string r, short b) {
    test = t;
    levels = l;
    reorder = r;
    bitset = b;
  }
};

std::vector<params> parameters = {
    params(
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.1.test.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.1.levels.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.1.reorder.txt",
        Bidi::LEVEL_DEFAULT_LTR),
    params(
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.2.test.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.2.levels.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.2.reorder.txt",
        Bidi::LTR),
    params(
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.4.test.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.4.levels.txt",
        "/Users/admin/Desktop/textlayout/src/bidi/test/BidiTest.4.reorder.txt",
        Bidi::RTL)};

void runTest(int bitset) {
  printf("\nTest case is bitset %s \n",
         bitset == 0 ? "Bidi::LEVEL_DEFAULT_LTR"
                     : (bitset == 1 ? "Bidi::LTR" : "Bidi::RTL"));
  auto parameter = parameters[bitset];
  auto test = readFile(parameter.test);
  auto level = readFile(parameter.levels);
  auto reorder = readFile(parameter.reorder);

  auto test_lines = decodeTestFile(test);
  auto level_lines = decodeGroundTruth(level);
  auto reorder_lines = decodeGroundTruth(reorder);
  std::cout << "total size:" << test_lines.size() << std::endl;

  test.clear();
  level.clear();
  reorder.clear();
  int cnt_not_x_test_cases = 0;
  int cnt_lvl_hit_in_not_x_test_cases = 0;
  int cnt_reorder_hit_in_not_x_test_cases = 0;
  int cnt_lvl = 0;
  int cnt_reorder = 0;
  int java_code_length = 0;
  for (int i = 0; i < test_lines.size(); i++) {
    auto test_line = test_lines[i];
    auto gt_level = level_lines[i];
    auto gt_order = reorder_lines[i];

    bool is_x_cases = false;
    for (auto n : gt_level) {
      if (n == -1) {
        is_x_cases = true;
        break;
      }
    }
    //    if(is_x_cases){
    //      continue;
    //    }
    if (!is_x_cases) {
      cnt_not_x_test_cases += 1;
    }
    auto& utils = BidiUtils::GetInstance();
    utils.SetPara(test_line, parameter.bitset);
    auto lvl = utils.GetLevels();
    auto order = utils.GetVisualMap();
    auto cmp_res = compare2GT(gt_level, gt_order, lvl, order);
    // bool lvl_cmp = compareVecs(gt_level,lvl);
    // bool order_cmp = compareVecs(gt_order,order);
    bool lvl_cmp = cmp_res[0];
    bool order_cmp = cmp_res[1];
    if (lvl_cmp) {
      cnt_lvl += 1;
      if (!is_x_cases) {
        cnt_lvl_hit_in_not_x_test_cases += 1;
      }
    }

    if (order_cmp) {
      cnt_reorder += 1;
      if (!is_x_cases) {
        cnt_reorder_hit_in_not_x_test_cases += 1;
      }
    }
    if (!order_cmp) {
      std::cout << "test case: " << test_line << std::endl;
      std::cout << "index: " << i << std::endl;
      std::cout << "ground truth levels:";
      for (auto l : gt_level) {
        std::cout << l << " ";
      }
      std::cout << std::endl;
      std::cout << "groud truth orders:";
      for (auto l : gt_order) {
        std::cout << l << " ";
      }
      std::cout << std::endl;
      std::cout << "bidi levels:";
      for (auto l : lvl) {
        std::cout << l << " ";
      }
      std::cout << std::endl;
      std::cout << "bidi orders:";
      for (auto l : order) {
        std::cout << l << " ";
      }
      std::cout << std::endl;
      std::cout << std::endl;
    }
  }
  std::cout << "levels hit:" << cnt_lvl << std::endl;
  std::cout << "levels hit in test cases without x:"
            << cnt_lvl_hit_in_not_x_test_cases << std::endl;
  std::cout << "reorder hit:" << cnt_reorder << std::endl;
  std::cout << "reorder hit in test cases without x:"
            << cnt_reorder_hit_in_not_x_test_cases << std::endl;
  std::cout << "test cases without x:" << cnt_not_x_test_cases << std::endl;
  std::cout << "test case total count:" << test_lines.size() << std::endl;
}

void runSimpleTest(std::string text, int bitset) {
  auto& utils = BidiUtils::GetInstance();
  utils.SetPara(text, bitset);
  auto lvl = utils.GetLevels();
  auto order = utils.GetVisualMap();
  std::cout << "bidi levels:";
  for (auto l : lvl) {
    std::cout << l << " ";
  }
  std::cout << std::endl;
  std::cout << "bidi orders:";
  for (auto l : order) {
    std::cout << l << " ";
  }
  std::cout << std::endl;
}

int main() {
  //  runSimpleTest("TWHB",Bidi::LEVEL_DEFAULT_LTR);
  //  runTest(0);
  //  std::cout<<"-----------------------------------------------"<<std::endl;
  //  runTest(1);
  //  std::cout<<"-----------------------------------------------"<<std::endl;
  //  runTest(2);
  return 0;
}
/*
 * test case: TWHB
index: 241773
ground truth levels:1 1 1 1
groud truth orders:3 2 1 0
bidi levels:0 0 0 0
bidi orders:0 1 2 3
 */
