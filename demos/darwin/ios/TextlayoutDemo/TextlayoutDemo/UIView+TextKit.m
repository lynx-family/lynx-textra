// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "UIView+TextKit.h"

@implementation TextKitView {
  NSTextStorage* storage;
  NSLayoutManager* layout_manager;
  NSDictionary* attributes;
  NSTextContainer* container;
  UIGraphicsImageRenderer* renderer;
  NSMutableAttributedString* content;
  void* buffer;
  CGSize buffer_size;
}

- (void)dealloc {
  free(buffer);
}

- (CGContextRef)createBitmapContext {
  size_t bytesPerPixel = 4;  // Usually RGBA format
  size_t bytesPerRow = bytesPerPixel * buffer_size.width;
  size_t bitsPerComponent = 8;  // 8 bits per color component

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
      buffer, buffer_size.width, buffer_size.height, bitsPerComponent,
      bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);

  CGColorSpaceRelease(colorSpace);

  return context;
}

- (void)layoutSubviews {
  self.backgroundColor = [UIColor darkGrayColor];
  storage = [[NSTextStorage alloc] init];
  layout_manager = [[NSLayoutManager alloc] init];
  container = [[NSTextContainer alloc] initWithSize:self.bounds.size];
  layout_manager.textStorage = storage;
  //  container.lineBreakMode = NSLineBreakByTruncatingTail; // Set ellipsis
  //  container.maximumNumberOfLines = 2; // Multi-line display
  container.layoutManager = layout_manager;
  [storage addLayoutManager:layout_manager];
  [layout_manager addTextContainer:container];

  buffer_size = CGSizeMake(self.bounds.size.width, 2000);
  buffer = calloc(buffer_size.height, 4 * buffer_size.width);
}
- (void)createParagraph {
  content = [[NSMutableAttributedString alloc] init];
}
- (void)addTextRunWithContent:(const char*)text
                        color:(uint32_t)color
                     fontSize:(uint32_t)size {
  NSMutableParagraphStyle* paragraphStyle =
      [[NSMutableParagraphStyle alloc] init];
  paragraphStyle.alignment = NSTextAlignmentLeft;  // Set to left alignment

  CGFloat alpha = ((color >> 24) & 0xFF) / 255.0;
  CGFloat red = ((color >> 16) & 0xFF) / 255.0;
  CGFloat green = ((color >> 8) & 0xFF) / 255.0;
  CGFloat blue = (color & 0xFF) / 255.0;

  UIColor* uiColor = [UIColor colorWithRed:red
                                     green:green
                                      blue:blue
                                     alpha:alpha];
  UIFont* font = [UIFont systemFontOfSize:size];
  attributes = @{
    NSFontAttributeName : font,
    NSParagraphStyleAttributeName : paragraphStyle,
    NSForegroundColorAttributeName : uiColor,
  };

  NSString* str = [NSString stringWithUTF8String:text];
  NSAttributedString* ns_str =
      [[NSAttributedString alloc] initWithString:str attributes:attributes];
  [content appendAttributedString:ns_str];
}
- (void)layoutWithContent {
  [storage setAttributedString:content];
  [layout_manager ensureLayoutForTextContainer:container];
}
- (void)drawOnLayer {
  CGContextRef context = [self createBitmapContext];
  UIGraphicsPushContext(context);
  CGPoint drawPoint = CGPointMake(10, 10);
  [layout_manager
      drawGlyphsForGlyphRange:NSMakeRange(0, [layout_manager numberOfGlyphs])
                      atPoint:drawPoint];
  UIGraphicsPopContext();
  CGContextRelease(context);
}

- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];

  CGPoint drawPoint = CGPointMake(0, 0);
  [layout_manager
      drawGlyphsForGlyphRange:NSMakeRange(0, [layout_manager numberOfGlyphs])
                      atPoint:drawPoint];
}
@end
