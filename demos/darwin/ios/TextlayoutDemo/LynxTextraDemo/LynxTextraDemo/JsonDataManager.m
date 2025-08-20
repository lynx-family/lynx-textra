// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "JsonDataManager.h"

@implementation JsonDataManager

- (NSArray<NSString*>*)getAvailableFiles {
  NSMutableArray<NSString*>* jsonFiles = [NSMutableArray array];

  NSBundle* mainBundle = [NSBundle mainBundle];
  NSArray<NSString*>* files = [mainBundle pathsForResourcesOfType:@"json"
                                                      inDirectory:@"json_data"];
  for (NSString* filePath in files) {
    NSString* fileName = [filePath lastPathComponent];
    [jsonFiles addObject:fileName];
  }
  [jsonFiles sortUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
  return jsonFiles;
}

- (NSString*)getJsonContentForFile:(NSString*)fileName {
  NSBundle* mainBundle = [NSBundle mainBundle];
  NSString* filePath =
      [mainBundle pathForResource:[fileName stringByDeletingPathExtension]
                           ofType:[fileName pathExtension]
                      inDirectory:@"json_data"];
  if (!filePath) {
    NSLog(@"JsonDataManager: File not found: %@", fileName);
    return nil;
  }

  NSError* error = nil;
  NSString* jsonContent =
      [NSString stringWithContentsOfFile:filePath
                                encoding:NSUTF8StringEncoding
                                   error:&error];
  if (error) {
    NSLog(@"JsonDataManager: Failed to read file: %@",
          error.localizedDescription);
    return nil;
  }
  return jsonContent;
}

@end
