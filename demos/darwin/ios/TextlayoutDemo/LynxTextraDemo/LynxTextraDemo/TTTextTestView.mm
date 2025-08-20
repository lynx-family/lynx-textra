// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TTTextTestView.h"

#include <json_parser/json_parser.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform/ios/ios_canvas_base.h>
#include <textra/platform/ios/ios_font_manager.h>
#include <textra/style.h>
#include <textra/text_layout.h>

#include <memory>

#import "JsonDataManager.h"
#include "paragraph_test.h"

@interface TTTextTestView () {
  std::unique_ptr<FontmgrCollection> font_collection_;
  // For cpp test case
  std::unique_ptr<ParagraphTest> paragraph_test_;
  NSInteger testId_;
  // For json test case
  JsonDataManager* jsonDataManager_;
  NSString* jsonFileName_;
}
@end

@implementation TTTextTestView

- (void)registerFontWithName:(NSString*)fontFileName
                          to:(FontManagerCoreText*)fontmgr
                          as:(const char*)font_name {
  // 1. Get font file path
  NSString* fontPath = [[NSBundle mainBundle] pathForResource:fontFileName
                                                       ofType:nil];
  if (!fontPath) {
    NSLog(@"Cannot find typeface file: %@", fontFileName);
    return;
  }

  CGDataProviderRef cg_data = CGDataProviderCreateWithFilename(
      [fontPath cStringUsingEncoding:kCFStringEncodingUTF8]);
  fontmgr->RegisterFont(cg_data, font_name);
}

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  auto fontmgr = std::make_shared<FontManagerCoreText>();
  [self registerFontWithName:@"Inter/Inter-VariableFont_opsz,wght.ttf"
                          to:fontmgr.get()
                          as:"Inter"];
  [self registerFontWithName:@"Noto_Sans_CJK/NotoSansCJK-Regular.ttc"
                          to:fontmgr.get()
                          as:"NotoSansCJK"];
  font_collection_ = std::make_unique<FontmgrCollection>(fontmgr);
  paragraph_test_ =
      std::make_unique<ParagraphTest>(kSystem, font_collection_.get());
  testId_ = 0;
  jsonFileName_ = nil;
  jsonDataManager_ = [[JsonDataManager alloc] init];
  return self;
}

- (void)setTestId:(NSInteger)testId {
  testId_ = testId;
  jsonFileName_ = nil;
  [self setNeedsDisplay];
}

- (void)setJsonFileName:(NSString*)jsonFileName {
  jsonFileName_ = jsonFileName;
  testId_ = 0;
  [self setNeedsDisplay];
}

- (void)dealloc {
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];
  UIGraphicsGetCurrentContext();
  IOSCanvasBase ios_canvas(UIGraphicsGetCurrentContext());
  ios_canvas.DrawColor(0xFFFFFFFF);

  // If JSON file name exists, use JSON file for testing
  if (jsonFileName_) {
    [self drawJsonTest:&ios_canvas];
  } else {
    // Otherwise use C++ test case
    paragraph_test_->TestWithId(&ios_canvas, self.frame.size.width, testId_);
  }
}

- (void)drawJsonTest:(IOSCanvasBase*)ios_canvas {
  // Read JSON file content
  NSString* jsonContent =
      [jsonDataManager_ getJsonContentForFile:jsonFileName_];
  if (!jsonContent) {
    return;
  }

  try {
    std::string jsonString = [jsonContent UTF8String];
    auto jsonDoc =
        ttoffice::tttext::ParseJsonStringIntoTTTextObject(jsonString);

    auto layoutRegion = ttoffice::tttext::LayoutJsonDocument(
        jsonDoc.get(), font_collection_.get(),
        ttoffice::tttext::ShaperType::kSystem);

    LayoutDrawer drawer(ios_canvas);
    drawer.DrawLayoutPage(layoutRegion.get());

  } catch (const std::exception& e) {
    NSLog(
        @"TTTextTestView: Exception occurred while processing JSON file: %@, "
        @"%s",
        jsonFileName_, e.what());
  }
}

@end
