// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TTTextRunDelegate_h
#define TTTextRunDelegate_h

#import <UIKit/UIKit.h>

typedef enum : NSUInteger { kNone, kUnderLine } TTTextLinkStyle;

@protocol TTTextLink <NSObject>
@optional
- (TTTextLinkStyle)linkStyle;

@end

@protocol TTTextFootnote <NSObject>
@optional
- (TTTextLinkStyle)linkStyle;
@end

@protocol TTTextRunDelegate <NSObject>
@required
- (CGFloat)GetAscent;
- (CGFloat)GetDescent;
- (CGFloat)GetWidth;
- (CGFloat)GetHeight;
//- (id)GetAttachment;
@optional
- (bool)Hide;
- (bool)NeedPlaceHolder;
@end

@interface TTTextRunDelegateImpl : NSObject <TTTextRunDelegate>
- (nonnull instancetype)initWithAscent:(CGFloat)ascent
                               Descent:(CGFloat)descent
                                 Width:(CGFloat)width
                            Attachment:(nullable id)attachment;
- (nullable id)GetAttachment;
- (void)SetHide:(bool)hide;
@end

#endif /* TTTextRunDelegate_h */
