// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <UIKit/UIKit.h>
#include <textra/fontmgr_collection.h>
#include <textra/i_canvas_helper.h>

NS_ASSUME_NONNULL_BEGIN

using namespace tttext;

@interface TTTextTestView : UIView

- (void)setTestId:(NSInteger)testId;
- (void)setJsonFileName:(NSString*)jsonFileName;

@end

NS_ASSUME_NONNULL_END
