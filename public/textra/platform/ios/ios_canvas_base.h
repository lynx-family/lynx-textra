// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PLATFORM_IOS_IOS_CANVAS_BASE_H_
#define PUBLIC_TEXTRA_PLATFORM_IOS_IOS_CANVAS_BASE_H_

#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <UIKit/UIKit.h>
#include <textra/i_canvas_helper.h>
#include <textra/platform/ios/typeface_coretext.h>

#include <memory>

using namespace tttext;

class FontManager;
/**
 * @brief A canvas helper implementation backed by Apple's CoreGraphics
 * framework.
 */
class IOSCanvasBase : public tttext::ICanvasHelper {
 public:
  IOSCanvasBase() { context_ = UIGraphicsGetCurrentContext(); }
  explicit IOSCanvasBase(CGContextRef context) : context_(context) {}
  ~IOSCanvasBase() = default;

 public:
  std::unique_ptr<Painter> CreatePainter() override {
    return std::make_unique<Painter>();
  }
  void StartPaint() override {
    if (!CheckValide()) return;
  }
  void EndPaint() override {
    if (!CheckValide()) return;
  }
  void Save() override {
    if (!CheckValide()) return;
    CGContextSaveGState(context_);
  }
  void Restore() override {
    if (!CheckValide()) return;
    CGContextRestoreGState(context_);
  }
  void Translate(float dx, float dy) override {
    if (!CheckValide()) return;
    CGContextTranslateCTM(context_, dx, dy);
  }
  void Scale(float sx, float sy) override {
    if (!CheckValide()) return;
    CGContextScaleCTM(context_, sx, sy);
  }
  void Rotate(float degrees) override {
    if (!CheckValide()) return;
    CGContextRotateCTM(context_, degrees);
  }
  void Skew(float sx, float sy) override {
    if (!CheckValide()) return;
    auto skew_mat = CGAffineTransformConcat(
        CGAffineTransformIdentity, CGAffineTransformMake(1, sy, sx, 1, 0, 0));
    CGContextConcatCTM(context_, skew_mat);
  }
  void ClipRect(float left, float top, float right, float bottom,
                bool doAntiAlias) override {
    if (!CheckValide()) return;
    CGContextClipToRect(context_,
                        CGRectMake(left, top, right - left, bottom - top));
  }
  void Clear() override {
    if (!CheckValide()) return;
    assert(false);
  }
  void ClearRect(float left, float top, float right, float bottom) override {
    if (!CheckValide()) return;
    CGContextClearRect(context_,
                       CGRectMake(left, top, right - left, bottom - top));
  }
  void FillRect(float left, float top, float right, float bottom,
                uint32_t color) override {
    if (!CheckValide()) return;
    CGContextSetRGBFillColor(context_, ((color >> 16) & 0xFF) / 255.0f,
                             ((color >> 8) & 0xFF) / 255.f,
                             (color & 0xFF) / 255.f,
                             ((color >> 24) & 0xFF) / 255.f);
    CGContextFillRect(context_,
                      CGRectMake(left, top, right - left, bottom - top));
  }
  void DrawColor(uint32_t color) override {
    if (!CheckValide()) return;
    CGContextSetRGBFillColor(context_, ((color >> 16) & 0xFF) / 255.0f,
                             ((color >> 8) & 0xFF) / 255.f,
                             (color & 0xFF) / 255.f,
                             ((color >> 24) & 0xFF) / 255.f);
    CGContextFillRect(context_, CGContextGetClipBoundingBox(context_));
  }
  void DrawLine(float x1, float y1, float x2, float y2,
                tttext::Painter* painter) override {
    if (!CheckValide()) return;
    CGContextBeginPath(context_);
    CGContextMoveToPoint(context_, x1, y1);
    CGContextAddLineToPoint(context_, x2, y2);
    CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawRect(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    if (!CheckValide()) return;
    CGContextBeginPath(context_);
    CGContextAddRect(context_,
                     CGRectMake(left, top, right - left, bottom - top));
    CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawRoundRect(float left, float top, float right, float bottom,
                     float radius, tttext::Painter* painter) override {
    if (!CheckValide()) return;
    CGContextBeginPath(context_);
    UIBezierPath* path = [UIBezierPath
        bezierPathWithRoundedRect:CGRectMake(left, top, right - left,
                                             bottom - top)
                     cornerRadius:radius];
    CGContextAddPath(context_, path.CGPath);
    CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawOval(float left, float top, float right, float bottom,
                tttext::Painter* painter) override {
    if (!CheckValide()) return;
    CGContextBeginPath(context_);
    CGContextAddEllipseInRect(
        context_, CGRectMake(left, top, right - left, bottom - top));
    CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawCircle(float x, float y, float radius,
                  tttext::Painter* painter) override {
    if (!CheckValide()) return;
    CGContextBeginPath(context_);
    CGContextAddArc(context_, x, y, radius, 0, 2 * M_PI, 1);
    CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawArc(float left, float top, float right, float bottom,
               float startAngle, float sweepAngle, bool useCenter,
               tttext::Painter* painter) override {
    if (!CheckValide()) return;
    assert(false);
    //  CGContextBeginPath(context_);
    //  CGContextClosePath(context_);
    //  CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void AddPath(Path* path) {
    //  switch (path->type_) {
    //    case ttoffice::PathType::kMoveTo: {
    //      auto *point = reinterpret_cast<ttoffice::MoveToPath *>(path);
    //      CGContextMoveToPoint(context_, point->point_.x_, point->point_.y_);
    //      break;
    //    }
    //    case ttoffice::PathType::kLines: {
    //      auto lines = reinterpret_cast<ttoffice::LinesPath *>(path);
    //      if (lines->points_.empty()) return;
    //      for (auto iter = lines->points_.begin(); iter !=
    //      lines->points_.end(); iter++) {
    //        CGContextAddLineToPoint(context_, iter->x_, iter->y_);
    //      }
    //    } break;
    //    case ttoffice::PathType::kArc: {
    //      auto arc = reinterpret_cast<ttoffice::ArcPath *>(path);
    //      CGContextAddLineToPoint(context_, arc->start_.x_, arc->start_.y_);
    //      CGContextAddArcToPoint(context_, arc->mid_.x_, arc->mid_.y_,
    //      arc->end_.x_, arc->end_.y_,
    //                             arc->radius_);
    //    } break;
    //    case ttoffice::PathType::kBezier: {
    //      auto bezier = reinterpret_cast<ttoffice::BezierPath *>(path);
    //      auto count = bezier->points_.size();
    //      if (count < 3) return;
    //      if (count == 3) {
    //        CGContextAddLineToPoint(context_, bezier->points_[0].x_,
    //        bezier->points_[0].y_); CGContextAddQuadCurveToPoint(context_,
    //        bezier->points_[1].x_, bezier->points_[1].y_,
    //                                     bezier->points_[2].x_,
    //                                     bezier->points_[2].y_);
    //      } else {
    //        CGContextAddLineToPoint(context_, bezier->points_[0].x_,
    //        bezier->points_[0].y_); CGContextAddCurveToPoint(context_,
    //        bezier->points_[1].x_, bezier->points_[1].y_,
    //                                 bezier->points_[2].x_,
    //                                 bezier->points_[2].y_,
    //                                 bezier->points_[3].x_,
    //                                 bezier->points_[3].y_);
    //      }
    //    } break;
    //    case ttoffice::PathType::kMultiPath: {
    //      auto multi = reinterpret_cast<ttoffice::MultiPath *>(path);
    //      for (auto &one_path : multi->path_list_) {
    //        AddPath(one_path.get());
    //      }
    //    } break;
    //    default:
    //      break;
    //  }
  }
  void DrawPath(Path* path, tttext::Painter* painter) override {
    //  if (!CheckValide()) return;
    //  CGContextBeginPath(context_);
    //  AddPath(path);
    //  CGContextClosePath(context_);
    //  CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawArcTo(float start_x, float start_y, float mid_x, float mid_y,
                 float end_x, float end_y, float radius,
                 tttext::Painter* painter) override {
    if (!CheckValide()) return;
    //  CGContextBeginPath(context_);
    CGContextMoveToPoint(context_, start_x, start_y);
    CGContextAddArcToPoint(context_, mid_x, mid_y, end_x, end_y, radius);
    //  CGContextClosePath(context_);
    CGContextSetLineWidth(context_, painter->GetStrokeWidth());
    CGContextDrawPath(context_, ApplyPainterStyle(painter));
  }
  void DrawText(const ITypefaceHelper* void_font, const char* text,
                uint32_t text_bytes, float x, float y,
                tttext::Painter* painter) override {
    if (!CheckValide()) return;
    auto font =
        reinterpret_cast<const TypefaceCoreText*>(void_font)->GetFontRef();
    if (font == nullptr) return;
    CFStringRef cf_text = CFStringCreateWithBytes(
        kCFAllocatorDefault, reinterpret_cast<const UInt8*>(text), text_bytes,
        kCFStringEncodingUTF8, FALSE);
    CFIndex cf_text_count = CFStringGetLength(cf_text);
    UniChar uni_chars[cf_text_count];
    CGGlyph cg_glyphs[cf_text_count];
    CFStringGetCharacters(cf_text, CFRangeMake(0, cf_text_count), uni_chars);
    BOOL ret =
        CTFontGetGlyphsForCharacters(font, uni_chars, cg_glyphs, cf_text_count);
    if (!ret) {
      font =
          CTFontCreateForString(font, cf_text, CFRangeMake(0, cf_text_count));
      ret = CTFontGetGlyphsForCharacters(font, uni_chars, cg_glyphs,
                                         cf_text_count);
    }
    if (!ret) {
      //    LogUtil::W("iOS Canvas CTFontGetGlyphsForCharacters return false");
    }
    uint16_t glyphs[cf_text_count];
    float pos_x[cf_text_count];
    float pos_y[cf_text_count];
    CGSize size;
    for (int i = 0; i < cf_text_count; i++) {
      glyphs[i] = cg_glyphs[i];
      CTFontGetAdvancesForGlyphs(font, kCTFontOrientationDefault, cg_glyphs + i,
                                 &size, 1);
      if (i == 0) {
        pos_x[0] = 0;
      } else {
        pos_x[i] = pos_x[i - 1] + size.width;
      }
      pos_y[i] = 0;
    }
    DrawGlyphs(void_font, (uint32_t)cf_text_count, glyphs, text, text_bytes, x,
               y, pos_x, pos_y, painter);
    CFRelease(cf_text);
  }
  void DrawGlyphs(const ITypefaceHelper* font, uint32_t glyph_count,
                  const uint16_t* glyphs, const char* text, uint32_t text_bytes,
                  float ox, float oy, float* x, float* y,
                  tttext::Painter* painter) override {
    if (!CheckValide()) return;
    if (font == nullptr) return;
    auto ct_font = ((tttext::TypefaceCoreText*)font)->GetFontRef();

    CGGlyph cg_glyphs[glyph_count];
    CGPoint cg_position[glyph_count];
    for (uint32_t i = 0; i < glyph_count; i++) {
      cg_glyphs[i] = glyphs[i];
      cg_position[i].x = x[i];
      cg_position[i].y = y[i];
    }
    auto color = painter->GetColor();
    CGContextSetRGBFillColor(context_, ((color >> 16) & 0xFF) / 255.0f,
                             ((color >> 8) & 0xFF) / 255.f,
                             (color & 0xFF) / 255.f,
                             ((color >> 24) & 0xFF) / 255.f);
    CGContextSaveGState(context_);
    auto text_mat = CGAffineTransformIdentity;
    if (painter->IsBold() || painter->IsItalic()) {
      NSDictionary* traits =
          (__bridge_transfer NSDictionary*)CTFontCopyTraits(ct_font);
      if (painter->IsBold()) {
        NSObject* obj = traits[(id)kCTFontWeightTrait];
        if (obj == nil ||
            ([obj isKindOfClass:[NSNumber class]] &&
             reinterpret_cast<NSNumber*>(obj).doubleValue < 0.001)) {
          // fake bold
          auto size = CTFontGetSize(ct_font);
          CGContextSetTextDrawingMode(context_, kCGTextFillStroke);
          CGContextSetLineWidth(context_, size / 40.0);
          CGContextSetRGBStrokeColor(context_, ((color >> 16) & 0xFF) / 255.0f,
                                     ((color >> 8) & 0xFF) / 255.f,
                                     (color & 0xFF) / 255.f,
                                     ((color >> 24) & 0xFF) / 255.f);
        }
      }
      if (painter->IsItalic()) {
        NSObject* obj = traits[(id)kCTFontSlantTrait];
        if ([obj isKindOfClass:[NSNumber class]]) {
          NSNumber* slant = reinterpret_cast<NSNumber*>(obj);
          if (slant.doubleValue == 0) {
            // add fake slant
            text_mat = CGAffineTransformConcat(
                text_mat, CGAffineTransformMake(1, 0, 0.3, 1, 0, 0));
          }
        }
      }
    }
    CGContextSetTextMatrix(context_, text_mat);
    CGContextTranslateCTM(context_, ox, oy);
    CGContextScaleCTM(context_, 1, -1);
    CTFontDrawGlyphs(ct_font, cg_glyphs, cg_position, glyph_count, context_);
    CGContextRestoreGState(context_);
    if (painter->IsUnderLine()) {
      auto underline_y = CTFontGetUnderlinePosition(ct_font);
      auto thickness = CTFontGetUnderlineThickness(ct_font);
      CGContextBeginPath(context_);
      CGContextMoveToPoint(context_, ox + cg_position[0].x, oy + underline_y);
      CGContextAddLineToPoint(context_, ox + cg_position[glyph_count].x,
                              oy + underline_y);
      CGContextClosePath(context_);
      CGContextSetLineWidth(context_, thickness);
      CGContextSetRGBStrokeColor(context_, ((color >> 16) & 0xFF) / 255.0f,
                                 ((color >> 8) & 0xFF) / 255.f,
                                 (color & 0xFF) / 255.f,
                                 ((color >> 24) & 0xFF) / 255.f);
      CGContextDrawPath(context_, kCGPathStroke);
    }
  }
  void DrawRunDelegate(const RunDelegate* run_delegate, float left, float top,
                       float right, float bottom,
                       tttext::Painter* painter) override {
    //  auto *ios_run_delegate = (IOSRunDelegate *)run_delegate;
    //  if (run_delegate_drawer_ == nullptr) {
    //    TTTextRunDelegateImpl *impl =
    //        (TTTextRunDelegateImpl *)ios_run_delegate->GetRunDelegate();
    //    UIImage *image = (UIImage *)impl.GetAttachment;
    //    CGRect rect = CGRectMake(0, 0, right - left, bottom - top);
    //    CGContextSaveGState(context_);
    //    CGContextTranslateCTM(context_, 0, rect.size.height);
    //    CGContextScaleCTM(context_, image.size.width / rect.size.width,
    //                      -image.size.height / rect.size.height);
    //    CGContextDrawImage(context_, rect, image.CGImage);
    //    CGContextRestoreGState(context_);
    //  } else {
    //    // 这里传的是相对屏幕的绝对rect
    //    CGRect rect = CGRectMake(left, top, right - left, bottom - top);
    //    [run_delegate_drawer_
    //    DrawRunDelegate:ios_run_delegate->GetRunDelegate()
    //                                   InRect:rect];
    //  }
    //  CGContextResetClip(context_);
  }
  void DrawBackgroundDelegate(const RunDelegate* delegate,
                              tttext::Painter* painter) override {}
  void DrawImage(const char* src, float left, float top, float right,
                 float bottom, tttext::Painter* painter) override {
    if (!CheckValide()) return;
  }
  void DrawImageRect(const char* src, float src_left, float src_top,
                     float src_right, float src_bottom, float dst_left,
                     float dst_top, float dst_right, float dst_bottom,
                     tttext::Painter* painter, bool srcRectPercent) override {
    if (!CheckValide()) return;
  }

 protected:
  BOOL CheckValide() { return context_ != NULL; }
  CGPathDrawingMode ApplyPainterStyle(tttext::Painter* painter) {
    CGPathDrawingMode mode = kCGPathFillStroke;
    if (painter->GetFillStyle() == tttext::FillStyle::kFill) {
      mode = kCGPathFill;
      CGContextSetRGBFillColor(
          context_, painter->GetRed() / 255.0f, painter->GetGreen() / 255.f,
          painter->GetBlue() / 255.f, painter->GetAlpha() / 255.f);
    } else if (painter->GetFillStyle() == tttext::FillStyle::kStroke) {
      mode = kCGPathStroke;
      CGContextSetRGBStrokeColor(
          context_, painter->GetRed() / 255.0f, painter->GetGreen() / 255.f,
          painter->GetBlue() / 255.f, painter->GetAlpha() / 255.f);
    } else if (painter->GetFillStyle() == tttext::FillStyle::kStrokeAndFill) {
      mode = kCGPathFillStroke;
      CGContextSetRGBFillColor(
          context_, painter->GetRed() / 255.0f, painter->GetGreen() / 255.f,
          painter->GetBlue() / 255.f, painter->GetAlpha() / 255.f);
      CGContextSetRGBStrokeColor(
          context_, painter->GetRed() / 255.0f, painter->GetGreen() / 255.f,
          painter->GetBlue() / 255.f, painter->GetAlpha() / 255.f);
    }
    return mode;
  }

 protected:
  CGContextRef context_;
};

#endif  // PUBLIC_TEXTRA_PLATFORM_IOS_IOS_CANVAS_BASE_H_
