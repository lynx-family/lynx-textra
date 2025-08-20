// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_LAYOUT_DRAWER_H_
#define PUBLIC_TEXTRA_LAYOUT_DRAWER_H_

#include <textra/i_typeface_helper.h>
#include <textra/layout_drawer_listener.h>
#include <textra/layout_page_listener.h>
#include <textra/macro.h>
#include <textra/paragraph.h>
#include <textra/text_line.h>
#include <textra/tt_color.h>

#include <memory>

namespace ttoffice {
namespace tttext {
class ICanvasHelper;
class Painter;
class LayoutRegion;
class RunRange;
class Style;
class BlockRegion;
class Border;
class TextAttachment;
class BaseRun;
class PointF;
using DrawerPiece = RunRange;
enum class RadiusDirection : uint8_t;
enum class ParagraphVerticalAlignment : uint8_t;
enum class BorderStyle : uint8_t;

/**
 * @brief Renders text layout results onto a canvas.
 *
 * Takes in a LayoutRegion containing layout results and converts them into
 * ICanvasHelper API calls. The LayoutDrawer logic is platform-independent,
 * while actual rendering is handled by platform-specific ICanvasHelper
 * implementations.
 *
 * It can render various content types, including:
 * - Glyphs positioned and styled correctly (font, color, etc.)
 * - Backgrounds and decorations (e.g. underline)
 */
class L_EXPORT LayoutDrawer {
 public:
  LayoutDrawer() = delete;
  /**
   * @brief Constructs a LayoutDrawer with a backing canvas.
   *
   * Subsequent drawing commands will render to this canvas.
   *
   * @param canvas The platform-specific canvas implementation used for actual
   * drawing.
   */
  explicit LayoutDrawer(ICanvasHelper* canvas);
  ~LayoutDrawer();

 public:
  /**
   * @brief Renders an entire LayoutRegion to the backing canvas.
   *
   * @param layout_page The layout region to render.
   */
  void DrawLayoutPage(LayoutRegion* layout_page);

  /**
   * @brief Renders a specific character range within a text line to the backing
   * canvas.
   *
   * @param i_line The text line to render.
   * @param char_start_in_line The starting character index (inclusive).
   * @param char_end_in_line The ending character index (exclusive).
   */
  void DrawTextLine(TextLine* i_line, uint32_t char_start_in_line,
                    uint32_t char_end_in_line);

  void SetListener(LayoutDrawerListener* listener);

 private:
  void DrawLineBackground(TextLine* i_line, uint32_t char_start_in_para,
                          uint32_t char_end_in_para);
  void DrawLineDecoration(TextLine* i_line, uint32_t char_start_in_para,
                          uint32_t char_end_in_para) const;
  void DrawDrawerPiece(const TextLine* i_line, const DrawerPiece* run_range,
                       uint32_t char_start_in_piece,
                       uint32_t char_end_in_piece) const;
  float DrawTextRun(const BaseRun* run, uint32_t start_char_in_run,
                    uint32_t end_char_in_run, float x, float y, float* pos_x,
                    float* pos_y, float* background_rect) const;
  void DrawGlyphsOrText(const BaseRun* run, uint16_t* glyphs,
                        uint32_t glyph_count, uint32_t glyph_start_index,
                        const ITypefaceHelper* font, float ox, float oy,
                        float* pos_x, float* pos_y, Painter* p) const;
  uint32_t TTColorToPlainColor(ThemeCategory theme, const TTColor& tt_color) {
    return listener_->FetchThemeColor(theme, tt_color);
  }

 private:
  ICanvasHelper* canvas_;
  LayoutDrawerListener* listener_;
};
}  // namespace tttext
}  // namespace ttoffice

#endif  // PUBLIC_TEXTRA_LAYOUT_DRAWER_H_
