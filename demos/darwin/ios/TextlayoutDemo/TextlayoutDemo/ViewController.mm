// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ViewController.h"

#include <vector>

#import "CoreTextTestView.h"
#import "TTTextView.h"
#import "TestDrawer.h"
#import "UIView+TextKit.h"
#import "UIView+TextKit2View.h"
@interface ViewController ()
@property(nonatomic) IBOutlet TTTextView* tttext_view;
@property(nonatomic) IBOutlet TextKit2View* textkit2_view;
@property(nonatomic) IBOutlet TextKitView* textkit_view;
@property(nonatomic) IBOutlet UITextView* log_view;
@property(nonatomic) IBOutlet UIButton* test_btn;
@property(nonatomic) IBOutlet UISwitch* enable_random_style;
@property(nonatomic) IBOutlet UISwitch* enable_english_test;
@property(readwrite) NSMutableAttributedString* attributedString;
@property(readwrite) int test_count;
@property(readwrite) float tttext_costs;
@property(readwrite) float tttext_draw_costs;
@property(readwrite) uint32_t tttext_char_count;
@property(readwrite) float tk2_costs;
@property(readwrite) float tk2_draw_costs;
@property(readwrite) uint32_t tk2_char_count;
@property(readwrite) float tk_costs;
@property(readwrite) float tk_draw_costs;
@property(readwrite) uint32_t tk_char_count;
@property(readwrite) float yytext_costs;
@property(readwrite) NSMutableArray<NSMutableAttributedString*>* paragraph_list;
@property(readwrite) NSString* content;
@property(readwrite) NSMutableArray<NSString*>* substrings;
@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  //  NSString *file = [[NSBundle mainBundle] pathForResource:@"很多字.txt"
  //                                                   ofType:nil];

  //  str = [str substringWithRange:NSMakeRange(0, str.length)];
  //  self.content = [str cStringUsingEncoding:NSUTF8StringEncoding];
  //  str = @"العربية";
  //  str = @"בעברית";
  //  str = @"קטע";
  //  str = @"זה קטע בעברית.";
  //  str = @"שלום, זה קטע בעברית.";
  //  str =
  //  @"中文حبًا\rशैक्षणिक中国\rวิ中\r加了一堆很长很长。مرحبًا ، هذه فقرة باللغة "
  //  @"العربية."
  //  @"为了测试所以加了一堆很长很长。\r为了测试所以加了一堆很长很长。שלום, זה "
  //  @"קטע בעברית. Word 和 WPS "
  //  @"可以说将桌面客户端中的富文本编辑器做到了极致，至今也是功能强大的富文本"
  //  @"编辑器。هذه فقرةفي .الل غة العر بي";
  //    str = @"शैक्षणिक सत्र 2019-20 के लिए निम्नलिखित समितियों का गठन किया जाता है I
  // सम्बंधित समिति के प्रभारी एवं सदस्यों से निवेदन है कि वे दी गयी जिम्मेदारी को पूर्ण मनोयोग
  // से निभाये ताकि विद्यालय का चहुमुखी विकास हो सके I सम्बंधित विभाग के लिए के वि सं /
  // अधोहस्ताक्षरी द्वारा दिए गए / दिए जाने वाले निर्देशों  के अनुसार  निर्धारित समय सीमाओं
  // का पालन करते हुए प्रदत्त कार्यों को पूरा किया जाए Iयह आदेश दिनांक 06/04/2019 से
  // प्रभावी माना  जाये . यदि विभागों में कोई बदलाव है तो 08/04/2019 तक सम्बंधित विभाग
  // का चार्ज नवनियुक्त शिक्षक/कर्मचारी/प्रभारी द्वारा ले लिया जाये I";     str =
  //@"วิกฤติเศรษฐกิจครั้งร้ายแรงที่สุดของประเทศไทยในปี พ.ศ. 2540
  // ทำให้มุมมองของนักธุรกิจขนาดใหญ่ต่อสังคมการเมืองเปลี่ยนแปลงไป
  // นักธุรกิจหมดหวังและเริ่มไม่เชื่อมั่นในความรู้ความสามารถของนักการเมืองและพรรคการเมืองในขณะนั้นว่าจะนำพาเศรษฐกิจของประเทศต่อไปได้อย่างราบรื่น";
  // str = @"中文";
  // str = @"中文حبًا\rशैक्षणिक中国\rวิ中\rวิกฤติเศรษฐกิจครั้งร้ายแรงที่สุดของประเทศไทยในปี
  // พ.ศ. 2540 ทำให้มุมมองของนักธุรกิจขนาดใหญ่ต่อสังคมการเมืองเปลี่ยนแปลงไป
  // ะนำพาเศรษฐกิจของประเทศต่อไปได้อย่างราบรื่น\r शैक्षणिक सत्र 2019-20 के लिए निम्नलिखित
  // समितियों का गठन किया जाता है I सम्बंधित समिति के प्रभारी एवं सदस्यों से निवेदन है कि वे
  // दी गयी जिम्मेदारी को पूर्ण मनोयोग से निभाये ताकि विद्यालय का चहुमुखी विकास हो सके I
  // सम्बंधित विभाग के लिए के वि सं \r加了一堆很长很长。مرحبًا ، هذه فقرة باللغة
  // العربية.为了测试所以加了一堆很长很长。为了测试所以加了一堆很长很长。שלום,
  // זה קטע בעברית. Word 和 WPS
  // 可以说将桌面客户端中的富文本编辑器做到了极致，至今也是功能强大的富文本编辑器。\r
  // مرحبًا ، هذه فقرة باللغة العربية. المسؤول عن المقررات ومفرداتها والمراجع
  // المعتمدة لها هم السادة أساتذة المقررات بحسب الجدول الآتي في الأسفل.";
  //  str = @"中文حبًا";
  //  str = @"萨伊的手指向了他——";
  // str = @"\357\273\277 \357 \273\277\357\273\277";
  //  CoreTextTestView *cttv = [[CoreTextTestView alloc]
  //  initWithFrame:self.view.bounds]; cttv.attributedString =
  //  self.attributedString; [self.view addSubview:cttv];
  //  UIButton *btn = [UIButton buttonWithType:UIButtonTypeSystem];
  //  auto screen_bounds = [UIScreen mainScreen].bounds;
  //  btn.frame = CGRectMake(screen_bounds.size.width-200,
  //  screen_bounds.origin.y, 150, 50); [btn setTitle:@"Performance Test"
  //  forState:UIControlStateNormal]; btn.backgroundColor =
  //  UIColor.darkGrayColor; btn.layer.cornerRadius = 20;
  [self.test_btn addTarget:self
                    action:@selector(OnStartPerformance:)
          forControlEvents:UIControlEventTouchUpInside];
}

- (void)PrepareTestContent {
  NSString* file;
  if ([self.enable_english_test isOn]) {
    file = [[NSBundle mainBundle] pathForResource:@"english.txt" ofType:nil];
  } else {
    file = [[NSBundle mainBundle] pathForResource:@"红楼梦.txt" ofType:nil];
  }

  self.substrings = [NSMutableArray array];
  self.content = [NSString stringWithContentsOfFile:file
                                           encoding:NSUTF8StringEncoding
                                              error:nil];
  [self.content
      enumerateSubstringsInRange:NSMakeRange(0, [self.content length])
                         options:NSStringEnumerationByComposedCharacterSequences
                      usingBlock:^(NSString* substring, NSRange substringRange,
                                   NSRange enclosingRange, BOOL* stop) {
                        if (substring && ![substring isEqualToString:@"\n"] &&
                            ![substring isEqualToString:@"\r"]) {
                          [self.substrings addObject:substring];
                        }
                      }];
}

- (void)OnStartPerformance:(UIButton*)sender {
  [self PrepareTestContent];
  [self performancTest];
}
- (void)performancTest {
  [self pre_layout];
  self.test_count = 100;
  self.tttext_costs = 0;
  self.tttext_draw_costs = 0;
  self.tttext_char_count = 0;
  self.tk2_costs = 0;
  self.tk2_draw_costs = 0;
  self.tk2_char_count = 0;
  self.tk_costs = 0;
  self.tk_draw_costs = 0;
  self.tk_char_count = 0;
  dispatch_after(
      dispatch_time(DISPATCH_TIME_NOW, 0), dispatch_get_main_queue(), ^{
        for (auto kk = 0; kk < self.test_count; kk++) {
          uint32_t char_start = arc4random_uniform(
              static_cast<uint32_t>([self.substrings count]));
          uint32_t char_count =
              arc4random_uniform(
                  fmin(1000, [self.substrings count] - char_start - 1)) +
              1;
          NSString* content = [[NSString alloc] init];
          for (auto k = 0; k < char_count; k++) {
            content = [content
                stringByAppendingString:[self.substrings
                                            objectAtIndex:char_start + k]];
          }
          [self initTestContent:char_start char_count:char_count];
          [self checklayouttime_textkit_with_content:content
                                          char_count:char_count];
          [self checklayouttime_tttext_with_content:content
                                         char_count:char_count];
          [self checklayouttime_textkit2_with_content:content
                                           char_count:char_count];
        }
        [self printSummary];
      });
}

- (void)printSummary {
  NSString* log;
  log =
      [NSString stringWithFormat:
                    @"lynxtextra:\n\tlayout speed:%5.2f(ms/1000 chars)\n\tdraw "
                    @"speed:%5.2f(ms/1000 chars)\n",
                    self.tttext_costs / self.test_count,
                    self.tttext_draw_costs / self.test_count];
  [self.log_view insertText:log];
  NSLog(@"%@", log);
  log = [NSString stringWithFormat:
                      @"Textkit1:\n\tlayout speed:%5.2f(ms/1000 chars)\n\tdraw "
                      @"speed:%5.2f(ms/1000 chars)\n",
                      self.tk_costs / self.test_count,
                      self.tk_draw_costs / self.test_count];
  [self.log_view insertText:log];
  NSLog(@"%@", log);
  log = [NSString stringWithFormat:
                      @"Textkit2:\n\tlayout speed:%5.2f(ms/1000 chars)\n\tdraw "
                      @"speed:%5.2f(ms/1000 chars)\n",
                      self.tk2_costs / self.test_count,
                      self.tk2_draw_costs / self.test_count];
  [self.log_view insertText:log];
  NSLog(@"%@", log);
}

- (void)pre_layout {
  NSString* content = @"hello testing...";
  uint32_t char_count = static_cast<uint32_t>(
      [content lengthOfBytesUsingEncoding:NSUTF32StringEncoding]);
  [_tttext_view createParagraph];
  [_textkit_view createParagraph];
  [_textkit2_view createParagraph];

  [_tttext_view
      addTextRunWithContent:[content cStringUsingEncoding:NSUTF8StringEncoding]
                      color:0xFFFF0000
                   fontSize:24];
  [_textkit_view
      addTextRunWithContent:[content cStringUsingEncoding:NSUTF8StringEncoding]
                      color:0xFF00FF00
                   fontSize:24];
  [_textkit2_view
      addTextRunWithContent:[content cStringUsingEncoding:NSUTF8StringEncoding]
                      color:0xFF0000FF
                   fontSize:24];

  [self checklayouttime_textkit_with_content:content char_count:char_count];
  [self checklayouttime_textkit2_with_content:content char_count:char_count];
  [self checklayouttime_tttext_with_content:content char_count:char_count];
}

- (void)initTestContent:(uint32_t)start_char char_count:(uint32_t)char_count {
  [_tttext_view createParagraph];
  [_textkit_view createParagraph];
  [_textkit2_view createParagraph];
  uint32_t argb = 0xFF00ff00;
  float font_size = 24;
  for (auto k = 0; k < char_count; k++) {
    if ([self.enable_random_style isOn]) {
      uint8_t red = arc4random_uniform(256);
      uint8_t green = arc4random_uniform(256);
      uint8_t blue = arc4random_uniform(256);
      argb = (0xFF << 24) | (red << 16) | (green << 8) | blue;
      font_size = arc4random_uniform(24) + 24;
    }

    [_tttext_view
        addTextRunWithContent:[[self.substrings objectAtIndex:start_char + k]
                                  cStringUsingEncoding:NSUTF8StringEncoding]
                        color:argb
                     fontSize:font_size];
    [_textkit_view
        addTextRunWithContent:[[self.substrings objectAtIndex:start_char + k]
                                  cStringUsingEncoding:NSUTF8StringEncoding]
                        color:argb
                     fontSize:font_size];
    [_textkit2_view
        addTextRunWithContent:[[self.substrings objectAtIndex:start_char + k]
                                  cStringUsingEncoding:NSUTF8StringEncoding]
                        color:argb
                     fontSize:font_size];
  }
}

- (void)checklayouttime_tttext_with_content:(NSString*)content
                                 char_count:(uint32_t)char_count {
  NSTimeInterval start, end;
  float duration;

  start = [[NSDate date] timeIntervalSince1970];
  [_tttext_view layoutWithContent];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float length = (float)char_count / 1000.f;
  float speed = duration / length;
  start = [[NSDate date] timeIntervalSince1970];
  [_tttext_view drawOnLayer];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float draw_speed = duration / length;

  self.tttext_costs += speed;
  self.tttext_draw_costs += draw_speed;
  self.tttext_char_count += char_count;
  [self.tttext_view setNeedsDisplay];
}

- (void)checklayouttime_textkit2_with_content:(NSString*)content
                                   char_count:(uint32_t)char_count {
  NSTimeInterval start, end;
  float duration;

  start = [[NSDate date] timeIntervalSince1970];
  [_textkit2_view layoutWithContent];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float length = (float)char_count / 1000.f;
  float speed = duration / length;
  start = [[NSDate date] timeIntervalSince1970];
  [_textkit2_view drawOnLayer];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float draw_speed = duration / length;

  self.tk2_costs += speed;
  self.tk2_draw_costs += draw_speed;
  self.tk2_char_count += char_count;
  [self.textkit2_view setNeedsDisplay];
}

- (void)checklayouttime_textkit_with_content:(NSString*)content
                                  char_count:(uint32_t)char_count {
  NSTimeInterval start, end;
  float duration;

  start = [[NSDate date] timeIntervalSince1970];
  [_textkit_view layoutWithContent];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float length = (float)char_count / 1000.f;
  float speed = duration / length;
  start = [[NSDate date] timeIntervalSince1970];
  [_textkit_view drawOnLayer];
  end = [[NSDate date] timeIntervalSince1970];
  duration = (float)((end - start) * 1000.f);
  float draw_speed = duration / length;

  self.tk_costs += speed;
  self.tk_draw_costs += draw_speed;
  self.tk_char_count += char_count;
  [self.textkit_view setNeedsDisplay];
}

@end
