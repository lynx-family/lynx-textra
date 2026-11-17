// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef SRC_PORTS_SHAPER_CORETEXT_FONT_FEATURE_SETTINGS_H_
#define SRC_PORTS_SHAPER_CORETEXT_FONT_FEATURE_SETTINGS_H_

#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>
#include <textra/font_info.h>

namespace ttoffice {
namespace tttext {

// Compose features with the matching/cascade attributes before creating the
// cached font, rather than copying that font on every shaping request.
inline void AddFontFeatureSettings(CFMutableDictionaryRef attributes,
                                   const FontDescriptor& fd) {
  if (fd.font_feature_settings_.empty()) {
    return;
  }
  NSMutableArray* features = [NSMutableArray array];
  for (const auto& [tag, value] : fd.font_feature_settings_) {
    [features addObject:@{
      (__bridge NSString*)kCTFontOpenTypeFeatureTag :
          [[NSString alloc] initWithBytes:tag
                                   length:4
                                 encoding:NSASCIIStringEncoding],
      (__bridge NSString*)kCTFontOpenTypeFeatureValue : @(value),
    }];
  }
  CFDictionarySetValue(attributes, kCTFontFeatureSettingsAttribute,
                       (__bridge CFArrayRef)features);
}

}  // namespace tttext
}  // namespace ttoffice

#endif  // SRC_PORTS_SHAPER_CORETEXT_FONT_FEATURE_SETTINGS_H_
