// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface TTTextView : UIView

//@property(nonatomic, strong, nullable) TTTextLayout *textLayout;
@property(nonatomic, strong, nullable) NSAttributedString* text;
@property(nullable, nonatomic, copy) NSAttributedString* attributedText;

@property(nonatomic) NSTextAlignment
    textAlignment;  // default is NSTextAlignmentNatural (before iOS 9, the
                    // default was NSTextAlignmentLeft)
@property(nonatomic) NSLineBreakMode lineBreakMode;
@property(nonatomic) CGFloat lineSpacing;
@property(nonatomic) CGFloat paragraphSpacing;
@property(nonatomic) CGFloat paragraphSpacingBefore;

- (void)createParagraph;
- (void)addTextRunWithContent:(const char*)text
                        color:(uint32_t)color
                     fontSize:(uint32_t)size;
- (void)layoutWithContent;
- (void)drawOnLayer;
@end

NS_ASSUME_NONNULL_END
