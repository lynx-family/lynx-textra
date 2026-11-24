// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <UIKit/UIKit.h>

#include "test/perf/shaping_benchmark.h"

@interface TextraPerfSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow* window;
@end

@implementation TextraPerfSceneDelegate
- (void)scene:(UIScene*)scene
    willConnectToSession:(UISceneSession*)session
                 options:(UISceneConnectionOptions*)options {
  self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
  self.window.rootViewController = [[UIViewController alloc] init];
  [self.window makeKeyAndVisible];
}
@end

@interface TextraPerfDelegate : UIResponder <UIApplicationDelegate>
@property(nonatomic, strong) UIWindow* window;
@property(nonatomic, strong) NSTimer* timer;
@end

@implementation TextraPerfDelegate
- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)options {
  NSString* directory = NSSearchPathForDirectoriesInDomains(
                            NSDocumentDirectory, NSUserDomainMask, YES)
                            .firstObject;
  NSString* requestPath =
      [directory stringByAppendingPathComponent:@"request.json"];
  NSString* responsePath =
      [directory stringByAppendingPathComponent:@"response.json"];
  NSString* metadata = [NSString
      stringWithContentsOfFile:[NSBundle.mainBundle pathForResource:@"metadata"
                                                             ofType:@"json"]
                      encoding:NSUTF8StringEncoding
                         error:nil];
  if (!metadata) return NO;
  self.timer = [NSTimer
      scheduledTimerWithTimeInterval:0.01
                             repeats:YES
                               block:^(NSTimer* timer) {
                                 NSString* request = [NSString
                                     stringWithContentsOfFile:requestPath
                                                     encoding:
                                                         NSUTF8StringEncoding
                                                        error:nil];
                                 if (!request) return;
                                 [NSFileManager.defaultManager
                                     removeItemAtPath:requestPath
                                                error:nil];
                                 std::string response;
                                 const int status = RunShapingBenchmark(
                                     request.UTF8String, metadata.UTF8String,
                                     &response);
                                 NSString* result =
                                     status ? [NSString
                                                  stringWithFormat:
                                                      @"{\"error\":\"benchmark "
                                                      @"failed\",\"code\":%d}",
                                                      status]
                                            : [NSString stringWithUTF8String:
                                                            response.c_str()];
                                 [result writeToFile:responsePath
                                          atomically:YES
                                            encoding:NSUTF8StringEncoding
                                               error:nil];
                               }];
  return YES;
}
@end

int main(int argc, char** argv) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil,
                             NSStringFromClass(TextraPerfDelegate.class));
  }
}
