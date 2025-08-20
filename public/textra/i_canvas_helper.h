// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_I_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_I_CANVAS_HELPER_H_

#include <textra/i_typeface_helper.h>
#include <textra/macro.h>
#include <textra/painter.h>
#include <textra/tt_path.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace ttoffice {
namespace tttext {
class RunDelegate;
constexpr uint32_t FAKE_BOLD_WIDTH() { return 3; }
constexpr float FAKE_BOLD_RATE() { return 1.0f / 20; }
/**
 * @brief Opcode for canvas operations. It allows canvas operations to be
 * serialized and replayed later.
 */
enum class CanvasOp : uint8_t {
  kStartPaint = 0,
  kEndPaint,
  kSave,
  kRestore,
  kTranslate,
  kScale,
  kRotate,
  kSkew,
  kClipRect,
  kClear,
  kClearRect,
  kFillRect,
  kDrawColor,
  kDrawLine,
  kDrawRect,
  kDrawOval,
  kDrawCircle,
  kDrawArc,
  kDrawPath,
  kDrawArcTo,
  kDrawText,
  kDrawGlyphs,
  kDrawRunDelegate,
  kDrawImage,
  kDrawImageRect,
  kDrawBackgroundDelegate,
  kDrawBlockRegion,
  kDrawRoundRect,
};

/**
 * @brief An abstract canvas interface that defines the methods required to
 * render text and graphics.
 *
 * This class decouples font rendering from specific graphics backends.
 * LayoutDrawer calls into this interface to render content. Subclasses can
 * implement platform-specific canvases, enabling text rendering across
 * different backends (e.g., Skia, system graphics APIs).
 */
class ICanvasHelper {
 public:
  ICanvasHelper() = default;
  virtual ~ICanvasHelper() = default;

 public:
  virtual std::unique_ptr<Painter> CreatePainter() = 0;
  virtual void StartPaint() = 0;
  virtual void EndPaint() = 0;

  // draw APIs
  virtual void Save() = 0;
  virtual void Restore() = 0;
  virtual void Translate(float dx, float dy) = 0;
  virtual void Scale(float sx, float sy) = 0;
  virtual void Rotate(float degrees) = 0;
  virtual void Skew(float sx, float sy) = 0;
  virtual void ClipRect(float left, float top, float right, float bottom,
                        bool doAntiAlias) = 0;
  virtual void Clear() = 0;
  virtual void ClearRect(float left, float top, float right, float bottom) = 0;
  virtual void FillRect(float left, float top, float right, float bottom,
                        uint32_t color) = 0;
  virtual void DrawColor(uint32_t color) = 0;
  virtual void DrawLine(float x1, float y1, float x2, float y2,
                        tttext::Painter* painter) = 0;
  virtual void DrawRect(float left, float top, float right, float bottom,
                        tttext::Painter* painter) = 0;
  virtual void DrawOval(float left, float top, float right, float bottom,
                        tttext::Painter* painter) = 0;
  virtual void DrawCircle(float x, float y, float radius,
                          tttext::Painter* painter) = 0;
  virtual void DrawArc(float left, float top, float right, float bottom,
                       float startAngle, float sweepAngle, bool useCenter,
                       tttext::Painter* painter) = 0;
  /*
   * Draw path
   * */
  virtual void DrawPath(Path* path, tttext::Painter* painter) = 0;
  virtual void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                         float end_x, float end_y, float radius,
                         tttext::Painter* painter) = 0;
  virtual void DrawText(const tttext::ITypefaceHelper* font, const char* text,
                        uint32_t text_bytes, float x, float y,
                        tttext::Painter* painter) = 0;
  virtual void DrawGlyphs(const tttext::ITypefaceHelper* font,
                          uint32_t glyph_count, const uint16_t* glyphs,
                          const char* text, uint32_t text_bytes, float origin_x,
                          float origin_y, float* x, float* y,
                          Painter* painter) = 0;
  virtual void DrawRunDelegate(const RunDelegate* delegate, float left,
                               float top, float right, float bottom,
                               tttext::Painter* painter) = 0;
  virtual void DrawBackgroundDelegate(const RunDelegate* delegate,
                                      tttext::Painter* painter) = 0;
  virtual void DrawImage(const char* src, float left, float top, float right,
                         float bottom, tttext::Painter* painter) = 0;

  /*
   * Note: if srcRectPercent is ture and value of src_left/top/right/bottom is
   * less than 1.0f will be treated as Percentage
   */
  virtual void DrawImageRect(const char* src, float src_left, float src_top,
                             float src_right, float src_bottom, float dst_left,
                             float dst_top, float dst_right, float dst_bottom,
                             tttext::Painter* painter,
                             bool srcRectPercent = false) = 0;
  // virtual void DrawBlockRegion(textlayout::BlockRegion* block) {}
  virtual void DrawRoundRect(float left, float top, float right, float bottom,
                             float radius, tttext::Painter* painter) {}
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_I_CANVAS_HELPER_H_
