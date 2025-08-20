#!/bin/bash
# Copyright 2023 The Lynx Authors. All rights reserved.

gn gen out/gn_testing --args="enable_unittests=true use_flutter_cxx=false" --ide=json
# Convert gn of gn_testing module to gn_testing.podspec
python3 tools/gn_tools/gn_to_podspec.py \
  --json-path out/gn_testing/project.json \
  --start-target //tools/gn_tools/testing:gn_testing_podspec_group \
  --compiler-target //tools/gn_tools/testing:gn_testing_base \
  --sub-target //tools/gn_tools/testing:gn_testing_subspecs_group \
  --podspec-path ./tools/gn_tools/testing/gn_testing.podspec

# Convert gn of gn_testing module to CMakeLists.txt
python3 tools/gn_tools/gn_to_cmake.py \
  --json-path out/gn_testing/project.json \
  --start-target //tools/gn_tools/testing:gn_testing_shared \
  --cmake-version '3.4.0' \
  --project-name 'gn_testing' \
  --compiler-target //tools/gn_tools/testing:gn_testing_base \
  --keep-libs dl \
  --additional-compiler-flags Oz

# Overwrite the old script with the script of the transformation
cp ./out/gn_testing/podspecs/gn_testing.podspec ./tools/gn_tools/testing/gn_testing.podspec
cp ./out/gn_testing/CMakeLists/tools/gn_tools/testing/CMakeLists.txt ./tools/gn_tools/testing/CMakeLists.txt

# print diff
if test "$(git diff --name-only tools/gn_tools/testing/gn_testing.podspec | wc -l)" -gt 0 ; then
  git diff tools/gn_tools/testing/gn_testing.podspec > ./out/gn_testing/podspec_diff.log;
  cat ./out/gn_testing/podspec_diff.log;
fi

# print diff
if test "$(git diff --name-only tools/gn_tools/testing/CMakeLists.txt | wc -l)" -gt 0 ; then
  git diff tools/gn_tools/testing/CMakeLists.txt > ./out/gn_testing/cmake_diff.log;
  cat ./out/gn_testing/cmake_diff.log;
fi

if test "$(git diff --name-only tools/gn_tools/testing/gn_testing.podspec | wc -l)" -gt 0 ; then
  exit 1;
fi

if test "$(git diff --name-only tools/gn_tools/testing/CMakeLists.txt | wc -l)" -gt 0 ; then
  exit 1;
fi
