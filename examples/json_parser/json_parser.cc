// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "examples/json_parser/json_parser.h"

#include <textra/font_info.h>
#include <textra/layout_definition.h>
#include <textra/layout_region.h>
#include <textra/paragraph_style.h>
#include <textra/text_layout.h>

#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/rapidjson.h"
#include "src/textlayout/paragraph_impl.h"
#include "src/textlayout/tt_shaper.h"

using namespace rapidjson;

namespace {
constexpr float kDefaultTextSize = 30.f;

ttoffice::tttext::Style GetDefaultStyle() {
  ttoffice::tttext::Style style;
  style.SetTextSize(kDefaultTextSize);
  return style;
}

ttoffice::tttext::ParagraphStyle GetDefaultParagraphStyle() {
  ttoffice::tttext::ParagraphStyle para_style;
  para_style.SetDefaultStyle(GetDefaultStyle());
  return para_style;
}
}  // namespace

// Check if a field exists in the JSON Value
bool HasOptionalField(const Value& data, const char* field_name) {
  return data.HasMember(field_name);
}

// Get a required field from JSON Value and convert it to the specified type
template <typename T>
T GetRequiredField(const Value& data, const char* field_name) {
  assert(data.HasMember(field_name) && "Required field not found");
  auto& field_value = data[field_name];
  if constexpr (std::is_same_v<T, bool>) {
    return field_value.GetBool();
  } else if constexpr (std::is_same_v<T, int>) {
    return field_value.GetInt();
  } else if constexpr (std::is_same_v<T, float>) {
    return field_value.GetFloat();
  } else if constexpr (std::is_same_v<T, double>) {
    return field_value.GetDouble();
  } else if constexpr (std::is_same_v<T, std::string>) {
    return field_value.GetString();
  } else if constexpr (std::is_same_v<T, Value::ConstObject>) {
    return field_value.GetObject();
  } else if constexpr (std::is_same_v<T, Value::ConstArray>) {
    return field_value.GetArray();
  }
  throw std::runtime_error("Unsupported type for field: " +
                           std::string(typeid(T).name()));
}

namespace ttoffice {
namespace tttext {
namespace {

// Parse JSON string into a Document object
Document ParseJsonContent(const std::string& content) {
  Document doc;
  ParseResult parse_result =
      doc.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(content.c_str());

  if (parse_result.IsError()) {
    throw std::runtime_error(
        "Unable to parse JSON: " +
        std::string(GetParseError_En(parse_result.Code())));
  }
  return doc;
}

// Convert ARGB hex string to uint32_t color value
uint32_t ParseArgbColor(const std::string& color_str) {
  // Remove the '#' prefix
  std::string hex = color_str;
  if (hex[0] == '#') {
    hex = hex.substr(1);
  }
  // Convert hex string to uint32_t
  return std::stoul(hex, nullptr, 16);
}

// Parse JSON Value into Style object
Style ParseStyle(const Value& style_data) {
  if (!style_data.IsObject()) {
    throw std::runtime_error("Style data must be an object");
  }
  Style style = GetDefaultStyle();

  if (HasOptionalField(style_data, "font_descriptor")) {
    const auto& font_descriptor_data = style_data["font_descriptor"];
    FontDescriptor font_descriptor;

    // Parse font family list
    if (HasOptionalField(font_descriptor_data, "font_family_list")) {
      const auto& family_list =
          font_descriptor_data["font_family_list"].GetArray();
      for (const auto& family : family_list) {
        if (family.IsString()) {
          font_descriptor.font_family_list_.push_back(family.GetString());
        }
      }
    }

    // Parse font style
    if (HasOptionalField(font_descriptor_data, "font_style")) {
      const auto& font_style_data = font_descriptor_data["font_style"];

      // Parse weight
      FontStyle::Weight font_weight = FontStyle::kNormal_Weight;
      if (HasOptionalField(font_style_data, "weight")) {
        auto weight = GetRequiredField<std::string>(font_style_data, "weight");
        if (weight == "kInvisible")
          font_weight = FontStyle::kInvisible_Weight;
        else if (weight == "kThin")
          font_weight = FontStyle::kThin_Weight;
        else if (weight == "kExtraLight")
          font_weight = FontStyle::kExtraLight_Weight;
        else if (weight == "kLight")
          font_weight = FontStyle::kLight_Weight;
        else if (weight == "kNormal")
          font_weight = FontStyle::kNormal_Weight;
        else if (weight == "kMedium")
          font_weight = FontStyle::kMedium_Weight;
        else if (weight == "kSemiBold")
          font_weight = FontStyle::kSemiBold_Weight;
        else if (weight == "kBold")
          font_weight = FontStyle::kBold_Weight;
        else if (weight == "kExtraBold")
          font_weight = FontStyle::kExtraBold_Weight;
        else if (weight == "kBlack")
          font_weight = FontStyle::kBlack_Weight;
        else if (weight == "kExtraBlack")
          font_weight = FontStyle::kExtraBlack_Weight;
        else
          throw std::runtime_error("Invalid font weight value: " + weight);
      }

      // Parse width
      FontStyle::Width font_width = FontStyle::kNormal_Width;
      if (HasOptionalField(font_style_data, "width")) {
        auto width = GetRequiredField<std::string>(font_style_data, "width");
        if (width == "kUltraCondensed")
          font_width = FontStyle::kUltraCondensed_Width;
        else if (width == "kExtraCondensed")
          font_width = FontStyle::kExtraCondensed_Width;
        else if (width == "kCondensed")
          font_width = FontStyle::kCondensed_Width;
        else if (width == "kSemiCondensed")
          font_width = FontStyle::kSemiCondensed_Width;
        else if (width == "kNormal")
          font_width = FontStyle::kNormal_Width;
        else if (width == "kSemiExpanded")
          font_width = FontStyle::kSemiExpanded_Width;
        else if (width == "kExpanded")
          font_width = FontStyle::kExpanded_Width;
        else if (width == "kExtraExpanded")
          font_width = FontStyle::kExtraExpanded_Width;
        else if (width == "kUltraExpanded")
          font_width = FontStyle::kUltraExpanded_Width;
        else
          throw std::runtime_error("Invalid font width value: " + width);
      }

      // Parse slant
      FontStyle::Slant font_slant = FontStyle::kUpright_Slant;
      if (HasOptionalField(font_style_data, "slant")) {
        auto slant = GetRequiredField<std::string>(font_style_data, "slant");
        if (slant == "kUpright") {
          font_slant = FontStyle::kUpright_Slant;
        } else if (slant == "kItalic") {
          font_slant = FontStyle::kItalic_Slant;
        } else if (slant == "kOblique") {
          font_slant = FontStyle::kOblique_Slant;
        } else {
          throw std::runtime_error("Invalid font slant value: " + slant);
        }
      }

      font_descriptor.font_style_ =
          FontStyle(font_weight, font_width, font_slant);
    }

    style.SetFontDescriptor(font_descriptor);
  }

  // Parse TextSize
  if (HasOptionalField(style_data, "text_size")) {
    auto text_size = GetRequiredField<float>(style_data, "text_size");
    style.SetTextSize(text_size);
  }

  // Parse TextScale
  if (HasOptionalField(style_data, "text_scale")) {
    auto text_scale = GetRequiredField<float>(style_data, "text_scale");
    style.SetTextScale(text_scale);
  }

  // Parse ForegroundColor
  if (HasOptionalField(style_data, "foreground_color")) {
    auto fg_color =
        GetRequiredField<std::string>(style_data, "foreground_color");
    style.SetForegroundColor(TTColor(ParseArgbColor(fg_color)));
  }

  // Parse BackgroundColor
  if (HasOptionalField(style_data, "background_color")) {
    auto bg_color =
        GetRequiredField<std::string>(style_data, "background_color");
    style.SetBackgroundColor(TTColor(ParseArgbColor(bg_color)));
  }

  // Parse DecorationColor
  if (HasOptionalField(style_data, "decoration_color")) {
    auto decoration_color =
        GetRequiredField<std::string>(style_data, "decoration_color");
    style.SetDecorationColor(TTColor(ParseArgbColor(decoration_color)));
  }

  // Parse DecorationType
  if (HasOptionalField(style_data, "decoration_type")) {
    auto decoration_type =
        GetRequiredField<std::string>(style_data, "decoration_type");
    if (decoration_type == "kNone") {
      style.SetDecorationType(DecorationType::kNone);
    } else if (decoration_type == "kUnderLine") {
      style.SetDecorationType(DecorationType::kUnderLine);
    } else if (decoration_type == "kOverline") {
      style.SetDecorationType(DecorationType::kOverline);
    } else if (decoration_type == "kLineThrough") {
      style.SetDecorationType(DecorationType::kLineThrough);
    } else {
      throw std::runtime_error("Invalid decoration_type value: " +
                               decoration_type);
    }
  }

  // Parse DecorationStyle
  if (HasOptionalField(style_data, "decoration_style")) {
    auto decoration_style =
        GetRequiredField<std::string>(style_data, "decoration_style");
    if (decoration_style == "kSolid") {
      style.SetDecorationStyle(LineType::kSolid);
    } else if (decoration_style == "kDouble") {
      style.SetDecorationStyle(LineType::kDouble);
    } else if (decoration_style == "kDotted") {
      style.SetDecorationStyle(LineType::kDotted);
    } else if (decoration_style == "kDashed") {
      style.SetDecorationStyle(LineType::kDashed);
    } else if (decoration_style == "kWavy") {
      style.SetDecorationStyle(LineType::kWavy);
    } else {
      throw std::runtime_error("Invalid decoration_style value: " +
                               decoration_style);
    }
  }

  // Parse DecorationThicknessMultiplier
  if (HasOptionalField(style_data, "decoration_thickness_multiplier")) {
    auto thickness =
        GetRequiredField<float>(style_data, "decoration_thickness_multiplier");
    style.SetDecorationThicknessMultiplier(thickness);
  }

  // Parse Bold
  if (HasOptionalField(style_data, "bold")) {
    auto bold = GetRequiredField<bool>(style_data, "bold");
    style.SetBold(bold);
  }

  // Parse Italic
  if (HasOptionalField(style_data, "italic")) {
    auto italic = GetRequiredField<bool>(style_data, "italic");
    style.SetItalic(italic);
  }

  // Parse VerticalAlignment
  if (HasOptionalField(style_data, "vertical_alignment")) {
    auto alignment =
        GetRequiredField<std::string>(style_data, "vertical_alignment");
    if (alignment == "kTop") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kTop);
    } else if (alignment == "kMiddle") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kMiddle);
    } else if (alignment == "kBaseLine") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kBaseLine);
    } else if (alignment == "kBottom") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kBottom);
    } else if (alignment == "kSuperScript") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kSuperScript);
    } else if (alignment == "kSubScript") {
      style.SetVerticalAlignment(CharacterVerticalAlignment::kSubScript);
    } else {
      throw std::runtime_error("Invalid vertical_alignment value: " +
                               alignment);
    }
  }

  // Parse WordSpacing
  if (HasOptionalField(style_data, "word_spacing")) {
    auto spacing = GetRequiredField<float>(style_data, "word_spacing");
    style.SetWordSpacing(spacing);
  }

  // Parse LetterSpacing
  if (HasOptionalField(style_data, "letter_spacing")) {
    auto spacing = GetRequiredField<float>(style_data, "letter_spacing");
    style.SetLetterSpacing(spacing);
  }

  // Parse ForegroundPainter
  if (HasOptionalField(style_data, "foreground_painter")) {
    // NOTE: Not supported yet
  }

  // Parse BackgroundPainter
  if (HasOptionalField(style_data, "background_painter")) {
    // NOTE: Not supported yet
  }

  // Parse WordBreak
  if (HasOptionalField(style_data, "word_break")) {
    auto word_break = GetRequiredField<std::string>(style_data, "word_break");
    if (word_break == "kNormal") {
      style.SetWordBreak(WordBreakType::kNormal);
    } else if (word_break == "kBreakAll") {
      style.SetWordBreak(WordBreakType::kBreakAll);
    } else if (word_break == "kKeepAll") {
      style.SetWordBreak(WordBreakType::kKeepAll);
    } else {
      throw std::runtime_error("Invalid word_break value: " + word_break);
    }
  }

  // Parse BaselineOffset
  if (HasOptionalField(style_data, "baseline_offset")) {
    auto offset = GetRequiredField<float>(style_data, "baseline_offset");
    style.SetBaselineOffset(offset);
  }

  // Parse TextShadowList
  if (HasOptionalField(style_data, "text_shadow_list")) {
    const auto& shadow_list_data = style_data["text_shadow_list"];
    if (shadow_list_data.IsArray()) {
      std::vector<TextShadow> text_shadow_list;
      for (size_t i = 0; i < shadow_list_data.Size(); i++) {
        const auto& shadow_data = shadow_list_data[i];
        if (shadow_data.IsObject()) {
          TextShadow shadow;

          // Parse color
          auto color_str = GetRequiredField<std::string>(shadow_data, "color");
          shadow.color_ = TTColor(ParseArgbColor(color_str));

          // Parse offset
          if (HasOptionalField(shadow_data, "offset")) {
            const auto& offset_data = shadow_data["offset"];
            if (offset_data.IsArray() && offset_data.Size() >= 2) {
              shadow.offset_[0] = offset_data[0].GetFloat();
              shadow.offset_[1] = offset_data[1].GetFloat();
            }
          }

          // Parse blur_radius
          shadow.blur_radius_ =
              GetRequiredField<double>(shadow_data, "blur_radius");

          text_shadow_list.push_back(shadow);
        }
      }
      style.SetTextShadowList(text_shadow_list);
    }
  }

  return style;
}

// Parse JSON Value into ParagraphStyle object
ParagraphStyle ParseParagraphStyle(const Value& style_data) {
  if (!style_data.IsObject()) {
    throw std::runtime_error("Paragraph style data must be an object");
  }

  ParagraphStyle para_style = GetDefaultParagraphStyle();

  // Parse horizontal_alignment
  if (HasOptionalField(style_data, "horizontal_alignment")) {
    auto alignment =
        GetRequiredField<std::string>(style_data, "horizontal_alignment");
    if (alignment == "kLeft") {
      para_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kLeft);
    } else if (alignment == "kCenter") {
      para_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kCenter);
    } else if (alignment == "kRight") {
      para_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kRight);
    } else if (alignment == "kJustify") {
      para_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kJustify);
    } else if (alignment == "kDistributed") {
      para_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kDistributed);
    } else {
      throw std::runtime_error("Invalid horizontal_alignment value: " +
                               alignment);
    }
  }

  // Parse vertical_alignment
  if (HasOptionalField(style_data, "vertical_alignment")) {
    auto alignment =
        GetRequiredField<std::string>(style_data, "vertical_alignment");
    if (alignment == "kTop") {
      para_style.SetVerticalAlign(ParagraphVerticalAlignment::kTop);
    } else if (alignment == "kCenter") {
      para_style.SetVerticalAlign(ParagraphVerticalAlignment::kCenter);
    } else if (alignment == "kBottom") {
      para_style.SetVerticalAlign(ParagraphVerticalAlignment::kBottom);
    } else {
      throw std::runtime_error("Invalid vertical_alignment value: " +
                               alignment);
    }
  }

  // Parse default_style
  if (HasOptionalField(style_data, "default_style")) {
    Style default_style = ParseStyle(style_data["default_style"]);
    para_style.SetDefaultStyle(default_style);
  }

  // Parse indent
  if (HasOptionalField(style_data, "indent")) {
    const auto& indent_data = style_data["indent"];

    if (HasOptionalField(indent_data, "start")) {
      auto start = GetRequiredField<float>(indent_data, "start");
      para_style.SetStartIndentInPx(start);
    }

    if (HasOptionalField(indent_data, "start_chars")) {
      auto start_chars = GetRequiredField<int>(indent_data, "start_chars");
      para_style.SetStartIndentInCharCnt(start_chars);
    }

    if (HasOptionalField(indent_data, "end")) {
      auto end = GetRequiredField<float>(indent_data, "end");
      para_style.SetEndIndentInPx(end);
    }

    if (HasOptionalField(indent_data, "end_chars")) {
      auto end_chars = GetRequiredField<int>(indent_data, "end_chars");
      para_style.SetEndIndentInCharCnt(end_chars);
    }

    if (HasOptionalField(indent_data, "first_line")) {
      auto first_line = GetRequiredField<float>(indent_data, "first_line");
      para_style.SetFirstLineIndentInPx(first_line);
    }

    if (HasOptionalField(indent_data, "first_line_chars")) {
      auto first_line_chars =
          GetRequiredField<int>(indent_data, "first_line_chars");
      para_style.SetFirstLineIndentInCharCnt(first_line_chars);
    }

    if (HasOptionalField(indent_data, "hanging")) {
      auto hanging = GetRequiredField<float>(indent_data, "hanging");
      para_style.SetHangingIndentInPx(hanging);
    }

    if (HasOptionalField(indent_data, "hanging_chars")) {
      auto hanging_chars = GetRequiredField<int>(indent_data, "hanging_chars");
      para_style.SetHangingIndentInCharCnt(hanging_chars);
    }
  }

  // Parse spacing
  if (HasOptionalField(style_data, "spacing")) {
    const auto& spacing_data = style_data["spacing"];

    if (HasOptionalField(spacing_data, "after_px")) {
      auto after_px = GetRequiredField<float>(spacing_data, "after_px");
      para_style.SetSpacingAfterInPx(after_px);
    }

    if (HasOptionalField(spacing_data, "before_px")) {
      auto before_px = GetRequiredField<float>(spacing_data, "before_px");
      para_style.SetSpacingBeforeInPx(before_px);
    }

    if (HasOptionalField(spacing_data, "line_px")) {
      auto line_px = GetRequiredField<float>(spacing_data, "line_px");
      if (HasOptionalField(spacing_data, "line_rule")) {
        auto line_rule =
            GetRequiredField<std::string>(spacing_data, "line_rule");
        if (line_rule == "kAuto") {
          para_style.SetLineHeightInPx(line_px, RulerType::kAtLeast);
        } else if (line_rule == "kAtLeast") {
          para_style.SetLineHeightInPxAtLeast(line_px);
        } else if (line_rule == "kExact") {
          para_style.SetLineHeightInPxExact(line_px);
        } else {
          throw std::runtime_error("Invalid spacing.line_rule value: " +
                                   line_rule);
        }
      }
    } else if (HasOptionalField(spacing_data, "line_percent")) {
      auto line_percent = GetRequiredField<float>(spacing_data, "line_percent");
      para_style.SetLineHeightInPercent(line_percent);
    }

    if (HasOptionalField(spacing_data, "line_space_before_px")) {
      auto line_space_before =
          GetRequiredField<float>(spacing_data, "line_space_before_px");
      para_style.SetLineSpaceBeforePx(line_space_before);
    }

    if (HasOptionalField(spacing_data, "line_space_after_px")) {
      auto line_space_after =
          GetRequiredField<float>(spacing_data, "line_space_after_px");
      para_style.SetLineSpaceAfterPx(line_space_after);
    }
  }

  // Parse write_direction
  if (HasOptionalField(style_data, "write_direction")) {
    auto direction =
        GetRequiredField<std::string>(style_data, "write_direction");
    if (direction == "kAuto") {
      para_style.SetWriteDirection(WriteDirection::kAuto);
    } else if (direction == "kLTR") {
      para_style.SetWriteDirection(WriteDirection::kLTR);
    } else if (direction == "kRTL") {
      para_style.SetWriteDirection(WriteDirection::kRTL);
    } else if (direction == "kTTB" || direction == "kBTT") {
      throw std::runtime_error("kBTT is currently not supported");
    } else {
      throw std::runtime_error("Invalid write_direction value: " + direction);
    }
  }

  // Parse ellipsis
  if (HasOptionalField(style_data, "ellipsis")) {
    auto ellipsis = GetRequiredField<std::string>(style_data, "ellipsis");
    para_style.SetEllipsis(ellipsis.c_str());
  }

  // Parse ellipsis_delegate
  if (HasOptionalField(style_data, "ellipsis_delegate")) {
    // TODO(hfuttyh): Implement ellipsis delegate parsing
  }

  // Parse max_lines
  if (HasOptionalField(style_data, "max_lines")) {
    auto max_lines = GetRequiredField<int>(style_data, "max_lines");
    para_style.SetMaxLines(max_lines);
  }

  // Parse line_height_override
  if (HasOptionalField(style_data, "line_height_override")) {
    auto override = GetRequiredField<bool>(style_data, "line_height_override");
    para_style.SetLineHeightOverride(override);
  }

  if (HasOptionalField(style_data, "half_leading")) {
    auto override = GetRequiredField<bool>(style_data, "half_leading");
    para_style.SetHalfLeading(override);
  }

  if (HasOptionalField(style_data, "enable_text_bounds")) {
    auto override = GetRequiredField<bool>(style_data, "enable_text_bounds");
    para_style.EnableTextBounds(override);
  }

  // Parse overflow_wrap
  if (HasOptionalField(style_data, "overflow_wrap")) {
    auto wrap = GetRequiredField<std::string>(style_data, "overflow_wrap");
    if (wrap == "kNormal") {
      para_style.SetOverflowWrap(OverflowWrap::kNormal);
    } else if (wrap == "kAnywhere") {
      para_style.SetOverflowWrap(OverflowWrap::kAnywhere);
    } else if (wrap == "kBreakWord") {
      para_style.SetOverflowWrap(OverflowWrap::kBreakWord);
    } else {
      throw std::runtime_error("Invalid overflow_wrap value: " + wrap);
    }
  }

  // Parse allow_break_around_punctuation
  if (HasOptionalField(style_data, "allow_break_around_punctuation")) {
    auto allow_break =
        GetRequiredField<bool>(style_data, "allow_break_around_punctuation");
    para_style.AllowBreakAroundPunctuation(allow_break);
  }

  return para_style;
}

// Parse JSON Value into a text run with optional style
std::pair<std::optional<Style>, std::string> ParseTextRun(
    const Value& run_data) {
  if (!run_data.IsObject()) {
    throw std::runtime_error("Run data must be an object");
  }

  std::optional<Style> style;
  auto run_type = GetRequiredField<std::string>(run_data, "type");
  if (run_type == "text") {
    if (HasOptionalField(run_data, "style")) {
      style = ParseStyle(run_data["style"]);
    }
    auto content = GetRequiredField<std::string>(run_data, "content");
    return std::make_pair(style, content);
  }
  return std::make_pair(std::nullopt, "");
}

// Parse JSON Value into a Paragraph object
std::unique_ptr<Paragraph> ParseParagraph(const Value& paragraph_data) {
  if (!paragraph_data.IsObject()) {
    throw std::runtime_error("Paragraph data must be an object");
  }

  auto paragraph = std::make_unique<ParagraphImpl>();
  ttoffice::tttext::ParagraphStyle para_style = GetDefaultParagraphStyle();
  paragraph->SetParagraphStyle(&para_style);

  // Parse paragraph style
  if (HasOptionalField(paragraph_data, "paragraph_style")) {
    ParagraphStyle style =
        ParseParagraphStyle(paragraph_data["paragraph_style"]);
    paragraph->SetParagraphStyle(&style);
  }

  // Parse runs
  if (HasOptionalField(paragraph_data, "runs")) {
    for (const auto& run_data : paragraph_data["runs"].GetArray()) {
      if (HasOptionalField(run_data, "type")) {
        auto run_type = GetRequiredField<std::string>(run_data, "type");
        if (run_type == "text") {
          auto [style, content] = ParseTextRun(run_data);
          paragraph->AddTextRun(style ? &style.value() : nullptr,
                                content.c_str());
        } else {
          // TODO(hfuttyh): implement parsing of non-text runs
        }
      }
    }
  }

  return paragraph;
}

// Parse JSON Value into TTTextContext object
std::unique_ptr<TTTextContext> ParseContext(const Value& data) {
  auto context = std::make_unique<TTTextContext>();

  // try to get paragraphs - layout_configuration
  if (HasOptionalField(data, "paragraphs")) {
    auto paragraphs_data =
        GetRequiredField<Value::ConstArray>(data, "paragraphs");
    for (const auto& paragraph_data : paragraphs_data) {
      if (HasOptionalField(paragraph_data, "layout_configuration")) {
        const auto& layout_config = paragraph_data["layout_configuration"];

        if (HasOptionalField(layout_config, "last_line_can_overflow")) {
          bool last_line_can_overflow =
              GetRequiredField<bool>(layout_config, "last_line_can_overflow");
          context->SetLastLineCanOverflow(last_line_can_overflow);
        }

        if (HasOptionalField(layout_config, "skip_spacing_before_first_line")) {
          bool skip_spacing = GetRequiredField<bool>(
              layout_config, "skip_spacing_before_first_line");
          context->SetSkipSpacingBeforeFirstLine(skip_spacing);
        }
      }
    }
  }

  return context;
}

// Parse JSON Value into a list of Paragraph objects
std::vector<std::unique_ptr<Paragraph>> ParseParagraphList(const Value& doc) {
  auto paragraphs_data = GetRequiredField<Value::ConstArray>(doc, "paragraphs");
  std::vector<std::unique_ptr<Paragraph>> paragraphs;
  for (const auto& paragraph_data : paragraphs_data) {
    paragraphs.push_back(ParseParagraph(paragraph_data));
  }
  return paragraphs;
}

// Parse JSON Value into a list of LayoutRegion objects
std::vector<std::unique_ptr<LayoutRegion>> ParseLayoutRegions(
    const Value& data) {
  if (!HasOptionalField(data, "layout_region")) {
    return {};
  }

  const auto& layout_region_data = data["layout_region"];
  auto regions_data =
      GetRequiredField<Value::ConstArray>(layout_region_data, "regions");
  std::vector<std::unique_ptr<LayoutRegion>> layout_regions;
  for (const auto& region_data : regions_data) {
    auto width = GetRequiredField<float>(region_data, "width");
    auto height = GetRequiredField<float>(region_data, "height");

    auto get_layout_mode = [](const Value& data,
                              const char* field_name) -> LayoutMode {
      if (!HasOptionalField(data, field_name)) {
        return LayoutMode::kDefinite;
      }
      auto mode_str = GetRequiredField<std::string>(data, field_name);
      if (mode_str == "kAtMost") {
        return LayoutMode::kAtMost;
      } else if (mode_str == "kDefinite") {
        return LayoutMode::kDefinite;
      } else if (mode_str == "kIndefinite") {
        return LayoutMode::kIndefinite;
      } else {
        throw std::runtime_error("Invalid layout mode: " + mode_str);
      }
    };

    auto width_mode = get_layout_mode(region_data, "width_mode");
    auto height_mode = get_layout_mode(region_data, "height_mode");

    layout_regions.emplace_back(
        std::make_unique<LayoutRegion>(width, height, width_mode, height_mode));
  }

  return layout_regions;
}

// Parse JSON Value into DebugSettings object
DebugSettings ParseDebugSettings(const Value& data) {
  if (!HasOptionalField(data, "debug_settings")) {
    return {};
  }

  DebugSettings debug_settings;
  const auto& debug_settings_data = data["debug_settings"];
  if (HasOptionalField(debug_settings_data, "draw_line_rectangle")) {
    auto draw_line_rectangle =
        GetRequiredField<bool>(debug_settings_data, "draw_line_rectangle");
    debug_settings.draw_line_rectangle_ = draw_line_rectangle;
  }
  return debug_settings;
}

void ParseModifiers(const Value& data,
                    std::vector<ModifyHAlignParams>& modify_align) {
  if (!HasOptionalField(data, "modifiers")) {
    return;
  }
  const auto& modifiers = data["modifiers"];
  if (!modifiers.IsArray()) {
    return;
  }
  for (auto& item : modifiers.GetArray()) {
    auto type = GetRequiredField<std::string>(item, "type");
    if (type == "ModifyLineHAlign") {
      auto region_index = GetRequiredField<int32_t>(item, "region_index");
      auto line_index = GetRequiredField<int32_t>(item, "line_index");
      auto value = GetRequiredField<std::string>(item, "value");
      auto align = ParagraphHorizontalAlignment::kLeft;
      if (value == "kRight") {
        align = ParagraphHorizontalAlignment::kRight;
      } else if (value == "kCenter") {
        align = ParagraphHorizontalAlignment::kCenter;
      } else if (value == "kJustify") {
        align = ParagraphHorizontalAlignment::kJustify;
      } else if (value == "kDistributed") {
        align = ParagraphHorizontalAlignment::kDistributed;
      }
      modify_align.emplace_back(ModifyHAlignParams{
          .region_index_ = region_index,
          .line_index_ = line_index,
          .alignment_ = align,
      });
    }
  }
}
}  // namespace

// Parse JSON string into TTText C++ objects
std::unique_ptr<JsonDocumentContent> ParseJsonStringIntoTTTextObject(
    const std::string& json_content) {
  // Create JSON Document
  Document doc = ParseJsonContent(json_content);

  // Parse JSON, create C++ objects
  auto paragraphs = ParseParagraphList(doc);
  auto tttext_context = ParseContext(doc);
  auto layout_regions = ParseLayoutRegions(doc);
  auto debug_settings = ParseDebugSettings(doc);

  std::vector<ModifyHAlignParams> modify_align;
  ParseModifiers(doc, modify_align);
  // Return result
  auto result = std::make_unique<JsonDocumentContent>(
      JsonDocumentContent{.paragraphs_ = std::move(paragraphs),
                          .layout_regions_ = std::move(layout_regions),
                          .tttext_context_ = std::move(tttext_context),
                          .modify_align_params_ = std::move(modify_align),
                          .debug_settings_ = debug_settings});
  return result;
}

// Perform layout for JSON document content
std::unique_ptr<LayoutRegion> LayoutJsonDocument(
    JsonDocumentContent* json_doc, FontmgrCollection* font_collection,
    ShaperType shaper_type) {
  constexpr float kDefaultWidth = 1000.f;
  constexpr float kDefaultHeight = 600.f;
  constexpr LayoutMode kDefaultWidthMode = LayoutMode::kAtMost;
  constexpr LayoutMode kDefaultHeightMode = LayoutMode::kAtMost;

  auto& paragraphs = json_doc->paragraphs_;
  auto& layout_regions = json_doc->layout_regions_;
  auto& tttext_context = json_doc->tttext_context_;

  // If layout_regions is empty, use the default layout_region
  std::unique_ptr<LayoutRegion> region;
  if (layout_regions.empty()) {
    region = std::make_unique<LayoutRegion>(
        kDefaultWidth, kDefaultHeight, kDefaultWidthMode, kDefaultHeightMode);
  } else {
    region = std::move(layout_regions.front());
    layout_regions.erase(layout_regions.begin());
  }

  // Create shaper and TextLayout
  auto shaper = TTShaper::CreateShaper(font_collection, shaper_type);
  TextLayout layout(std::move(shaper));

  // Layout each paragraph
  for (auto& paragraph : paragraphs) {
    layout.Layout(paragraph.get(), region.get(), *tttext_context);
    tttext_context->ResetLayoutPosition({0, 0});
  }

  // If the width_mode/height_mode of the region is kAtMost, it means the user
  // wants to shrink the layout region to just fit the paragraphs, so first
  // measure, then create a region of the appropriate size
  LayoutMode width_mode = region->GetWidthMode();
  LayoutMode height_mode = region->GetHeightMode();
  if (width_mode == LayoutMode::kAtMost || height_mode == LayoutMode::kAtMost) {
    const float width = width_mode == LayoutMode::kAtMost
                            ? region->GetLayoutedWidth()
                            : region->GetPageWidth();
    const float height = height_mode == LayoutMode::kAtMost
                             ? region->GetLayoutedHeight()
                             : region->GetPageHeight();
    auto new_layout_region =
        std::make_unique<LayoutRegion>(width, height, width_mode, height_mode);

    // Layout with the new region
    TTTextContext new_context;
    for (auto& paragraph : paragraphs) {
      layout.Layout(paragraph.get(), new_layout_region.get(), new_context);
      new_context.ResetLayoutPosition({0, 0});
    }
    region = std::move(new_layout_region);
  }

  if (!json_doc->modify_align_params_.empty()) {
    for (auto& modify : json_doc->modify_align_params_) {
      if (modify.region_index_ != 0) {
        continue;
      }
      if (modify.line_index_ < 0 ||
          modify.line_index_ >= region->GetLineCount()) {
        continue;
      }
      region->GetLine(modify.line_index_)
          ->ModifyHorizontalAlignment(modify.alignment_);
    }
  }

  return region;
}

}  // namespace tttext
}  // namespace ttoffice
