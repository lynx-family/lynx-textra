// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ViewController.h"

#import "TTTextTestView.h"
#import "TTTextView.h"
#import "TestListViewController.h"

@interface ViewController ()
@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.title = @"LynxTextra Demo";

  TestListViewController* testListVC = [[TestListViewController alloc] init];
  [self addChildViewController:testListVC];
  testListVC.view.frame = self.view.bounds;
  [self.view addSubview:testListVC.view];
}

@end
