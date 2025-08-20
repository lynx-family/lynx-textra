// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TTTextRunDelegate.h"

#import "TTTextDrawerDelegate.h"

@interface TTTextRunDelegateImpl ()
@property(nonatomic) CGFloat ascent;
@property(nonatomic) CGFloat descent;
@property(nonatomic) CGFloat width;
@property(nonatomic) id attachment;
@property(nonatomic) id<TTTextDrawerDelegate> drawer;
@property(nonatomic) bool hide;
@end

@implementation TTTextRunDelegateImpl

- (instancetype)initWithAscent:(CGFloat)ascent
                       Descent:(CGFloat)descent
                         Width:(CGFloat)width
                    Attachment:(id)attachment;
{
  self = [super init];
  self.ascent = ascent;
  self.descent = descent;
  self.width = width;
  self.attachment = attachment;
  self.drawer = NULL;
  self.hide = false;
  return self;
}

- (CGFloat)GetAscent {
  return self.ascent;
}

- (CGFloat)GetDescent {
  return self.descent;
}

- (CGFloat)GetWidth {
  return self.width;
}

- (CGFloat)GetHeight {
  return self.descent + self.ascent;
}

- (id)GetAttachment {
  return self.attachment;
}

- (void)SetHide:(bool)hide {
  self.hide = hide;
}
- (bool)Hide {
  return self.hide;
}
@end
