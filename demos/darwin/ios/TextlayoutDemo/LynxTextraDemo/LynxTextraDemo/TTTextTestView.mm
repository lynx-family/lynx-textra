// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TTTextTestView.h"

#include <CoreText/CoreText.h>
#include <json_parser/json_parser.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform_helper.h>
#include <textra/style.h>
#include <textra/text_layout.h>

#include <memory>
#include <unordered_map>

#import "JsonDataManager.h"
#include "paragraph_test.h"

namespace {
// Demo-only bundled-font aliases. Native font/canvas implementations stay
// private.
class DemoAssetFonts final : public IFontManager {
 public:
  void Add(const char* family, CTFontRef font) {
    fonts_[family] = PlatformHelper::CreateTypefaceHelper(font);
  }
  int countFamilies() const override { return static_cast<int>(fonts_.size()); }
  TypefaceRef matchFamilyStyle(const char* family, const FontStyle&) override {
    auto found = fonts_.find(family ? family : "");
    return found == fonts_.end() ? nullptr : found->second;
  }
  TypefaceRef matchFamilyStyleCharacter(const char*, const FontStyle&,
                                        const char*[], int, uint32_t) override {
    return nullptr;
  }
  TypefaceRef makeFromFile(const char*, int) override { return nullptr; }
  TypefaceRef legacyMakeTypeface(const char*, FontStyle) const override {
    return nullptr;
  }

 private:
  std::unordered_map<std::string, TypefaceRef> fonts_;
};
}  // namespace

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
                          to:(DemoAssetFonts*)fontmgr
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
  if (!cg_data) return;
  auto graphics_font = CGFontCreateWithDataProvider(cg_data);
  CGDataProviderRelease(cg_data);
  if (!graphics_font) return;
  auto font = CTFontCreateWithGraphicsFont(graphics_font, 24, nullptr, nullptr);
  CGFontRelease(graphics_font);
  if (!font) return;
  fontmgr->Add(font_name, font);
  CFRelease(font);
}

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  auto fontmgr = PlatformHelper::CreateFontManager();
  auto assets = std::make_shared<DemoAssetFonts>();
  [self registerFontWithName:@"Inter/Inter-VariableFont_opsz,wght.ttf"
                          to:assets.get()
                          as:"Inter"];
  [self registerFontWithName:@"Noto_Sans_CJK/NotoSansCJK-Regular.ttc"
                          to:assets.get()
                          as:"NotoSansCJK"];
  font_collection_ = std::make_unique<FontmgrCollection>(fontmgr);
  font_collection_->SetAssetFontManager(assets);
  paragraph_test_ = std::make_unique<ParagraphTest>(RenderingBackend::kIOS,
                                                    font_collection_.get());
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
  auto ios_canvas =
      PlatformHelper::CreateCanvasHelper(UIGraphicsGetCurrentContext());
  ios_canvas->DrawColor(0xFFFFFFFF);

  // If JSON file name exists, use JSON file for testing
  if (jsonFileName_) {
    [self drawJsonTest:ios_canvas.get()];
  } else {
    // Otherwise use C++ test case
    paragraph_test_->TestWithId(ios_canvas.get(), self.frame.size.width,
                                testId_);
  }
}

- (void)drawJsonTest:(ICanvasHelper*)ios_canvas {
  // Read JSON file content
  NSString* jsonContent =
      [jsonDataManager_ getJsonContentForFile:jsonFileName_];
  if (!jsonContent) {
    return;
  }

  std::string jsonString = [jsonContent UTF8String];
  auto jsonDoc = ttoffice::tttext::ParseJsonStringIntoTTTextObject(jsonString);

  auto layoutRegion = ttoffice::tttext::LayoutJsonDocument(
      jsonDoc.get(), font_collection_.get(),
      ttoffice::tttext::RenderingBackend::kIOS);

  LayoutDrawer drawer(ios_canvas);
  drawer.DrawLayoutPage(layoutRegion.get());
}

@end
