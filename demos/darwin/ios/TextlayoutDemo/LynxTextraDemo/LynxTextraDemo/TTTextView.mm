// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "TTTextView.h"

#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/paragraph.h>
#include <textra/platform/ios/ios_canvas_base.h>
#include <textra/platform/ios/ios_font_manager.h>
#include <textra/style.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <memory>

#include "paragraph_test.h"

@interface TTTextView () {
  std::unique_ptr<TextLayout> layout_;
  std::unique_ptr<TTTextContext> context_;
  std::unique_ptr<Paragraph> paragraph_;
  std::unique_ptr<LayoutRegion> layout_region_;
  std::unique_ptr<FontmgrCollection> font_collection_;
}
@end

@implementation TTTextView

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  auto fontmgr = std::make_shared<FontManagerCoreText>();
  font_collection_ = std::make_unique<FontmgrCollection>(fontmgr);
  layout_ =
      std::make_unique<TextLayout>(font_collection_.get(), ShaperType::kSystem);
  context_ = std::make_unique<TTTextContext>();
  paragraph_ = Paragraph::Create();
  return self;
}

- (void)dealloc {
}

- (void)LayoutParagraphWithSize:(CGSize)size {
  //  auto str =
  //      "如果上面加上一行。This is a line of English "
  //      "text.\n上面再加上一行。\n\"[ Chapter 1 ]\"هذا نص عربي لا معنى له ،
  //      فقط " "لأغراض الاختبار ، طالما أنه طويل بما فيه الكفاية ، فكل شيء آخر
  //      لا " "يهم.";
  //    auto str = "❄️ 冬日慵懒🤗🤗🤗测试中文能看到中文吗🤗🤗🤗";
  auto str = "عربي";
  //  auto str = "میں بہت بڑا مداح ہوں!";
  //  auto str = "❄️ 冬日慵懒";
  FontDescriptor fd;
  fd.font_style_ =
      FontStyle(FontStyle::kMedium_Weight, FontStyle::kNormal_Width,
                FontStyle::kUpright_Slant);
  tttext::Style style;
  style.SetFontDescriptor(fd);
  style.SetTextSize(24);
  paragraph_->AddTextRun(&style, str);
  layout_region_ = std::make_unique<LayoutRegion>(size.width, size.height);
  layout_->Layout(paragraph_.get(), layout_region_.get(), *context_);
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];
  UIGraphicsGetCurrentContext();
  IOSCanvasBase ios_canvas(UIGraphicsGetCurrentContext());
  ios_canvas.DrawColor(0xFFFFFFFF);
  LayoutDrawer layout_drawer(&ios_canvas);
  layout_drawer.DrawLayoutPage(layout_region_.get());
}

- (void)layoutSubviews {
  //  auto safe_area = UIEdgeInsetsInsetRect(self.frame, self.safeAreaInsets);
  [self LayoutParagraphWithSize:self.frame.size];
}

+ (void)TestWithId:(int32_t)id
          onCanvas:(ICanvasHelper*)canvas
    fontCollection:(FontmgrCollection*)font_collection {
  ParagraphTest paragraph_test(kSelfRendering, font_collection);
  paragraph_test.TestWithId(canvas, 500, id);
}
@end
