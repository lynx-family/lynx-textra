// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "UIView+TextKit2View.h"

@interface TextKit2View ()

@property(nonatomic, retain) NSTextContentStorage* storage_;
@property(nonatomic, retain) NSTextLayoutManager* layout_manager_;
@property(nonatomic, retain) NSTextContainer* container_;

@end

@implementation TextKit2View {
  NSMutableArray* fragment_array_;
  NSDictionary* attributes;
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
  self.storage_ = [[NSTextContentStorage alloc] init];
  self.container_ = [[NSTextContainer alloc] initWithSize:self.bounds.size];
  self.layout_manager_ = [[NSTextLayoutManager alloc] init];
  self.layout_manager_.textContainer = self.container_;
  [self.storage_ addTextLayoutManager:self.layout_manager_];
  fragment_array_ = [[NSMutableArray alloc] init];
  UIFont* font = [UIFont systemFontOfSize:24];  // Set font size to 24

  NSMutableParagraphStyle* paragraphStyle =
      [[NSMutableParagraphStyle alloc] init];
  paragraphStyle.alignment = NSTextAlignmentLeft;  // Set to left alignment

  attributes = @{
    NSFontAttributeName : font,
    NSParagraphStyleAttributeName : paragraphStyle,
    NSKernAttributeName : @5
  };
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
  [self.storage_ setAttributedString:content];
  [self.layout_manager_
      enumerateTextLayoutFragmentsFromLocation:nil
                                       options:
                                           NSTextLayoutFragmentEnumerationOptionsEnsuresLayout
                                    usingBlock:^BOOL(
                                        NSTextLayoutFragment* layoutFragment) {
                                      [fragment_array_
                                          addObject:layoutFragment];
                                      return true;
                                    }];
}

- (void)drawOnLayer {
  CGContextRef context = [self createBitmapContext];
  for (NSTextLayoutFragment* fg in fragment_array_) {
    CGPoint drawPoint = CGPointMake(10, 10);
    if (fg.state == NSTextLayoutFragmentStateLayoutAvailable) {
      [fg drawAtPoint:drawPoint inContext:context];
    } else {
      NSLog(@"fg state error");
    }
  }
  CGContextRelease(context);
}

- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];

  CGContextRef context = UIGraphicsGetCurrentContext();
  for (NSTextLayoutFragment* fg in fragment_array_) {
    // Ensure drawing in the correct context

    // Set drawing position
    CGPoint drawPoint = CGPointMake(10, 10);

    // Draw the text layout fragment in the current context
    [fg drawAtPoint:drawPoint inContext:context];
  }
}
@end
