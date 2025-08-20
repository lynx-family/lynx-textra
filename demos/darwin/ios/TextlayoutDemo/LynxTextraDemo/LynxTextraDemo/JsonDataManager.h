// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * JSON data file manager
 * Responsible for reading JSON files from the json_data directory in the main
 * bundle
 */
@interface JsonDataManager : NSObject

/**
 * Get all available JSON file names
 * @return Array of JSON file names, sorted alphabetically
 */
- (NSArray<NSString*>*)getAvailableFiles;

/**
 * Get the content of the specified JSON file
 * @param fileName JSON file name (including extension)
 * @return JSON file content string, returns nil if file doesn't exist or read
 * fails
 */
- (nullable NSString*)getJsonContentForFile:(NSString*)fileName;

@end

NS_ASSUME_NONNULL_END
