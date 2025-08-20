// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "CoreTextTestView.h"

#import <CoreText/CoreText.h>

@interface CoreTextTestView () {
  CGContextRef context_;
}
@end

@implementation CoreTextTestView {
}
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];

  CFMutableAttributedStringRef cf_attributed_string =
      CFAttributedStringCreateMutableCopy(
          kCFAllocatorDefault, self.attributedString.length,
          (__bridge CFAttributedStringRef)self.attributedString);

  int len = (int)CFAttributedStringGetLength(cf_attributed_string);
  CGFloat otherNum = 0.f;
  CFNumberRef otherCFNum =
      CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &otherNum);
  CFAttributedStringSetAttribute(cf_attributed_string, CFRangeMake(0, len),
                                 kCTLigatureAttributeName, otherCFNum);
  context_ = UIGraphicsGetCurrentContext();
  CTLineRef line = CTLineCreateWithAttributedString(cf_attributed_string);
  CTLineEnumerateCaretOffsets(
      line, ^(double offset, CFIndex charIndex, bool leadingEdge, bool* stop) {
        NSLog(@"offset:%5.2f, idx:%d, leadingEdge:%d\n", offset, (int)charIndex,
              leadingEdge);
      });
  CFArrayRef array_ref = CTLineGetGlyphRuns(line);
  auto array_count = CFArrayGetCount(array_ref);
  for (int k = 0; k < array_count; k++) {
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(array_ref, k);
    CFRange range = CTRunGetStringRange(run);
    CTFontRef font = (CTFontRef)CFDictionaryGetValue(
        CTRunGetAttributes(run), (__bridge CFStringRef)NSFontAttributeName);
    auto glyph_count = CTRunGetGlyphCount(run);
    CGGlyph glyphs[glyph_count];
    CTRunGetGlyphs(run, CFRangeMake(0, glyph_count), glyphs);
    CGSize advances[glyph_count];
    CTRunGetAdvances(run, range, advances);
    NSLog(@"glyph_count:%ld, char_count:%lu", glyph_count,
          [self.attributedString length]);
    [self draw:static_cast<int32_t>(glyph_count)
              Glyph:glyphs
           withFont:font
        andAdvances:advances];
  }
}

- (void)draw:(int)number
          Glyph:(CGGlyph*)glyphs
       withFont:(CTFontRef)font
    andAdvances:(const CGSize*)advances {
  CGContextSetRGBFillColor(context_, 1, 0, 0, 1);
  CGContextSaveGState(context_);
  float x = 50;
  float y = 100;
  auto text_mat = CGAffineTransformIdentity;
  CGContextSetTextMatrix(context_, text_mat);
  CGContextTranslateCTM(context_, x, y);
  CGContextScaleCTM(context_, 1, -1);
  CGPoint pos = CGPointMake(0, 0);
  for (CFIndex k = 0; k < number; k++) {
    CTFontDrawGlyphs(font, glyphs + k, &pos, 1, context_);
    pos.y -= 15;
  }
  CGContextRestoreGState(context_);
}

@end
