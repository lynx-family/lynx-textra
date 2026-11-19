// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <filesystem>

#include "test/capi/skity_comparison.hpp"
int main(int argc, char** argv) {
  if (argc != 2) {
    std::fprintf(stderr, "Usage: %s OUTPUT_DIRECTORY\n", argv[0]);
    return 2;
  }
  std::filesystem::create_directories(argv[1]);
  return textra_demo::RunSkityComparison(argv[1]) ? 0 : 1;
}
