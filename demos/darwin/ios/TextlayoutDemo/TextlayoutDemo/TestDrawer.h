// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TestDrawer_h
#define TestDrawer_h
#import <UIKit/UIKit.h>

#ifdef USE_TTTEXT
#import "TTText-lite/TTTextDrawerDelegate.h"
@interface TestDrawer : NSObject <TTTextDrawerDelegate>
@property(nonatomic, weak) UIView* parentView;
@property(nonatomic) CGPoint zeroPoint;
@property(nonatomic) int tag;
@property(nonatomic) int times;
- (void)Clear;
@end
#endif
#endif /* TestDrawer_h */
