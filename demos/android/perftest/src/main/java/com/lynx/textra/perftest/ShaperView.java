// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// package com.lynx.textra.perftest;
//
// import android.content.Context;
// import android.graphics.Canvas;
// import android.graphics.fonts.Font;
// import android.graphics.text.PositionedGlyphs;
// import android.graphics.text.TextRunShaper;
// import android.os.Build;
// import android.text.SpannableStringBuilder;
// import android.text.Spanned;
// import android.text.StaticLayout;
// import android.text.TextDirectionHeuristic;
// import android.text.TextPaint;
// import android.text.TextShaper;
// import android.text.style.AbsoluteSizeSpan;
// import android.text.style.ForegroundColorSpan;
// import android.util.AttributeSet;
// import android.view.View;
//
// import androidx.annotation.NonNull;
// import androidx.annotation.RequiresApi;
//
// import java.util.LinkedList;
//
///**
// * TODO: document your custom view class.
// */
//@RequiresApi(api = Build.VERSION_CODES.S)
// public class ShaperView extends View implements TextShaper.GlyphsConsumer, TextDirectionHeuristic
// {
//
//    private String content_;
//    private TextPaint paint = new TextPaint();
//
//    public class ShapeResult {
//        public ShapeResult(int glyph_count) {
//            glyphs_ = new int[glyph_count];
//            position_ = new float[2 * glyph_count];
//            font_ = new Font[glyph_count];
//            glyph_count_ = glyph_count;
//        }
//
//        public int glyph_count_;
//        public int[] glyphs_;
//        public float[] position_;
//        public Font[] font_;
//        public TextPaint paint_;
//    }
//
//    private LinkedList<ShapeResult> result_ = new LinkedList<>();
//
//    public ShaperView(Context context) {
//        super(context);
//        init(null, 0);
//    }
//
//    public ShaperView(Context context, AttributeSet attrs) {
//        super(context, attrs);
//        init(attrs, 0);
//    }
//
//    public ShaperView(Context context, AttributeSet attrs, int defStyle) {
//        super(context, attrs, defStyle);
//        init(attrs, defStyle);
//    }
//
//    private void init(AttributeSet attrs, int defStyle) {
//    }
//
//    public void CreateParagraph() {
//        result_ = new LinkedList<>();
//    }
//
//    public void AddText(String content, float text_size, int color) {
//        content_ = content;
//        paint.setTextSize(text_size);
//        paint.setColor(color);
//    }
//
//    public void LayoutContent() {
//        PositionedGlyphs glyphs = TextRunShaper.shapeTextRun(content_, 0, content_.length(), 0,
//        content_.length(), 0, 0, false, paint); ShapeResult result = new
//        ShapeResult(glyphs.glyphCount()); Font prev_font = null; for (int k = 0; k <
//        glyphs.glyphCount(); k++) {
//            result.glyphs_[k] = glyphs.getGlyphId(k);
//            result.position_[2 * k] = glyphs.getGlyphX(k);
//            result.position_[2 * k + 1] = glyphs.getGlyphY(k);
//            result.font_[k] = glyphs.getFont(k);
//            int ii = result.font_[k].getSourceIdentifier();
//            if (prev_font != null && !result.font_[k].equals(prev_font)) {
//                int i1 = prev_font.getSourceIdentifier();
//                int i2 = result.font_[k].getSourceIdentifier();
//                int i3 = 0;
//            } else {
//                prev_font = result.font_[k];
//            }
//        }
//        result_.add(result);
//    }
//
//    public void DrawContent(Canvas canvas) {
//        for (ShapeResult result : result_) {
////            canvas.drawGlyphs(result.glyphs_, 0, result.position_, 0, result.glyph_count_,
/// result.font_, result.paint_);
//        }
//    }
//
//    @Override
//    protected void onDraw(Canvas canvas) {
//        super.onDraw(canvas);
//        DrawContent(canvas);
//    }
//
//    @Override
//    public void accept(int start, int count, @NonNull PositionedGlyphs glyphs, @NonNull TextPaint
//    paint) {
//        ShapeResult result = new ShapeResult(glyphs.glyphCount());
//        for (int k = 0; k < glyphs.glyphCount(); k++) {
//            result.glyphs_[k] = glyphs.getGlyphId(k);
//            result.position_[2 * k] = glyphs.getGlyphX(k);
//            result.position_[2 * k + 1] = glyphs.getGlyphY(k);
//        }
////        result.font_ = glyphs.getFont(0);
//        result.paint_ = paint;
//        result_.add(result);
//    }
//
//    @Override
//    public boolean isRtl(char[] array, int start, int count) {
//        return true;
//    }
//
//    @Override
//    public boolean isRtl(CharSequence cs, int start, int count) {
//        return true;
//    }
//}
