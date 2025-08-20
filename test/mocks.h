// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef Mocks_h
#define Mocks_h

#include <gmock/gmock.h>
#include <textra/i_canvas_helper.h>
#include <textra/i_typeface_helper.h>

#include "tt_shaper.h"

namespace ttoffice {
namespace tttext {

class MockTTShaper : public TTShaper {
 public:
  explicit MockTTShaper(FontmgrCollection font_collection) noexcept
      : TTShaper(std::move(font_collection)) {}

  MOCK_METHOD(void, OnShapeText, (const ShapeKey&, ShapeResult*), (const));
};

class MockTypefaceHelper : public ITypefaceHelper {
 public:
  explicit MockTypefaceHelper(uint32_t unique_id = 1)
      : ITypefaceHelper(unique_id) {}

  MOCK_METHOD(float, GetHorizontalAdvance, (GlyphID glyph_id, float font_size),
              (const, override));
  MOCK_METHOD(void, GetHorizontalAdvances,
              (GlyphID glyph_ids[], uint32_t count, float widths[],
               float font_size),
              (const, override));
  MOCK_METHOD(void, GetWidthBound,
              (float* rect_ltwh, GlyphID glyph_id, float font_size),
              (const, override));
  MOCK_METHOD(void, GetWidthBounds,
              (float* rect_ltrb, GlyphID glyph_ids[], uint32_t glyph_count,
               float font_size),
              (override));
  MOCK_METHOD(const void*, GetFontData, (), (const, override));
  MOCK_METHOD(size_t, GetFontDataSize, (), (const, override));
  MOCK_METHOD(int, GetFontIndex, (), (const, override));
  MOCK_METHOD(uint16_t, UnicharToGlyph,
              (Unichar codepoint, uint32_t variationSelector),
              (const, override));
  MOCK_METHOD(void, UnicharsToGlyphs,
              (const Unichar* unichars, uint32_t count, GlyphID* glyphs),
              (const, override));
  MOCK_METHOD(uint32_t, GetUnitsPerEm, (), (const, override));
  MOCK_METHOD(void, OnCreateFontInfo, (FontInfo * info, float font_size),
              (const, override));
};

class MockCanvasHelper : public ICanvasHelper {
 public:
  MOCK_METHOD(std::unique_ptr<Painter>, CreatePainter, (), (override));
  MOCK_METHOD(void, StartPaint, (), (override));
  MOCK_METHOD(void, EndPaint, (), (override));

  MOCK_METHOD(void, Save, (), (override));
  MOCK_METHOD(void, Restore, (), (override));
  MOCK_METHOD(void, Translate, (float dx, float dy), (override));
  MOCK_METHOD(void, Scale, (float sx, float sy), (override));
  MOCK_METHOD(void, Rotate, (float degrees), (override));
  MOCK_METHOD(void, Skew, (float sx, float sy), (override));
  MOCK_METHOD(void, ClipRect,
              (float left, float top, float right, float bottom,
               bool doAntiAlias),
              (override));
  MOCK_METHOD(void, Clear, (), (override));
  MOCK_METHOD(void, ClearRect,
              (float left, float top, float right, float bottom), (override));
  MOCK_METHOD(void, FillRect,
              (float left, float top, float right, float bottom,
               uint32_t color),
              (override));
  MOCK_METHOD(void, DrawColor, (uint32_t color), (override));
  MOCK_METHOD(void, DrawLine,
              (float x1, float y1, float x2, float y2,
               tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawRect,
              (float left, float top, float right, float bottom,
               tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawOval,
              (float left, float top, float right, float bottom,
               tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawCircle,
              (float x, float y, float radius, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawArc,
              (float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawPath, (Path * path, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawArcTo,
              (float start_x, float start_y, float mid_x, float mid_y,
               float end_x, float end_y, float radius,
               tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawText,
              (const tttext::ITypefaceHelper* font, const char* text,
               uint32_t text_bytes, float x, float y, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawGlyphs,
              (const tttext::ITypefaceHelper* font, uint32_t glyph_count,
               const uint16_t* glyphs, const char* text, uint32_t text_bytes,
               float origin_x, float origin_y, float* x, float* y,
               Painter* painter),
              (override));
  MOCK_METHOD(void, DrawRunDelegate,
              (const RunDelegate* delegate, float left, float top, float right,
               float bottom, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawBackgroundDelegate,
              (const RunDelegate* delegate, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawImage,
              (const char* src, float left, float top, float right,
               float bottom, tttext::Painter* painter),
              (override));
  MOCK_METHOD(void, DrawImageRect,
              (const char* src, float src_left, float src_top, float src_right,
               float src_bottom, float dst_left, float dst_top, float dst_right,
               float dst_bottom, tttext::Painter* painter, bool srcRectPercent),
              (override));
  MOCK_METHOD(void, DrawRoundRect,
              (float left, float top, float right, float bottom, float radius,
               tttext::Painter* painter),
              (override));
};
}  // namespace tttext
}  // namespace ttoffice
#endif
