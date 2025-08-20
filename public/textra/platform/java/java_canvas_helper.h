// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_CANVAS_HELPER_H_
#define PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_CANVAS_HELPER_H_

#include <jni.h>
#include <textra/i_canvas_helper.h>
#include <textra/platform/java/buffer_output_stream.h>
#include <textra/platform/java/scoped_global_ref.h>
#include <textra/platform/java/tttext_jni_proxy.h>

#include <memory>
/**
 * @brief A canvas helper implementation that writes serialized drawing commands
 * to a BufferOutputStream.
 */
namespace ttoffice {
namespace tttext {
class JavaCanvasHelper : public ICanvasHelper {
 public:
  JavaCanvasHelper() = default;
  ~JavaCanvasHelper() override = default;

 public:
  std::unique_ptr<Painter> CreatePainter() override {
    return std::make_unique<Painter>();
  }

  void StartPaint() override {}

  void EndPaint() override {}

  void Save() override { stream_.Write(static_cast<uint8_t>(CanvasOp::kSave)); }

  void Restore() override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kRestore));
  }

  void Translate(float dx, float dy) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kTranslate));
    stream_.Write(dx);
    stream_.Write(dy);
  }

  void Scale(float sx, float sy) override {}

  void Rotate(float degrees) override {}

  void Skew(float sx, float sy) override {}

  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    stream_.Write(static_cast<uint8_t>(tttext::CanvasOp::kClipRect));
    stream_.Write(left);
    stream_.Write(top);
    stream_.Write(right);
    stream_.Write(bottom);
  }

  void Clear() override {}

  void ClearRect(float left, float top, float right, float bottom) override {}

  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {}
  void DrawColor(uint32_t color) override {}
  void DrawLine(float x1, float y1, float x2, float y2,
                Painter* painter) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kDrawLine));
    stream_.Write(x1);
    stream_.Write(y1);
    stream_.Write(x2);
    stream_.Write(y2);
    WritePaint(painter);
  }
  void DrawRect(float left, float top, float right, float bottom,
                Painter* painter) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kDrawRect));
    stream_.Write(left);
    stream_.Write(top);
    stream_.Write(right);
    stream_.Write(bottom);
    WritePaint(painter);
  }
  void DrawOval(float left, float top, float right, float bottom,
                Painter* painter) override {}
  void DrawCircle(float x, float y, float radius, Painter* painter) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kDrawCircle));
    stream_.Write(x);
    stream_.Write(y);
    stream_.Write(radius);
    WritePaint(painter);
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               Painter* painter) override {}
  void DrawPath(Path* path, Painter* painter) override {}

  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 Painter* painter) override {}

  void DrawText(const ITypefaceHelper* font, const char* text,
                uint32_t text_bytes, float x, float y,
                Painter* painter) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kDrawText));
    stream_.WriteInt64(font->GetUniqueId());
    stream_.Write(x);
    stream_.Write(y);
    WritePaint(painter);
  }
  void DrawGlyphs(const ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float ox, float oy, float* x, float* y,
                  Painter* painter) override {
    for (auto k = 0; k < glyph_count; k++) x[k] += ox;
    stream_.Write((uint8_t)tttext::CanvasOp::kDrawText);
    stream_.Write(font->GetUniqueId());
    stream_.WriteInt32(glyph_count);
    bool rtl = text_bytes == 1;
    for (auto k = 0; k < glyph_count; k++) {
      auto rk = rtl ? glyph_count - 1 - k : k;
      stream_.Write(glyphs[rk]);
    }
    stream_.Write(*x);
    stream_.Write(oy);
    WritePaint(painter);
  }
  void DrawRunDelegate(const RunDelegate* delegate, float left, float top,
                       float right, float bottom, Painter* painter) override {
    //    auto* java_delegate = (TTTextRunDelegate*)delegate;
    //    stream_.WriteInt32((uint8_t)CanvasOp::kDrawRunDelegate);
    //    stream_.WriteInt32(java_delegate->GetID());
    //    stream_.WriteFloat(left);
    //    stream_.WriteFloat(top);
    //    stream_.WriteFloat(right);
    //    stream_.WriteFloat(bottom);
    //    WritePaint(painter);
  }
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              Painter* painter) override {}

  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, Painter* painter) override {}

  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     Painter* painter, bool srcRectPercent) override {}

  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, Painter* painter) override {
    stream_.Write(static_cast<uint8_t>(CanvasOp::kDrawRoundRect));
    stream_.Write(left);
    stream_.Write(top);
    stream_.Write(right);
    stream_.Write(bottom);
    stream_.Write(radius);
    WritePaint(painter);
  }

  void ClearBuffer() { stream_.Clear(); }
  BufferOutputStream& GetBuffer() { return stream_; }

 private:
  void WritePaint(Painter* painter) {
    stream_.WriteFloat(painter->GetStrokeWidth());
    stream_.WriteInt32(painter->GetColor());
    stream_.WriteFloat(painter->GetTextSize());
    int8_t flag = 0;
    switch (painter->GetFillStyle()) {
      case FillStyle::kFill:
        break;
      case FillStyle::kStroke:
        flag = flag | 1;
        break;
      case FillStyle::kStrokeAndFill:
        flag = flag | 2;
        break;
    }
    if (painter->IsBold()) {
      flag = flag | (1 << 2);
    }
    if (painter->IsItalic()) {
      flag = flag | (1 << 3);
    }
    if (painter->IsUnderLine()) {
      flag = flag | (1 << 4);
    }
    stream_.WriteInt8(flag);
  }

 private:
  BufferOutputStream stream_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_PLATFORM_JAVA_JAVA_CANVAS_HELPER_H_
