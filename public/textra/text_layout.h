// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TEXT_LAYOUT_H_
#define PUBLIC_TEXTRA_TEXT_LAYOUT_H_

#include <textra/layout_region.h>
#include <textra/macro.h>
#include <textra/paragraph.h>
#include <textra/style.h>

#include <memory>

namespace ttoffice {
namespace tttext {
class Paragraph;
class LayoutRegion;
class FontmgrCollection;
class TTShaper;
enum ShaperType : uint8_t {
  kSystem,
  kSelfRendering,
};
/**
 * @brief Core class of the text layout engine, containing the core layout logic
 * and connecting all inputs to produce laid out lines of text.
 *
 * Inputs to the layout engine include:
 * - Paragraph, Style, and ParagraphStyle, which together define the text
 * content and its formatting.
 * - LayoutRegion and TextLayoutConfig, which specify the layout area and
 * configuration settings.
 * - A Shaper, which computes the metrics of each glyph.
 * - A TTTextContext, which maintains intermediate layout state during
 * processing.
 *
 * TextLayout implements the core logic for laying out paragraphs within the
 * specified region. This includes positioning glyphs at the correct locations
 * and performing line breaking.
 *
 * The output is a set of text lines stored in a LayoutRegion.
 */
class L_EXPORT TextLayout {
 public:
  explicit TextLayout(FontmgrCollection* font_collection,
                      ShaperType type = kSystem);
  explicit TextLayout(std::unique_ptr<TTShaper> shaper);
  ~TextLayout();
  TextLayout(const TextLayout&) = delete;
  TextLayout& operator=(const TextLayout&) = delete;

 public:
  static bool Preload(ShaperType type);

  // Clears all cached shaping results. New lookups cannot reuse results that
  // were being computed when this method was called.
  static void ClearShapeCache();

  LayoutResult Layout(Paragraph* para, LayoutRegion* page,
                      TTTextContext& context) const {
    return LayoutEx(para, page, context);
  }

  /**
   * Layout interface, can specify the starting position for layout and get
   * layout results Layouts according to the height and width defined by
   * LayoutPage, returns end position and layout results when the page is full
   * Returns kUnsupported for kSingleLine paragraphs; use LayoutSingleLine.
   * @param para [in] paragraph structure to be laid out
   * @param page [in/out] layout page, stores line information after layout
   * @param context [in/out] layout context, stores layout information after *
   * layout
   */
  LayoutResult LayoutEx(Paragraph* para, LayoutRegion* page,
                        TTTextContext& context) const;

  // Lays out a Paragraph::Create(kSingleLine) result after AddTextRun as an
  // unwrapped, one-style LTR line, without a LayoutRegion. No drawing takes
  // place; draw with DrawSingleLine. Returns false for a regular paragraph,
  // empty/invalid text, or unsupported content/styles (controls, backgrounds,
  // decorations, custom painters or unsupported editing operations).
  bool LayoutSingleLine(Paragraph* paragraph, TTTextContext& context) const;

 private:
  std::unique_ptr<TTShaper> shaper_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TEXT_LAYOUT_H_
