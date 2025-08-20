// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_region.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "examples/json_parser/json_parser.h"
#include "test/test_utils.h"

using namespace ttoffice::tttext;

// Read file content into a string
std::string ReadFile(const char* file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + std::string(file_path));
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

/**
 * A CLI executable that generates a PNG file from an input JSON file.
 *
 * This program takes a JSON file path as input, parses the JSON into TTText
 * objects (paragraphs, layout regions, context), performs text layout, and
 * renders the layout to a PNG file.
 *
 * See ./specification.json for details on how to write the input JSON file,
 * including allowed fields, types, values, and whether each field is required
 * or optional.
 *
 * Usage: ./program <json_file_path>
 * Output: <json_file_name>.png
 */
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <json_file_path>" << std::endl;
    return 1;
  }

  const char* file_path = argv[1];

  // Read file content
  std::string json_content = ReadFile(file_path);

  // Parse JSON string into TTText C++ objects
  std::unique_ptr<JsonDocumentContent> parsed_text =
      ParseJsonStringIntoTTTextObject(json_content);
  auto& debug_settings = parsed_text->debug_settings_;

  // Create font collection and perform layout
  auto& font_collection = TestUtils::getFontmgrCollection();
  std::unique_ptr<LayoutRegion> region = LayoutJsonDocument(
      parsed_text.get(), &font_collection, ShaperType::kSelfRendering);

  std::string file_path_str(file_path);
  std::string base_name =
      file_path_str.substr(file_path_str.find_last_of("/\\") + 1);
  base_name = base_name.substr(0, base_name.find_last_of('.'));
  std::string output_file = base_name + ".png";

  float bitmap_width = (region->GetWidthMode() == LayoutMode::kDefinite)
                           ? region->GetPageWidth()
                           : region->GetLayoutedWidth();

  float bitmap_height = (region->GetHeightMode() == LayoutMode::kDefinite)
                            ? region->GetPageHeight()
                            : region->GetLayoutedHeight();

  // Create Bitmap canvas
  skity::Bitmap bitmap(bitmap_width, bitmap_height,
                       skity::AlphaType::kPremul_AlphaType,
                       skity::ColorType::kRGBA);

  std::unique_ptr<skity::Canvas> canvas =
      skity::Canvas::MakeSoftwareCanvas(&bitmap);
  canvas->DrawColor(skity::Color_WHITE);
  SkityCanvasHelper canvas_helper(canvas.get());

  // Draw layout page
  TestUtils::DrawLayoutRegionOnCanvas(canvas_helper, *region);

  // Draw line rectangle
  if (debug_settings.draw_line_rectangle_) {
    TestUtils::DrawLineRectOnCanvas(canvas_helper, *region);
  }

  // Save as PNG file
  std::shared_ptr<skity::Codec> codec = skity::Codec::MakePngCodec();
  std::shared_ptr<skity::Data> data = codec->Encode(bitmap.GetPixmap().get());
  data->WriteToFile(output_file.c_str());
  std::cout << "Output file: '" << output_file << "'" << std::endl;
}
