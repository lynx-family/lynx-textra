// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TestListViewController.h"

#import "JsonDataManager.h"
#import "TTTextTestView.h"
#include "paragraph_test.h"

#pragma mark - TestCaseViewController

@interface TTTextTestViewController : UIViewController
@property(nonatomic, assign) NSInteger testId;
@property(nonatomic, strong) NSString* jsonFileName;
@end

@implementation TTTextTestViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];

  TTTextTestView* testView = [[TTTextTestView alloc] init];
  if (self.testId) {
    [testView setTestId:self.testId];
  } else if (self.jsonFileName) {
    [testView setJsonFileName:self.jsonFileName];
  }
  [self.view addSubview:testView];

  UILayoutGuide* safeArea = self.view.safeAreaLayoutGuide;
  testView.translatesAutoresizingMaskIntoConstraints = NO;
  [NSLayoutConstraint activateConstraints:@[
    [testView.topAnchor constraintEqualToAnchor:safeArea.topAnchor],
    [testView.leadingAnchor constraintEqualToAnchor:safeArea.leadingAnchor],
    [testView.trailingAnchor constraintEqualToAnchor:safeArea.trailingAnchor],
    [testView.bottomAnchor constraintEqualToAnchor:safeArea.bottomAnchor]
  ]];
}

@end

#pragma mark - TestListViewController

@interface TestListViewController ()
@property(nonatomic, strong) NSArray<NSDictionary*>* testCaseInfos;
@end

@implementation TestListViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.title = @"LynxTextra Demo";
  [self.tableView registerClass:[UITableViewCell class]
         forCellReuseIdentifier:@"TestCaseCell"];

  NSMutableArray* testCases = [NSMutableArray array];

  // Add C++ test cases
  auto cppTestCases = ParagraphTest::GetTestCaseIdAndName();
  for (const auto& [test_id, test_name] : cppTestCases) {
    [testCases addObject:@{
      @"type" : @"cpp",
      @"id" : @(test_id),
      @"name" : [NSString stringWithUTF8String:test_name.c_str()]
    }];
  }

  // Add JSON file test cases
  JsonDataManager* jsonDataManager = [[JsonDataManager alloc] init];
  NSArray<NSString*>* jsonFiles = [jsonDataManager getAvailableFiles];
  for (NSInteger i = 0; i < jsonFiles.count; i++) {
    NSString* fileName = jsonFiles[i];
    [testCases addObject:@{
      @"type" : @"json",
      @"id" : @(i),
      @"name" : [fileName stringByDeletingPathExtension],
      @"fileName" : fileName
    }];
  }

  self.testCaseInfos = testCases;
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView*)tableView
    numberOfRowsInSection:(NSInteger)section {
  return self.testCaseInfos.count;
}

- (UITableViewCell*)tableView:(UITableView*)tableView
        cellForRowAtIndexPath:(NSIndexPath*)indexPath {
  UITableViewCell* cell =
      [tableView dequeueReusableCellWithIdentifier:@"TestCaseCell"
                                      forIndexPath:indexPath];
  NSDictionary* testCaseInfo = self.testCaseInfos[indexPath.row];
  NSInteger testId = [testCaseInfo[@"id"] integerValue];
  NSString* testName = testCaseInfo[@"name"];
  NSString* type = testCaseInfo[@"type"];

  cell.textLabel.text =
      [NSString stringWithFormat:@"%ld: %@ (%@)", (long)testId, testName, type];
  cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
  return cell;
}

#pragma mark - UITableViewDelegate

- (void)tableView:(UITableView*)tableView
    didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
  TTTextTestViewController* testCaseVC =
      [[TTTextTestViewController alloc] init];
  NSDictionary* testCaseInfo = self.testCaseInfos[indexPath.row];
  NSString* type = testCaseInfo[@"type"];
  if ([type isEqualToString:@"cpp"]) {
    NSInteger testId = [testCaseInfo[@"id"] integerValue];
    testCaseVC.testId = testId;
    testCaseVC.jsonFileName = nil;
  } else {
    testCaseVC.jsonFileName = testCaseInfo[@"fileName"];
    testCaseVC.testId = 0;
  }
  testCaseVC.title =
      [NSString stringWithFormat:@"%ld: %@", (long)testCaseVC.testId,
                                 testCaseInfo[@"name"]];

  [self.navigationController pushViewController:testCaseVC animated:YES];
}

@end
