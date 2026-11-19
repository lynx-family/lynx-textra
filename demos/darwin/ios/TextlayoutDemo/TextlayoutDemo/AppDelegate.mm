// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AppDelegate.h"

#import <ImageIO/ImageIO.h>

#include "test/capi/ios_comparison.hpp"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  if ([[NSProcessInfo processInfo].arguments containsObject:@"--textra-capi"]) {
    NSString* output = [NSHomeDirectory()
        stringByAppendingPathComponent:@"Documents/capi-comparison"];
    [[NSFileManager defaultManager] createDirectoryAtPath:output
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    bool ok = textra_demo::RunIosComparison(output.UTF8String);
    NSLog(@"TEXTRA_CAPI_COMPARISON %@ %@", ok ? @"PASS" : @"FAIL", output);
    // The regular demo remains available. The comparison images and report
    // are persisted so simulator scripts can verify them independently.
  }
  return YES;
}

#pragma mark - UISceneSession lifecycle

- (UISceneConfiguration*)application:(UIApplication*)application
    configurationForConnectingSceneSession:
        (UISceneSession*)connectingSceneSession
                                   options:(UISceneConnectionOptions*)options {
  // Called when a new scene session is being created.
  // Use this method to select a configuration to create the new scene with.
  return
      [[UISceneConfiguration alloc] initWithName:@"Default Configuration"
                                     sessionRole:connectingSceneSession.role];
}

- (void)application:(UIApplication*)application
    didDiscardSceneSessions:(NSSet<UISceneSession*>*)sceneSessions {
  // Called when the user discards a scene session.
  // If any sessions were discarded while the application was not running, this
  // will be called shortly after application:didFinishLaunchingWithOptions. Use
  // this method to release any resources that were specific to the discarded
  // scenes, as they will not return.
}

@end
