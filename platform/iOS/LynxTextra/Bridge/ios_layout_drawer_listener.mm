// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ios_layout_drawer_listener.h"

#import <Foundation/Foundation.h>

uint32_t IOSLayoutDrawerListener::FetchThemeColor(ThemeCategory type,
                                                  const TTColor& run_color) {
  return run_color.IsUndefined() || run_color.IsThemeColor()
             ? 0xFF000000
             : run_color.GetPlainColor();
};
