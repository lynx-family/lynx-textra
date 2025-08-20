// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.textra;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.Rect;
import android.os.Build;
import java.io.IOException;

public class JavaCanvasHelper {
  protected Canvas canvas_;
  protected final Paint paint_ = new Paint();
  protected JavaFontManager mFontManager = null;

  public JavaCanvasHelper(JavaFontManager fontManager) {
    mFontManager = fontManager;
  }

  public void drawBuffer(Canvas canvas, byte[] input) {
    canvas_ = canvas;
    BBufferInputStream inputStream = new BBufferInputStream(input);
    try {
      while (inputStream.available() > 0) {
        TTTextDefinition.CanvasOp op = TTTextDefinition.GetCanvasOp(inputStream.readByte());
        drawOp(op, inputStream);
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  protected void drawOp(TTTextDefinition.CanvasOp op, BBufferInputStream stream)
      throws IOException {
    switch (op) {
      case kStartPaint:
        startPaint();
        break;
      case kEndPaint:
        endPaint();
        break;
      case kSave:
        save();
        break;
      case kRestore:
        restore();
        break;
      case kClear:
        clear();
        break;
      case kTranslate:
        translate(stream);
        break;
      case kScale:
        scale(stream);
        break;
      case kRotate:
        rotate(stream);
        break;
      case kSkew:
        skew(stream);
        break;
      case kClipRect:
        clipRect(stream);
        break;
      case kClearRect:
        clearRect(stream);
        break;
      case kFillRect:
        fillRect(stream);
        break;
      case kDrawArc:
        drawArc(stream);
        break;
      case kDrawLine:
        drawLine(stream);
        break;
      case kDrawOval:
        drawOval(stream);
        break;
      case kDrawPath:
        drawPath(stream);
        break;
      case kDrawRect:
        drawRect(stream);
        break;
      case kDrawText:
        drawText(stream);
        break;
      case kDrawArcTo:
        drawArcTo(stream);
        break;
      case kDrawColor:
        drawColor(stream);
        break;
      case kDrawImage:
        drawImage(stream);
        break;
      case kDrawCircle:
        drawCircle(stream);
        break;
      case kDrawGlyphs:
        drawGlyphs(stream);
        break;
      case kDrawImageRect:
        drawImgRect(stream);
        break;
      case kDrawRunDelegate:
        drawRunDelegate(stream);
        break;
      case kDrawBackgroundDelegate:
        drawBackgroundDelegate(stream);
        break;
      case kDrawBlockRegion:
        break;
      case kDrawRoundRect:
        drawRoundRect(stream);
    }
  }

  protected void startPaint() throws IOException {}

  protected void endPaint() throws IOException {}

  protected void save() {
    canvas_.save();
  }

  protected void restore() {
    canvas_.restore();
  }

  protected void clear() {
    // TODO: Clear
  }

  protected void translate(BBufferInputStream stream) throws IOException {
    float x = TTTextUtils.Dp2Px(stream.readFloat());
    float y = TTTextUtils.Dp2Px(stream.readFloat());
    canvas_.translate(x, y);
  }

  protected void scale(BBufferInputStream stream) throws IOException {
    float x = stream.readFloat();
    float y = stream.readFloat();
    canvas_.scale(x, y);
  }

  protected void rotate(BBufferInputStream stream) throws IOException {
    float degree = stream.readFloat();
    canvas_.rotate(degree);
  }

  protected void skew(BBufferInputStream stream) throws IOException {
    float x = TTTextUtils.Dp2Px(stream.readFloat());
    float y = TTTextUtils.Dp2Px(stream.readFloat());
    canvas_.skew(x, y);
  }

  protected void clipRect(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    canvas_.clipRect(l, t, r, b);
  }

  protected void clearRect(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    // TODO: ClearRect
  }

  protected void fillRect(BBufferInputStream stream) throws IOException {
    int color = stream.readInt();
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = new Paint();
    p.setColor(color);
    p.setStyle(Paint.Style.FILL);
    canvas_.drawRect(l, t, r, b, p);
  }

  protected void drawColor(BBufferInputStream stream) throws IOException {
    int color = stream.readInt();
    // TODO: DrawColor
  }

  protected void drawLine(BBufferInputStream stream) throws IOException {
    float x1 = TTTextUtils.Dp2Px(stream.readFloat());
    float y1 = TTTextUtils.Dp2Px(stream.readFloat());
    float x2 = TTTextUtils.Dp2Px(stream.readFloat());
    float y2 = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    canvas_.drawLine(x1, y1, x2, y2, p);
  }

  protected void drawRect(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    canvas_.drawRect(l, t, r, b, p);
  }

  protected void drawRoundRect(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    float radii = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      canvas_.drawRoundRect(l, t, r, b, radii, radii, p);
    } else {
      canvas_.drawRect(l, t, r, b, p);
    }
  }

  protected void drawOval(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    //        canvas_.drawOval(l, t, r, b, p);
  }

  protected void drawCircle(BBufferInputStream stream) throws IOException {
    float x = TTTextUtils.Dp2Px(stream.readFloat());
    float y = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    canvas_.drawCircle(x, y, r, p);
  }

  protected void drawArc(BBufferInputStream stream) throws IOException {
    float l = TTTextUtils.Dp2Px(stream.readFloat());
    float t = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    float b = TTTextUtils.Dp2Px(stream.readFloat());
    float start = TTTextUtils.Dp2Px(stream.readFloat());
    float end = TTTextUtils.Dp2Px(stream.readFloat());
    boolean center = stream.readInt() != 0;
    Paint p = ReadPaint(stream, paint_);
  }

  protected void drawPath(BBufferInputStream stream) throws IOException {
    Path path = new Path();
    ReadPath(path, stream);
    path.close();
    Paint paint = ReadPaint(stream, paint_);
    canvas_.drawPath(path, paint);
  }

  protected void drawArcTo(BBufferInputStream stream) throws IOException {
    float x1 = TTTextUtils.Dp2Px(stream.readFloat());
    float y1 = TTTextUtils.Dp2Px(stream.readFloat());
    float x2 = TTTextUtils.Dp2Px(stream.readFloat());
    float y2 = TTTextUtils.Dp2Px(stream.readFloat());
    float x3 = TTTextUtils.Dp2Px(stream.readFloat());
    float y3 = TTTextUtils.Dp2Px(stream.readFloat());
    float r = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
  }

  protected void drawText(BBufferInputStream stream) throws IOException {
    JavaTypeface ttfont = mFontManager.GetTypefaceByIndex(stream.readInt());
    int char_count = stream.readInt();
    if (char_count > text_.length)
      text_ = new char[char_count];
    for (int k = 0; k < char_count; k++) {
      text_[k] = (char) stream.readShort();
    }
    while (char_count > 0 && text_[char_count - 1] < 32) char_count--;

    float x = TTTextUtils.Dp2Px(stream.readFloat());
    float y = TTTextUtils.Dp2Px(stream.readFloat());
    paint_.setTypeface(ttfont.mTypeface);
    Paint p = ReadPaint(stream, paint_);
    p.setFakeBoldText(is_bold_);
    p.setTextSkewX(is_italic_ ? -0.25f : 0f);
    canvas_.drawText(text_, 0, char_count, x, y, p);
  }

  protected void drawGlyphs(BBufferInputStream stream) throws IOException {
    JavaTypeface ttfont = mFontManager.GetTypefaceByIndex(stream.readInt());
    float x = TTTextUtils.Dp2Px(stream.readFloat());
    float y = TTTextUtils.Dp2Px(stream.readFloat());
    int glyph_count = stream.readInt();
    int[] glyph_ids = new int[glyph_count];
    float[] pos = new float[2 * glyph_count];
    for (int k = 0; k < glyph_count; k++) {
      glyph_ids[k] = stream.readShort();
      pos[2 * k] = stream.readFloat() + x;
      pos[2 * k + 1] = stream.readFloat() + y;
    }
  }

  protected void drawRunDelegate(BBufferInputStream stream) throws IOException {
    int id = stream.readInt();
    float dl = TTTextUtils.Dp2Px(stream.readFloat());
    float dt = TTTextUtils.Dp2Px(stream.readFloat());
    float dr = TTTextUtils.Dp2Px(stream.readFloat());
    float db = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
  }

  protected void drawBackgroundDelegate(BBufferInputStream stream) throws IOException {
    int id = stream.readInt();
    Paint p = ReadPaint(stream, paint_);
  }

  protected void drawImage(BBufferInputStream stream) throws IOException {
    int len = stream.readInt();
    byte[] buffer = new byte[len];
    stream.read(buffer, 0, len);
    float dl = TTTextUtils.Dp2Px(stream.readFloat());
    float dt = TTTextUtils.Dp2Px(stream.readFloat());
    float dr = TTTextUtils.Dp2Px(stream.readFloat());
    float db = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    Bitmap img = BitmapFactory.decodeByteArray(buffer, 0, len);
    canvas_.drawBitmap(img, new Rect(0, 0, img.getWidth(), img.getHeight()),
        new Rect((int) dl, (int) dt, (int) dr, (int) db), p);
  }

  protected void drawImgRect(BBufferInputStream stream) throws IOException {
    int len = stream.readInt();
    byte[] buffer = new byte[len];
    stream.read(buffer, 0, len);
    float sl = TTTextUtils.Dp2Px(stream.readFloat());
    float st = TTTextUtils.Dp2Px(stream.readFloat());
    float sr = TTTextUtils.Dp2Px(stream.readFloat());
    float sb = TTTextUtils.Dp2Px(stream.readFloat());
    float dl = TTTextUtils.Dp2Px(stream.readFloat());
    float dt = TTTextUtils.Dp2Px(stream.readFloat());
    float dr = TTTextUtils.Dp2Px(stream.readFloat());
    float db = TTTextUtils.Dp2Px(stream.readFloat());
    Paint p = ReadPaint(stream, paint_);
    Bitmap img = BitmapFactory.decodeByteArray(buffer, 0, len);
    canvas_.drawBitmap(img, new Rect((int) sl, (int) st, (int) sr, (int) sb),
        new Rect((int) dl, (int) dt, (int) dr, (int) db), p);
  }

  protected Paint ReadPaint(BBufferInputStream stream, Paint painter) throws IOException {
    painter.setAntiAlias(true);
    painter.setStrokeWidth(TTTextUtils.Dp2Px(stream.readFloat()));
    color_ = stream.readInt();
    painter.setColor(color_);
    text_size_ = TTTextUtils.Dp2Px(stream.readFloat());
    painter.setTextSize(text_size_);
    int flag = stream.readByte();
    switch (flag & 0x3) {
      case 0:
        painter.setStyle(Paint.Style.FILL);
        break;
      case 1:
        painter.setStyle(Paint.Style.STROKE);
        break;
      case 2:
        painter.setStyle(Paint.Style.FILL_AND_STROKE);
        break;
      case 3:
        break;
    }
    is_bold_ = (flag & (1 << 2)) > 0;
    is_italic_ = (flag & (1 << 3)) > 0;
    is_underline_ = (flag & (1 << 4)) > 0;
    return painter;
  }

  protected void ReadPath(Path path, BBufferInputStream stream) throws IOException {
    TTTextDefinition.PathType type = TTTextDefinition.GetPathType(stream.readInt());
    switch (type) {
      case kLines: {
        int len = stream.readInt();
        for (int i = 0; i < len; i++) {
          float x = TTTextUtils.Dp2Px(stream.readFloat());
          float y = TTTextUtils.Dp2Px(stream.readFloat());
          path.lineTo(x, y);
        }
      } break;
      case kArc: {
        float x1 = TTTextUtils.Dp2Px(stream.readFloat());
        float y1 = TTTextUtils.Dp2Px(stream.readFloat());
        float x2 = TTTextUtils.Dp2Px(stream.readFloat());
        float y2 = TTTextUtils.Dp2Px(stream.readFloat());
        float x3 = TTTextUtils.Dp2Px(stream.readFloat());
        float y3 = TTTextUtils.Dp2Px(stream.readFloat());
        float r = TTTextUtils.Dp2Px(stream.readFloat());
        // TODO:add arc
      } break;
      case kBezier: {
        int len = stream.readInt();
        PointF[] points = new PointF[len];
        for (int i = 0; i < len; i++) {
          float x = TTTextUtils.Dp2Px(stream.readFloat());
          float y = TTTextUtils.Dp2Px(stream.readFloat());
          points[i] = new PointF(x, y);
        }
        if (len == 2) {
          path.quadTo(points[0].x, points[0].y, points[1].x, points[1].y);
        } else if (len >= 3) {
          path.cubicTo(
              points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
        }
      } break;
      case kMoveTo: {
        float x = TTTextUtils.Dp2Px(stream.readFloat());
        float y = TTTextUtils.Dp2Px(stream.readFloat());
        path.moveTo(x, y);
      } break;
      case kMultiPath: {
        int count = stream.readInt();
        for (int i = 0; i < count; i++) {
          ReadPath(path, stream);
        }
      }
    }
  }

  // TODO: b i u
  protected boolean is_bold_;
  protected boolean is_italic_;
  protected boolean is_underline_;

  protected float text_size_;

  protected int color_;
  protected Paint mPainter = new Paint();
  protected char[] text_ = new char[20];
}
