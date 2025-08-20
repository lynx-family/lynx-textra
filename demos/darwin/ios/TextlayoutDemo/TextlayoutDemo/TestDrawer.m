// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TestDrawer.h"

#ifdef USE_TTTEXT
#import <Foundation/Foundation.h>
@implementation TestDrawer
- (void)DrawRunDelegate:(id<TTTextRunDelegate>)attachment InRect:(CGRect)rect {
  [self drawAttachment:((TTTextRunDelegateImpl*)attachment).GetAttachment
                InRect:rect];
}
- (void)drawAttachment:(id)attachment InRect:(CGRect)rect {
  UIImage* img = (UIImage*)attachment;
  UIImageView* view = [[UIImageView alloc]
      initWithFrame:CGRectMake(rect.origin.x + self.zeroPoint.x,
                               rect.origin.y + self.zeroPoint.y,
                               rect.size.width, rect.size.height)];
  view.image = img;
  view.tag = self.tag;
  [self.parentView addSubview:view];
}
- (void)Clear {
  UIView* subview = [self.parentView viewWithTag:self.tag];
  while (subview != nil) {
    [subview removeFromSuperview];
    subview = [self.parentView viewWithTag:self.tag];
  }
}
- (UIColor*)FetchThemeColor:(ThemeColorType)type Extra:(NSString*)extra {
  if (type == kLink) {
    return [UIColor blueColor];
  }
  self.times++;
  if (self.times % 3 == 0) return [UIColor redColor];
  if (self.times % 3 == 1) return [UIColor blueColor];
  if (self.times % 3 == 2) return [UIColor yellowColor];
  return [UIColor blackColor];
}

- (void)DrawBackgroundDelegate:(id<TTTextRunDelegate>)attachment {
  return;
}

@end
#endif
