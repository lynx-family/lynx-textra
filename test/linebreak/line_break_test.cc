// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <cstdio>

#include "boundary_analyst.h"
#include "u_8_string.h"

const char* GetBoundaryTypeString(ttoffice::textlayout::BoundaryType type) {
  switch (type) {
    case ttoffice::textlayout::BoundaryType::kGraphme:
      return "graphme";
    case ttoffice::textlayout::BoundaryType::kWord:
      return "word";
    case ttoffice::textlayout::BoundaryType::kLineBreakable:
      return "linebreakable";
    case ttoffice::textlayout::BoundaryType::kMustLineBreak:
      return "mustlinebreak";
    case ttoffice::textlayout::BoundaryType::kParagraph:
      return "paragraph";
    case ttoffice::textlayout::BoundaryType::kNone:
    default:
      return "none";
  }
}

std::string BoundaryToString(
    const ttoffice::textlayout::BoundaryAnalyst& boundary_analyst,
    const std::u32string& string) {
  std::string out;
  for (size_t i = 0; i < string.length(); i++) {
    if (string[i] == U' ') {
      out += "space";
    } else if (string[i] == U'\n') {
      out += "\\n";
    } else if (string[i] == U'\r') {
      out += "\\r";
    } else {
      out += ttoffice::base::U32StringToU8(string.data() + i, 1);
    }
    out += " ";
    out += GetBoundaryTypeString(boundary_analyst.GetBoundaryType(i));
    out += "\n";
  }
  return out;
}

int main(int argc, const char** argv) {
  if (argc < 3) {
    return -1;
  } else {
    auto* input = argv[1];
    auto* output = argv[2];
    FILE* inputf = fopen(input, "r+");
    std::string content;
    if (inputf != nullptr) {
      fseek(inputf, 0, SEEK_END);
      auto size = ftell(inputf);
      rewind(inputf);
      content.resize(size);
      fread(content.data(), 1, size, inputf);
    }
    fclose(inputf);
    auto char32s =
        ttoffice::base::U8StringToU32(content.data(), content.size());
    ttoffice::textlayout::BoundaryAnalyst boundary_analyst(char32s.c_str(),
                                                           char32s.length());
    FILE* file = fopen(output, "w+");
    if (file != nullptr) {
      auto string = BoundaryToString(boundary_analyst, char32s);
      fwrite(string.c_str(), 1, string.length(), file);
    }
    fclose(file);
  }
  return 0;
}
