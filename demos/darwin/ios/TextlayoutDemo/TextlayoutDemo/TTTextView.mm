// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TTTextView.h"

#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform/ios/ios_canvas_base.h>
#include <textra/platform/ios/ios_font_manager.h>
#include <textra/text_layout.h>
#include <textra/tt_color.h>

#include <memory>

using namespace ttoffice::tttext;

@interface TTTextView ()

@end

@implementation TTTextView {
  std::unique_ptr<tttext::LayoutRegion> region;
  std::unique_ptr<tttext::FontmgrCollection> font_collection;
  std::unique_ptr<tttext::TextLayout> layout;
  std::unique_ptr<tttext::TTTextContext> context;
  std::unique_ptr<tttext::Paragraph> paragraph;
  UIFont* font;
  CALayer* layer_;
  //  UIGraphicsImageRenderer *renderer;
  void* buffer;
  CGSize buffer_size;
}

- (instancetype)init {
  self = [super init];
  return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
  }
  return self;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  self = [super initWithCoder:coder];
  return self;
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

- (void)createParagraph {
  paragraph = tttext::Paragraph::Create();
  tttext::ParagraphStyle p_style;
  p_style.SetHorizontalAlign(tttext::ParagraphHorizontalAlignment::kLeft);
  //  p_style.SetMaxLines(2);
  //  p_style.SetEllipsis("…");
  paragraph->SetParagraphStyle(&p_style);
}

- (void)addTextRunWithContent:(const char*)text
                        color:(uint32_t)color
                     fontSize:(uint32_t)size {
  tttext::Style r_style;
  tttext::FontDescriptor fd;
  fd.platform_font_ = (uint64_t)(__bridge void*)font;
  r_style.SetFontDescriptor(fd);
  r_style.SetTextSize(size);
  //  r_style.SetLetterSpacing(5);
  r_style.SetForegroundColor(tttext::TTColor{color});
  paragraph->AddTextRun(&r_style, text);
}

- (void)layoutWithContent {
  context->Reset();
  region = std::make_unique<LayoutRegion>(self.bounds.size.width,
                                          self.bounds.size.height);
  layout->Layout(paragraph.get(), region.get(), *context);
}

- (void)layoutSubviews {
  [super layoutSubviews];
  self.backgroundColor = [UIColor grayColor];

  if (CGSizeEqualToSize(self.bounds.size, CGSizeZero)) {
    return;
  }

  if (region != nullptr &&
      CGSizeEqualToSize(
          self.bounds.size,
          CGSizeMake(region->GetPageWidth(), region->GetPageHeight()))) {
    return;
  }
  auto font_mgr = std::make_shared<tttext::FontManagerCoreText>();
  font_collection = std::make_unique<tttext::FontmgrCollection>(font_mgr);
  layout = std::make_unique<tttext::TextLayout>(font_collection.get(),
                                                tttext::ShaperType::kSystem);
  context = std::make_unique<tttext::TTTextContext>();
  region = std::make_unique<LayoutRegion>(self.bounds.size.width,
                                          self.bounds.size.height);
  font = [UIFont systemFontOfSize:24];
  layer_ = [CALayer layer];
  layer_.frame =
      CGRectMake(0, 0, self.bounds.size.width, self.bounds.size.height);

  buffer_size = CGSizeMake(self.bounds.size.width, 2000);
  buffer = calloc(buffer_size.height, 4 * buffer_size.width);

  //  renderer = [[UIGraphicsImageRenderer alloc]
  //  initWithSize:self.bounds.size]; renderer = [[UIGraphicsImageRenderer
  //  alloc] initWithSize:CGSizeMake(self.bounds.size.width, 1000)];
}

- (void)drawOnLayer {
  //  UIImage *image = [renderer
  //  imageWithActions:^(UIGraphicsImageRendererContext * _Nonnull context) {
  //    CGContextRef cgContext = context.CGContext;
  CGContextRef context = [self createBitmapContext];
  IOSCanvasBase ios_canvas(context);
  tttext::LayoutDrawer layout_drawer(&ios_canvas);
  layout_drawer.DrawLayoutPage(region.get());
  CGContextRelease(context);
  //  }];
}
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];
  if (region == nullptr) return;
  IOSCanvasBase ios_canvas(UIGraphicsGetCurrentContext());
  tttext::LayoutDrawer layout_drawer(&ios_canvas);
  layout_drawer.DrawLayoutPage(region.get());
}

@end
