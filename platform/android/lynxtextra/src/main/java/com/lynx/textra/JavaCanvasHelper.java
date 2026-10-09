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
  private final BBufferInputStream inputStream = new BBufferInputStream();
  protected Canvas canvas_;
  protected final Paint paint_ = new Paint();
  protected JavaFontManager mFontManager = null;

  public JavaCanvasHelper(JavaFontManager fontManager) {
    mFontManager = fontManager;
  }

  public void drawBuffer(Canvas canvas, byte[] input) {
    canvas_ = canvas;
    inputStream.setBuffer(input);
    try {
      while (inputStream.available() > 0) {
        int op = inputStream.readByte();
        drawOp(op, inputStream);
        if (op == TTTextDefinition.CanvasOp.END_PAINT) {
          break;
        }
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  protected void drawOp(int op, BBufferInputStream stream) throws IOException {
    switch (op) {
      case TTTextDefinition.CanvasOp.START_PAINT:
        startPaint();
        break;
      case TTTextDefinition.CanvasOp.END_PAINT:
        endPaint();
        break;
      case TTTextDefinition.CanvasOp.SAVE:
        save();
        break;
      case TTTextDefinition.CanvasOp.RESTORE:
        restore();
        break;
      case TTTextDefinition.CanvasOp.CLEAR:
        clear();
        break;
      case TTTextDefinition.CanvasOp.TRANSLATE:
        translate(stream);
        break;
      case TTTextDefinition.CanvasOp.SCALE:
        scale(stream);
        break;
      case TTTextDefinition.CanvasOp.ROTATE:
        rotate(stream);
        break;
      case TTTextDefinition.CanvasOp.SKEW:
        skew(stream);
        break;
      case TTTextDefinition.CanvasOp.CLIP_RECT:
        clipRect(stream);
        break;
      case TTTextDefinition.CanvasOp.CLEAR_RECT:
        clearRect(stream);
        break;
      case TTTextDefinition.CanvasOp.FILL_RECT:
        fillRect(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_ARC:
        drawArc(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_LINE:
        drawLine(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_OVAL:
        drawOval(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_PATH:
        drawPath(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_RECT:
        drawRect(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_TEXT:
        drawText(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_ARC_TO:
        drawArcTo(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_COLOR:
        drawColor(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_IMAGE:
        drawImage(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_CIRCLE:
        drawCircle(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_GLYPHS:
        drawGlyphs(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_IMAGE_RECT:
        drawImgRect(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_RUN_DELEGATE:
        drawRunDelegate(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_BACKGROUND_DELEGATE:
        drawBackgroundDelegate(stream);
        break;
      case TTTextDefinition.CanvasOp.DRAW_BLOCK_REGION:
        break;
      case TTTextDefinition.CanvasOp.DRAW_ROUND_RECT:
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
    float x = stream.readFloat();
    float y = stream.readFloat();
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
    float x = stream.readFloat();
    float y = stream.readFloat();
    canvas_.skew(x, y);
  }

  protected void clipRect(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    canvas_.clipRect(l, t, r, b);
  }

  protected void clearRect(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    // TODO: ClearRect
  }

  protected void fillRect(BBufferInputStream stream) throws IOException {
    int color = stream.readInt();
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
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
    float x1 = stream.readFloat();
    float y1 = stream.readFloat();
    float x2 = stream.readFloat();
    float y2 = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      canvas_.drawLine(x1, y1, x2, y2, p);
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        canvas_.drawLine(x1, y1, x2, y2, p);
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        canvas_.drawLine(x1, y1, x2, y2, p);
      }
    }
  }

  protected void drawRect(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      canvas_.drawRect(l, t, r, b, p);
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        canvas_.drawRect(l, t, r, b, p);
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        canvas_.drawRect(l, t, r, b, p);
      }
    }
  }

  protected void drawRoundRect(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    float radii = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        canvas_.drawRoundRect(l, t, r, b, radii, radii, p);
      } else {
        canvas_.drawRect(l, t, r, b, p);
      }
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
          canvas_.drawRoundRect(l, t, r, b, radii, radii, p);
        } else {
          canvas_.drawRect(l, t, r, b, p);
        }
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
          canvas_.drawRoundRect(l, t, r, b, radii, radii, p);
        } else {
          canvas_.drawRect(l, t, r, b, p);
        }
      }
    }
  }

  protected void drawOval(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    //        canvas_.drawOval(l, t, r, b, p);
  }

  protected void drawCircle(BBufferInputStream stream) throws IOException {
    float x = stream.readFloat();
    float y = stream.readFloat();
    float r = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      canvas_.drawCircle(x, y, r, p);
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        canvas_.drawCircle(x, y, r, p);
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        canvas_.drawCircle(x, y, r, p);
      }
    }
  }

  protected void drawArc(BBufferInputStream stream) throws IOException {
    float l = stream.readFloat();
    float t = stream.readFloat();
    float r = stream.readFloat();
    float b = stream.readFloat();
    float start = stream.readFloat();
    float end = stream.readFloat();
    boolean center = stream.readInt() != 0;
    Paint p = ReadPaint(stream, paint_);
  }

  protected void drawPath(BBufferInputStream stream) throws IOException {
    Path path = new Path();
    ReadPath(path, stream);
    path.close();
    Paint p = ReadPaint(stream, paint_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      canvas_.drawPath(path, p);
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        canvas_.drawPath(path, p);
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        canvas_.drawPath(path, p);
      }
    }
  }

  protected void drawArcTo(BBufferInputStream stream) throws IOException {
    float x1 = stream.readFloat();
    float y1 = stream.readFloat();
    float x2 = stream.readFloat();
    float y2 = stream.readFloat();
    float x3 = stream.readFloat();
    float y3 = stream.readFloat();
    float r = stream.readFloat();
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

    float x = stream.readFloat();
    float y = stream.readFloat();
    paint_.setTypeface(ttfont.mTypeface);
    Paint p = ReadPaint(stream, paint_);
    p.setFakeBoldText(is_bold_);
    p.setTextSkewX((is_italic_ ? -0.25f : 0f) + text_skew_);
    p.setColor(color_);
    if (color_ != 0 && stroke_color_ == color_) {
      p.setStyle(Paint.Style.FILL_AND_STROKE);
      canvas_.drawText(text_, 0, char_count, x, y, p);
    } else {
      if (color_ != 0) {
        p.setStyle(Paint.Style.FILL);
        canvas_.drawText(text_, 0, char_count, x, y, p);
      }
      if (stroke_color_ != 0) {
        p.setStyle(Paint.Style.STROKE);
        p.setColor(stroke_color_);
        canvas_.drawText(text_, 0, char_count, x, y, p);
      }
    }
  }

  protected void drawGlyphs(BBufferInputStream stream) throws IOException {
    JavaTypeface ttfont = mFontManager.GetTypefaceByIndex(stream.readInt());
    float x = stream.readFloat();
    float y = stream.readFloat();
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
    float dl = stream.readFloat();
    float dt = stream.readFloat();
    float dr = stream.readFloat();
    float db = stream.readFloat();
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
    float dl = stream.readFloat();
    float dt = stream.readFloat();
    float dr = stream.readFloat();
    float db = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    Bitmap img = BitmapFactory.decodeByteArray(buffer, 0, len);
    canvas_.drawBitmap(img, new Rect(0, 0, img.getWidth(), img.getHeight()),
        new Rect((int) dl, (int) dt, (int) dr, (int) db), p);
  }

  protected void drawImgRect(BBufferInputStream stream) throws IOException {
    int len = stream.readInt();
    byte[] buffer = new byte[len];
    stream.read(buffer, 0, len);
    float sl = stream.readFloat();
    float st = stream.readFloat();
    float sr = stream.readFloat();
    float sb = stream.readFloat();
    float dl = stream.readFloat();
    float dt = stream.readFloat();
    float dr = stream.readFloat();
    float db = stream.readFloat();
    Paint p = ReadPaint(stream, paint_);
    Bitmap img = BitmapFactory.decodeByteArray(buffer, 0, len);
    canvas_.drawBitmap(img, new Rect((int) sl, (int) st, (int) sr, (int) sb),
        new Rect((int) dl, (int) dt, (int) dr, (int) db), p);
  }

  protected Paint ReadPaint(BBufferInputStream stream, Paint painter) throws IOException {
    painter.setAntiAlias(true);
    painter.setStrokeWidth(stream.readFloat());
    color_ = stream.readInt();
    stroke_color_ = stream.readInt();
    text_size_ = stream.readFloat();
    painter.setTextSize(text_size_);
    text_skew_ = stream.readFloat();
    int flag = stream.readByte();
    switch (flag & 0x3) {
      case 1:
        painter.setStrokeCap(Paint.Cap.ROUND);
        break;
      case 2:
        painter.setStrokeCap(Paint.Cap.SQUARE);
        break;
      case 0:
      default:
        painter.setStrokeCap(Paint.Cap.BUTT);
        break;
    }
    is_bold_ = (flag & (1 << 2)) > 0;
    is_italic_ = (flag & (1 << 3)) > 0;
    is_underline_ = (flag & (1 << 4)) > 0;
    return painter;
  }

  protected void ReadPath(Path path, BBufferInputStream stream) throws IOException {
    int type = stream.readInt();
    switch (type) {
      case TTTextDefinition.PathType.LINES: {
        int len = stream.readInt();
        for (int i = 0; i < len; i++) {
          float x = stream.readFloat();
          float y = stream.readFloat();
          path.lineTo(x, y);
        }
      } break;
      case TTTextDefinition.PathType.ARC: {
        float x1 = stream.readFloat();
        float y1 = stream.readFloat();
        float x2 = stream.readFloat();
        float y2 = stream.readFloat();
        float x3 = stream.readFloat();
        float y3 = stream.readFloat();
        float r = stream.readFloat();
        // TODO:add arc
      } break;
      case TTTextDefinition.PathType.BEZIER: {
        int len = stream.readInt();
        PointF[] points = new PointF[len];
        for (int i = 0; i < len; i++) {
          float x = stream.readFloat();
          float y = stream.readFloat();
          points[i] = new PointF(x, y);
        }
        if (len == 2) {
          path.quadTo(points[0].x, points[0].y, points[1].x, points[1].y);
        } else if (len >= 3) {
          path.cubicTo(
              points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
        }
      } break;
      case TTTextDefinition.PathType.MOVE_TO: {
        float x = stream.readFloat();
        float y = stream.readFloat();
        path.moveTo(x, y);
      } break;
      case TTTextDefinition.PathType.MULTI_PATH: {
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
  protected float text_skew_;

  protected int color_;
  protected int stroke_color_;
  protected Paint mPainter = new Paint();
  protected char[] text_ = new char[20];
}
