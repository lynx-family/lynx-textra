// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef EXAMPLES_JSON_PARSER_JSON_PARSER_H_
#define EXAMPLES_JSON_PARSER_JSON_PARSER_H_

#include <textra/fontmgr_collection.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/text_layout.h>

#include <memory>
#include <string>
#include <vector>

namespace ttoffice {
namespace tttext {

struct DebugSettings {
  bool draw_line_rectangle_ = false;
};

struct JsonDocumentContent {
  std::vector<std::unique_ptr<Paragraph>> paragraphs_;
  std::vector<std::unique_ptr<LayoutRegion>> layout_regions_;
  std::unique_ptr<TTTextContext> tttext_context_;
  DebugSettings debug_settings_;
};

/**
 * Parses a JSON string into TTText C++ objects. This enables easy creation of
 * examples and test cases using JSON files, without writing C++ code or
 * recompiling.
 *
 * See ./specification.json for details on how to write the input JSON file,
 * including allowed fields, types, values, and whether each field is required
 * or optional.
 *
 * Clients can use the parsed objects as needed, such as in demo apps or
 * exporting them as images.
 *
 * @param json_content JSON string content to parse.
 * @return TTText C++ objects parsed from the JSON string.
 */
std::unique_ptr<JsonDocumentContent> ParseJsonStringIntoTTTextObject(
    const std::string& json_content);

/**
 * Performs layout for a parsed JSON document content.
 *
 * This function uses the paragraphs, layout regions, and context information in
 * json_doc, together with the provided font collection and shaper type, to
 * layout all paragraphs and returns the final LayoutRegion object.
 *
 * If json_doc->layout_regions_ is empty, a default layout region will be
 * created. If the layout region's width_mode or height_mode is kAtMost, the
 * region size will be automatically adjusted to fit the content.
 *
 * @param json_doc The parsed JSON document content.
 * @param font_collection The font collection to use for layout.
 * @param shaper_type The type of shaper to use for layout.
 * @return The layout region after layout is complete.
 */
std::unique_ptr<LayoutRegion> LayoutJsonDocument(
    JsonDocumentContent* json_doc, FontmgrCollection* font_collection,
    ShaperType shaper_type);

}  // namespace tttext
}  // namespace ttoffice

#endif  // EXAMPLES_JSON_PARSER_JSON_PARSER_H_
