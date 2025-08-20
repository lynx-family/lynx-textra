// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TTTextDrawerDelegate_h
#define TTTextDrawerDelegate_h
#import <UIKit/UIKit.h>

#import "TTTextRunDelegate.h"
typedef enum : NSUInteger {
  kNormal,
  kLink,
  kBackground,
  kBorder,
  kFootnote
} ThemeColorType;
@protocol TTTextDrawerDelegate <NSObject>
@optional
- (nullable UIColor*)FetchThemeColor:(ThemeColorType)type
                               Extra:(nullable NSString*)extra;
@required
- (void)DrawRunDelegate:(id<TTTextRunDelegate>)attachment InRect:(CGRect)rect;

- (void)DrawBackgroundDelegate:(id<TTTextRunDelegate>)attachment;
@end

#endif /* TTTextDrawerDelegate_h */
