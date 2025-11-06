// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"

#include "paragraph_test.h"  // NOLINT

#include <textra/fontmgr_collection.h>
#include <textra/i_font_manager.h>
#include <textra/layout_definition.h>
#include <textra/layout_drawer.h>
#include <textra/macro.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

ParagraphTest::ParagraphTest(ShaperType type,
                             FontmgrCollection* font_collection)
    : shaper_type_(type),
      font_collection_(std::move(font_collection)),
      painter_(nullptr) {}
void ParagraphTest::DrawParagraph(ICanvasHelper* canvas, Paragraph& paragraph,
                                  float width, LayoutMode width_mode,
                                  LayoutMode height_mode, uint32_t start,
                                  uint32_t end) const {
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 500, width_mode,
                                                 LayoutMode::kAtMost);
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  auto& page = *page_ptr;
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  if (start == 0 && end >= paragraph.GetCharCount()) {
    drawer.DrawLayoutPage(&page);
  } else {
    for (uint32_t i = 0; i < page.GetLineCount(); i++) {
      auto* line = page.GetLine(i);
      auto start_pos = line->GetStartCharPos();
      auto line_start = std::max(start, start_pos) - start_pos;
      auto line_end = std::min(end, line->GetEndCharPos()) - start_pos;
      if (line_end > line_start) {
        drawer.DrawTextLine(line, line_start, line_end);
      }
    }
  }
  auto paint = canvas->CreatePainter();
  auto page_width = page.GetPageWidth();
  auto page_height = page.GetPageHeight();
  auto layout_width = page.GetLayoutedWidth();
  auto layout_height = page.GetLayoutedHeight();
  if (draw_page_bound_) {
    paint->SetStrokeColor(TTColor::BLACK);
    canvas->DrawRect(0, 0, page_width, page_height, paint.get());
  }
  if (draw_layouted_bound_) {
    paint->SetStrokeColor(TTColor::GREEN);
    canvas->DrawRect(0, 0, layout_width, layout_height, paint.get());
  }
}

std::vector<std::pair<std::string, ParagraphTest::TestCaseFunction>>
ParagraphTest::GetTestCases() {
  static const std::vector<
      std::pair<std::string, ParagraphTest::TestCaseFunction>>
      kTestCases = {
          // human-friendly test name, function
          {"TestLargeImg", &ParagraphTest::TestLargeImg},
          {"TestSupSub", &ParagraphTest::TestSupSub},
          {"BlockTest", &ParagraphTest::BlockTest},
          {"ParagraphBlockTest", &ParagraphTest::ParagraphBlockTest},
          {"TestMultiParagraph", &ParagraphTest::TestMultiParagraph},
          {"TestDecoration", &ParagraphTest::TestDecoration},
          {"TestOneString", &ParagraphTest::TestOneString},
          {"TestLineSpacing", &ParagraphTest::TestLineSpacing},
          {"TestMaxLine", &ParagraphTest::TestMaxLine},
          {"TestPageHeight", &ParagraphTest::TestPageHeight},
          {"TestGhostContent", &ParagraphTest::TestGhostContent},
          {"TestEllipsis", &ParagraphTest::TestEllipsis},
          {"TestWordBreak", &ParagraphTest::TestWordBreak},
          {"TestLigature", &ParagraphTest::TestLigature},
          {"ShapeWithOffsetYTest", &ParagraphTest::ShapeWithOffsetYTest},
          {"TestFontStyle", &ParagraphTest::TestFontStyle},
          {"TestCRLF", &ParagraphTest::TestCRLF},
          {"TestLineBreak", &ParagraphTest::TestLineBreak},
          {"TestLayoutMode", &ParagraphTest::TestLayoutMode},
          {"TestLayoutMode2", &ParagraphTest::TestLayoutMode2},
          {"TestRTLText", &ParagraphTest::TestRTLText},
          {"TestMultiStyleInWord", &ParagraphTest::TestMultiStyleInWord},
          {"TestLetterSpacing", &ParagraphTest::TestLetterSpacing},
          {"TestLineHeight", &ParagraphTest::TestLineHeight},
          {"TestFontCollection", &ParagraphTest::TestFontCollection},
          {"TestLineHeightOverride", &ParagraphTest::TestLineHeightOverride},
          {"TestEmoji", &ParagraphTest::TestEmoji},
          {"TestIntrinsicWidth", &ParagraphTest::TestIntrinsicWidth},
          {"TestLongestLine", &ParagraphTest::TestLongestLine},
          {"TestAlignment", &ParagraphTest::TestAlignment},
          {"TestBaselineOffset", &ParagraphTest::TestBaselineOffset},
          {"TestPieceDraw", &ParagraphTest::TestPieceDraw},
          {"TestWordBoundary", &ParagraphTest::TestWordBoundary},
          {"TestRunDelegateEllipsis", &ParagraphTest::TestRunDelegateEllipsis},
          {"TestWhiteSpaceBreakLine", &ParagraphTest::TestWhiteSpaceBreakLine},
          {"TestLayoutedWidth", &ParagraphTest::TestLayoutedWidth},
          {"TestCharacterVerticalAlignment",
           &ParagraphTest::TestCharacterVerticalAlignment},
          {"TestParagraphVerticalAlignment",
           &ParagraphTest::TestParagraphVerticalAlignment},
          {"TestItalicFont", &ParagraphTest::TestItalicFont},
          {"TestCJKBreak", &ParagraphTest::TestCJKBreak},
          {"TestAlignWithBBox", &ParagraphTest::TestAlignWithBBox},
          {"TestModifyHAlignAfterLayout",
           &ParagraphTest::TestModifyHAlignAfterLayout},
          {"TestApplyStyleInRange", &ParagraphTest::TestApplyStyleInRange},
      };
  return kTestCases;
}

std::vector<std::pair<uint32_t, std::string>>
ParagraphTest::GetTestCaseIdAndName() {
  auto test_cases = GetTestCases();
  std::vector<std::pair<uint32_t, std::string>> test_case_list;
  for (size_t idx = 0; idx < test_cases.size(); ++idx) {
    auto [test_name, _] = test_cases[idx];
    test_case_list.emplace_back(idx, test_name);
  }
  return test_case_list;
}

void ParagraphTest::TestWithId(ICanvasHelper* canvas, float width,
                               uint32_t test_id) {
  auto test_cases = GetTestCases();
  if (test_id < test_cases.size()) {
    TestCaseFunction test_case_func = std::get<1>(test_cases[test_id]);
    (this->*test_case_func)(canvas, width);
  }
}
void ParagraphTest::DefaultTest(ICanvasHelper* canvas) {
  std::string para_content1 =
      "如果上面加上一行。This is a line of English text.\n"
      "上面再加上一行。\n"
      "\"[ Chapter 1 ]\"هذا نص عربي لا معنى له ، فقط لأغراض الاختبار ، طالما "
      "أنه طويل بما فيه الكفاية ، فكل شيء آخر لا يهم.\n"
      "这一句的显示顺序有问题：为了测试所以加了一堆很长很长啥意义都没有的有的没"
      "的。ه ذ 123 ه فقر ةفي  ا للغ ة الع ربي "
      "ة.中间插入了一段阿拉伯文，阿拉伯文中间又插入了阿拉伯数字。(And there is "
      "also English.)\n"
      "ما هي تجربة إدخال中文文本 الصينية\n"
      "ما هي تجربة إدخال中文文本 الصينية所以在这前面又插上一句\n"
      "123 ه فقر ةفي ا للغ";
  std::string para_hebrew = "שלום, זה קטע בעברית.";
  std::string word_hebrew = "שלום שלום";
  std::string para_arabic =
      "مرحبًا ، هذه فقرة باللغة العربية. المسؤول عن المقررات ومفرداتها والمراجع "
      "المعتمدة لها هم السادة أساتذة المقررات بحسب الجدول الآتي في الأسفل.";
  std::string para_content3 =
      "Word 和 WPS "
      "可以说将桌面客户端中的富文本编辑器做到了极致，至今也是功能强大的富文本编"
      "辑器。هذه فقرةفي الل غة العر بي\n"
      "但是它们的设计初衷就是做一款单机的文字处理软件，自然会遇到不支持互联网上"
      "的音视频格式、存储备份依靠本地计算机的文件系统、多人协作依靠文件拷贝等问"
      "题。中华人民共和国 english text 123456 ه ذ ه فقرةفي ا للغة ا لعر بية "
      "This is an english paragraph";
  std::string para_content4 =
      "向 app 的忠实用户，邀请大家一起来找出并修复 app 的 "
      "bug，一起见证 app 的成长。参与内测有机会获得金币、周边等奖励哦。\n4）"
      "“我也有着想要写书的梦想”\n杯里泡枸杞：我是个学生党，但是我也有着想要写书"
      "的梦想。可我不知道怎么做，官方可以实现我的梦想吗？\nLDY：啥时候可以自己"
      "写书？？？\n……\n回答：每个资深书虫都一定动过自己亲自写一本书的念头，如"
      "果你想尝试，可以点开 app 旗下原创文学平台\n";
  std::string para_content5 =
      "如果上面加上一行。This is a line of English "
      "text.\n上面再加上一行。\n\"[ Chapter 1 ]\"هذا نص عربي لا معنى له ، فقط "
      "لأغراض الاختبار ، طالما أنه طويل بما فيه الكفاية ، فكل شيء آخر لا "
      "يهم."
      "这一句的显示顺序有问题：为了测试所以加了一堆很长很长啥意义都没有的有的没"
      "的。ه ذ 123 ه فقر ةفي  ا للغ ة الع ربي "
      "ة.中间插入了一段阿拉伯文，阿拉伯文中间又插入了阿拉伯数字。(And there is "
      "also English.)\nما هي تجربة إدخال中文文本 الصينية ما هي تجربة "
      "إدخال中文文本 الصيني所以在这前面又插上一句123 ه فقر ةفي ا للغ";
  std::string para_content6 =
      "为了测试所以加了一堆很长很长啥意义都没有的有的没的。ه ذ 123 ه فقر ةفي  "
      "ا للغ ة الع ربي ة.中间插入了一段阿拉伯文";
  std::string word_tai = "วิ中";
  std::string para_tai =
      "วิกฤติเศรษฐกิจครั้งร้ายแรงที่สุดของประเทศไทยในปี พ.ศ. 2540 "
      "ทำให้มุมมองของนักธุรกิจขนาดใหญ่ต่อสังคมการเมืองเปลี่ยนแปลงไป  "
      "นักธุรกิจหมดหวังและเริ่มไม่เชื่อมั่นในความรู้ความสามารถของนักการเมืองและพรรคการเมืองในขณ"
      "ะนั้นว่าจะนำพาเศรษฐกิจของประเทศต่อไปได้อย่างราบรื่น\nวิกฤติเศรษฐกิจไม่เพียงแต่ทำลายความ"
      "เชื่อมั่นของกลุ่มทุนขนาดใหญ่ต่อสังคมการเมืองเท่านั้น  "
      "ที่สำคัญทำให้พวกเขาตัดสินใจสร้างพรรคการเมืองของตัวเองขึ้น  "
      "ส่งผลให้บทบาททางการเมืองของพวกเขาเปลี่ยนแปลงไปอย่างสำคัญ กล่าวคือ "
      "จากที่เคยเล่นบทอยู่หลังฉากทางการเมือง  "
      "พวกเขาได้ก้าวออกมาเป็นตัวแสดงบนเวทีทางการเมืองด้วยตัวเอง\nหลังจากก่อตั้งพรรคการเมืองไ"
      "ด้ราว 1 ปี  กลุ่มทุนที่ใช้นโยบายจากลัทธิอรรถประโยชน์ (Utilitarianism) "
      "ที่ว่ารัฐบาลประชาธิปไตยมีหน้าที่ทำให้ประชาชนมีความสุขมากที่สุด  "
      "ก็สามารถชนะการเลือกตั้งอย่างถล่มทลาย  และมีอำนาจติดต่อกันหลายปี  "
      "วาทกรรมของกลุ่มทุนเรื่องเสรีภาพในการแข่งขันอย่างไม่ขีดจำกัด  "
      "ในท้ายที่สุดกลับพบว่าเป็นการเอื้อต่อการแสวงหาสัมปทาน  "
      "และการผูกขาดโครงการขนาดใหญ่ของภาครัฐ";
  std::string para_indi =
      "शैक्षणिक सत्र 2019-20 के लिए निम्नलिखित समितियों का गठन किया जाता है I सम्बंधित "
      "समिति के प्रभारी एवं सदस्यों से निवेदन है कि वे दी गयी जिम्मेदारी को पूर्ण मनोयोग से "
      "निभाये ताकि विद्यालय का चहुमुखी विकास हो सके I सम्बंधित विभाग के लिए के वि सं / "
      "अधोहस्ताक्षरी द्वारा दिए गए / दिए जाने वाले निर्देशों  के अनुसार  निर्धारित समय "
      "सीमाओं का पालन करते हुए प्रदत्त कार्यों को पूरा किया जाए Iयह आदेश दिनांक "
      "06/04/2019 से प्रभावी माना  जाये . यदि विभागों में कोई बदलाव है तो 08/04/2019 "
      "तक सम्बंधित विभाग का चार्ज नवनियुक्त शिक्षक/कर्मचारी/प्रभारी द्वारा ले लिया जाये I";
  std::string word_indi = "शैक्षणिक中国";
  std::string word_chinese = "中国";
  std::string word_arabic_eng = "حبًاABC";
  std::string word_arabic_chinese = "中文حبًا";
  std::string para_mic =
      "中文حبًا\nशैक्षणिक中国\nวิ中\n วิกฤติเศรษฐกิจครั้งร้ายแรงที่สุดของประเทศไทยในปี พ.ศ. "
      "2540 ทำให้มุมมองของนักธุรกิจขนาดใหญ่ต่อสังคมการเมืองเปลี่ยนแปลงไป  "
      "ะนำพาเศรษฐกิจของประเทศต่อไปได้อย่างราบรื่น शैक्षणिक सत्र 2019-20 के लिए "
      "निम्नलिखित समितियों का गठन किया जाता है I सम्बंधित समिति के प्रभारी एवं सदस्यों से "
      "निवेदन है कि वे दी गयी जिम्मेदारी को पूर्ण मनोयोग से निभाये ताकि विद्यालय का "
      "चहुमुखी विकास हो सके I सम्बंधित विभाग के लिए के वि सं / अधोहस्ताक्षरी द्वारा दिए "
      "गए / दिए जाने वाले निर्देशों  के अनुसार  निर्धारित समय सीमाओं का पालन करते हुए "
      "प्रदत्त कार्यों को पूरा किया जाए Iयह आदेश दिनांक 06/04/2019 से प्रभावी माना  जाये "
      ". यदि विभागों में कोई बदलाव है तो 08/04/2019 तक सम्बंधित विभाग का चार्ज "
      "नवनियुक्त शिक्षक/कर्मचारी/प्रभारी द्वारा ले लिया जाये I "
      "加了一堆很长很长。مرحبًا ، هذه فقرة باللغة "
      "العربية."
      "为了测试所以加了一堆很长很长。为了测试所以加了一堆很长很长。שלום, זה "
      "קטע בעברית. Word 和 WPS "
      "可以说将桌面客户端中的富文本编辑器做到了极致，至今也是功能强大的富文本编"
      "辑器。\r  مرحبًا ، هذه فقرة باللغة العربية. المسؤول عن المقررات "
      "ومفرداتها والمراجع المعتمدة لها هم السادة أساتذة المقررات بحسب الجدول "
      "الآتي في الأسفل.";
  std::string word_arabic = "江南：(⸝⸝･᷄ٹ･᷅)";
  std::string word_test = "ⓗⓔⓛⓛⓞ";
  std::string control_text = "zhong\r\f\u2028\n";
  std::vector<std::string> para_content_vec = {
      para_content1,   word_hebrew,
      para_hebrew,     para_arabic,
      para_content3,   para_indi,
      para_tai,        word_tai,
      word_arabic_eng, word_arabic_chinese,
      word_indi,       word_chinese,
      word_arabic,     word_test};

  for (auto& text : para_content_vec) {
    TestSkParagraph(canvas, text);
    TestTTParagraph(canvas, text, test_width);
  }
}
void ParagraphTest::TestSkParagraph(ICanvasHelper* canvas,
                                    const std::string& text) {
#ifdef SKP_TEST
  font_collection_->setDefaultFontManager(SkFontMgr::RefDefault());
  skia::textlayout::ParagraphStyle paragraph_style;
  paragraph_style.setTextAlign(skia::textlayout::TextAlign::kLeft);
  paragraph_style.setTextDirection(skia::textlayout::TextDirection::kLtr);
  TextStyle text_style;
  text_style.setFontSize(20);
  text_style.setColor(SK_ColorBLACK);
  paragraph_style.setTextStyle(text_style);
  auto builder = TTText::make(paragraph_style, font_collection_);
  builder->addText(text.c_str(), text.length());
  auto paragraph = builder->Build();
  auto start = clock();
  paragraph->layout(test_width);
  LogUtil::E("SkParagraph Layout cost:%f",
             (clock() - start) * 1000.f / CLOCKS_PER_SEC);
  paragraph->paint(canvas, 500, 0);
#endif
}
void ParagraphTest::TestTTParagraph(ICanvasHelper* canvas,
                                    const std::string& text,
                                    float width) const {
  auto paragraph = Paragraph::Create();
  auto& ppr = paragraph->GetParagraphStyle();
  ppr.SetWriteDirection(WriteDirection::kLTR);
  ppr.SetHorizontalAlign(ParagraphHorizontalAlignment::kLeft);
  Style run_pr;
  run_pr.SetTextSize(20);
  paragraph->AddTextRun(&run_pr, text.c_str(),
                        static_cast<uint32_t>(text.length()));

  // auto start = clock();
  DrawParagraph(canvas, *paragraph, width, LayoutMode::kDefinite,
                LayoutMode::kIndefinite);
  // // LogUtil::E("TTTextLayout Layout cost:%f",
  // //            (clock() - start)*  1000.f / CLOCKS_PER_SEC);
  // LayoutDrawer ld(canvas);
  // ld.DrawLayoutPage(page_list[0].get());
  //
  // auto height = page_list[0]->GetLayoutedHeight();
  // auto width = page_list[0]->GetLayoutedWidth();
  // auto paint = canvas->CreatePainter();
  // paint->SetFillStyle(FillStyle::kStroke);
  // paint->SetColor(0xFFFF0000);
  // canvas->DrawRect(0, 0, width, height, paint.get());
  // width = page_list[0]->GetPageWidth();
  // height = page_list[0]->GetPageHeight();
  // paint->SetColor(0xFF00FF00);
  // canvas->DrawRect(0, 0, width, height, paint.get());
}
void ParagraphTest::DisplayInfo(ICanvasHelper* canvas, float width) const {}
void ParagraphTest::BlockTest(ICanvasHelper* canvas, float width) const {
  // Note: not implemented
}
void ParagraphTest::ParagraphBlockTest(ICanvasHelper* canvas,
                                       float width) const {
  // Note: not implemented
}
void ParagraphTest::TestSupSub(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  Style run_style;
  float base_size = 12;
  std::string content = "中国中国中国中国";
  run_style.SetTextSize(base_size);
  paragraph.AddTextRun(
      &run_style, content.c_str(),
      static_cast<uint32_t>(static_cast<uint32_t>(content.length())));
  run_style.SetTextScale(0.58);
  run_style.SetVerticalAlignment(CharacterVerticalAlignment::kSuperScript);
  content = "你好";
  paragraph.AddTextRun(&run_style, content.c_str(),
                       static_cast<uint32_t>(content.length()));
  run_style.SetVerticalAlignment(CharacterVerticalAlignment::kSubScript);
  paragraph.AddTextRun(&run_style, content.c_str(),
                       static_cast<uint32_t>(content.length()));
  run_style.SetTextScale(1.0f);
  run_style.SetVerticalAlignment(CharacterVerticalAlignment::kBaseLine);
  content = "正常正常正常正常正常正常正常正常正常";
  paragraph.AddTextRun(&run_style, content.c_str(),
                       static_cast<uint32_t>(content.length()));
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
}
void ParagraphTest::TestMultiStyleInWord(ICanvasHelper* canvas,
                                         float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kDistributed);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  paragraph.AddTextRun(&run_style, "two egg");
  TTColor color(TTColor::BLUE);
  run_style.SetForegroundColor(color);
  run_style.SetDecorationType(DecorationType::kUnderLine);
  color = TTColor ::RED;
  run_style.SetDecorationColor(color);
  paragraph.AddTextRun(&run_style, "s");
  color.SetColor(TTColor::GREEN);
  run_style.SetBackgroundColor(color);
  paragraph.AddTextRun(&run_style, "dse");
  run_style.SetDecorationType(DecorationType::kNone);
  run_style.SetBackgroundColor(TTColor(0));
  color = TTColor::BLACK;
  run_style.SetForegroundColor(color);
  paragraph.AddTextRun(&run_style, " qwerqwerqwerqwe");
  DrawParagraph(canvas, paragraph, width);
}
void ParagraphTest::TestMultiParagraph(ICanvasHelper* canvas,
                                       float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  // clang-format off
  paragraph.AddTextRun(
     & run_style,
      "You can\nhave Word hyphenate\r\nyour text automatically\n");
  // clang-format on
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
}
void ParagraphTest::TestLargeImg(ICanvasHelper* canvas, float width) const {
  TextLayout layout(font_collection_, shaper_type_);
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  TestShape object(test_width + 100, 100);
  paragraph.AddTextRun(nullptr, "\n");
  auto page = std::make_unique<LayoutRegion>(test_width, test_height);
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, page.get(), context);
  DrawParagraph(canvas, paragraph, width);
}
void ParagraphTest::TestOneString(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  // clang-format off
  paragraph.AddTextRun(
     & run_style,
      "############################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################################");
  // clang-format on
  //  run_style.SetTextSize(48);
  //  paragraph.ApplyStyleInRange(run_style, 4, 3);
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
  auto paint = canvas->CreatePainter();
  paint->SetStrokeColor(TTColor::BLACK);
  canvas->DrawRect(0, 0, page.GetPageWidth(), page.GetPageHeight(),
                   paint.get());
}
void ParagraphTest::TestLineSpacing(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  auto font_size = 24;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kJustify);
  paragraph.GetParagraphStyle().SetLineHeightInPxExact(font_size * 1.5);
  Style run_style;
  run_style.SetTextSize(font_size);
  // clang-format off
  paragraph.AddTextRun(
     & run_style,
      "You can have Word hyphenate your text automatically as you type your text, 24.5% or later, in one pass. You can also use manual hyphenation to have Word make hyphenation suggestions. Automatic hyphenation is quicker and easier, although manual hyphenation provides more control.\n"
      "“哦...emm...brotherbrother...dont...kill...me...no...no...no...”\n");
  // clang-format on
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
  auto paint = canvas->CreatePainter();
  paint->SetStrokeColor(TTColor::BLACK);
  canvas->DrawRect(0, 0, page.GetPageWidth(), font_size, paint.get());

  paragraph.GetParagraphStyle().SetLineHeightInPercent(1);
  page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  context.Reset();
  layout.Layout(&paragraph, &page, context);
  canvas->Save();
  canvas->Translate(width, 0);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), font_size, paint.get());
  canvas->Restore();
}

void ParagraphTest::TestDecoration(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  run_style.SetDecorationType(DecorationType::kUnderLine);
  run_style.SetDecorationColor(TTColor(0xFF000000));
  paragraph.AddTextRun(&run_style, "下划线 Underline");
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
  auto paint = canvas->CreatePainter();
  paint->SetStrokeColor(TTColor::BLACK);
  canvas->DrawRect(0, 0, page.GetPageWidth(), page.GetPageHeight(),
                   paint.get());
}
void ParagraphTest::TestMaxLine(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  auto font_size = 24;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  paragraph.GetParagraphStyle().SetMaxLines(3);
  Style run_style;
  run_style.SetTextSize(font_size);
  // clang-format off
  paragraph.AddTextRun(
     & run_style,
      "You can have Word hyphenate your text automatically as you type your text, 24.5% or later, in one pass. You can also use manual hyphenation to have Word make hyphenation suggestions. Automatic hyphenation is quicker and easier, although manual hyphenation provides more control.\n"
      "“哦...emm...brotherbrother...dont...kill...me...no...no...no...”\n");
  // clang-format on
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  auto& page = *page_ptr;
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  layout.Layout(&paragraph, &page, context);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
  auto paint = canvas->CreatePainter();
  paint->SetStrokeColor(TTColor::BLACK);
  canvas->DrawRect(0, 0, page.GetPageWidth(), font_size, paint.get());

  paragraph.GetParagraphStyle().SetMaxLines(0);
  page_ptr = std::make_unique<LayoutRegion>(width, 10000);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  context.Reset();
  layout.Layout(&paragraph, &page, context);
  canvas->Save();
  canvas->Translate(width, 0);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), font_size, paint.get());
  canvas->Restore();
}
void ParagraphTest::TestPageHeight(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  auto font_size = 24;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  run_style.SetTextSize(font_size);
  // clang-format off
  paragraph.AddTextRun(
     & run_style,
      "You can have Word hyphenate your text automatically as you type your text, 24.5% or later, in one pass. You can also use manual hyphenation to have Word make hyphenation suggestions. Automatic hyphenation is quicker and easier, although manual hyphenation provides more control.\n"
      "“哦...emm...brotherbrother...dont...kill...me...no...no...no...”\n");
  // clang-format on
  TTTextContext context;
  LayoutDrawer drawer(canvas);
  auto paint = canvas->CreatePainter();
  paint->SetStrokeColor(TTColor::BLACK);
  auto page_ptr = std::make_unique<LayoutRegion>(width, 50);

  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  TextLayout layout(font_collection_, shaper_type_);
  layout.Layout(&paragraph, page_ptr.get(), context);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), page_ptr->GetPageHeight(),
                   paint.get());

  paragraph.GetParagraphStyle().SetMaxLines(0);
  page_ptr = std::make_unique<LayoutRegion>(width, 40);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  context.Reset();
  layout.Layout(&paragraph, page_ptr.get(), context);
  canvas->Save();
  canvas->Translate(width, 0);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), page_ptr->GetPageHeight(),
                   paint.get());
  canvas->Restore();

  paragraph.GetParagraphStyle().SetMaxLines(0);
  page_ptr = std::make_unique<LayoutRegion>(width, 40);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(true);
  context.Reset();
  layout.Layout(&paragraph, page_ptr.get(), context);
  canvas->Save();
  canvas->Translate(2 * width, 0);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), page_ptr->GetPageHeight(),
                   paint.get());
  canvas->Restore();

  paragraph.GetParagraphStyle().SetMaxLines(0);
  page_ptr = std::make_unique<LayoutRegion>(width, 10);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  context.Reset();
  layout.Layout(&paragraph, page_ptr.get(), context);
  canvas->Save();
  canvas->Translate(0 * width, 100);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), page_ptr->GetPageHeight(),
                   paint.get());
  canvas->Restore();

  paragraph.GetParagraphStyle().SetMaxLines(0);
  page_ptr = std::make_unique<LayoutRegion>(width, 10);
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(true);
  context.Reset();
  layout.Layout(&paragraph, page_ptr.get(), context);
  canvas->Save();
  canvas->Translate(1 * width, 100);
  drawer.DrawLayoutPage(page_ptr.get());
  canvas->DrawRect(0, 0, page_ptr->GetPageWidth(), page_ptr->GetPageHeight(),
                   paint.get());
  canvas->Restore();
}
void ParagraphTest::TestGhostContent(ICanvasHelper* canvas, float width) const {
  // Note: not implemented
}
void ParagraphTest::TestLetterSpacing(ICanvasHelper* canvas,
                                      float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  run_style.SetTextSize(24);
  run_style.SetLetterSpacing(0);
  paragraph.AddTextRun(&run_style, "letter-spacing:0\n");
  paragraph.AddTextRun(&run_style, "中国人民共和国 letter-spacing:5\n");
  paragraph.AddTextRun(&run_style, "letter-spacing:5\n");
  run_style.SetLetterSpacing(5);
  paragraph.AddTextRun(&run_style, "中国人民共和国 letter-spacing:5\n");
  run_style.SetLetterSpacing(0);
  paragraph.AddTextRun(&run_style, "letter-spacing:-5\n");
  run_style.SetLetterSpacing(-5);
  paragraph.AddTextRun(&run_style, "中国人民共和国 letter-spacing:-5\n");
  DrawParagraph(canvas, paragraph, width);
}
void ParagraphTest::TestEllipsis(ICanvasHelper* canvas, float width) const {
  ParagraphStyle paragraph_style;
  paragraph_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kLeft);
  paragraph_style.SetEllipsis(u"…");
  auto paragraph = Paragraph::Create();
  paragraph->SetParagraphStyle(&paragraph_style);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  // clang-format off
  paragraph->AddTextRun(
     & run_style,
      "可以说将桌面客户端中的富文本编辑器做到了极致，至今也是功能强大的富文本编辑器”\n");
  // clang-format on
  DrawParagraph(canvas, *paragraph, width, LayoutMode::kAtMost);

  canvas->Translate(0, 100);
  paragraph_style.SetMaxLines(1);
  paragraph->SetParagraphStyle(&paragraph_style);
  DrawParagraph(canvas, *paragraph, width, LayoutMode::kAtMost);

  canvas->Translate(0, 100);
  paragraph = Paragraph::Create();
  paragraph->SetParagraphStyle(&paragraph_style);
  // bad case: \n in paragraph will not work with at most
  paragraph->AddTextRun(&run_style,
                        "可以\n说将桌面客户端中的富文本编辑器做到了极致，至今也"
                        "是功能强大的富文本编辑器”\n");
  DrawParagraph(canvas, *paragraph, width, LayoutMode::kAtMost);

  canvas->Translate(0, 100);
  paragraph_style.SetHorizontalAlign(ParagraphHorizontalAlignment::kRight);
  paragraph->SetParagraphStyle(&paragraph_style);
  DrawParagraph(canvas, *paragraph, width, LayoutMode::kAtMost);
}
void ParagraphTest::TestWordBreak(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  run_style.SetWordBreak(WordBreakType::kBreakAll);
  // clang-format off
  paragraph.AddTextRun(& run_style,
    "You can have Word hyphenate your text automatically as you type your text, "
    "24.5% or later, in one pass. You can also use manual hyphenation to have Word make "
    "hyphenation suggestions. Automatic hyphenation is quicker and easier, although manual "
    "hyphenation provides more control.\n"
    "“哦...emm...brotherbrother...dont...kill...me...no...no...no...”\n");
  // clang-format on
  DrawParagraph(canvas, paragraph, width);
}

void ParagraphTest::TestLigature(ICanvasHelper* canvas, float width) const {
#ifdef ANDROID
#else
  // auto fc = std::make_shared<FontCollection>();
  // auto font_mgr = std::make_shared<TestFontMgr>(
  //     std::vector<std::shared_ptr<ITypefaceHelper>>{TFH_DEFAULT});
  // fc->SetDefaultFontManager(IFontManager::RefDefault());
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  Style run_style;
  float base_size = 24;
  run_style.SetTextSize(base_size);
  // clang-format off
  paragraph.AddTextRun(& run_style,
    "This is a long and\n Honorificabilitudinitatibus");
  // clang-format on
  DrawParagraph(canvas, paragraph, width);
#endif
}

void ParagraphTest::ShapeWithOffsetYTest(ICanvasHelper* canvas,
                                         float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  Style default_rpr;
  default_rpr.SetTextSize(24);
  paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
  paragraph.AddTextRun(nullptr, "一二三\n");
  paragraph.AddTextRun(nullptr, "四五六");
  auto footnote_shape = std::make_unique<TestShape>(10, 10);
  // todo: the y offset doesn't work here
  paragraph.AddShapeRun(nullptr, std::move(footnote_shape), false);
  DrawParagraph(canvas, paragraph, width);
}
#define EXPECT_EQ(x, y) x
#define EXPECT_FLOAT_EQ(x, y) x
void ParagraphTest::TestFontStyle(ICanvasHelper* canvas, float width) const {
  Style default_rpr;
  default_rpr.SetTextSize(24);
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
  Style font_size_rpr;
  font_size_rpr.SetTextSize(30);
  paragraph.AddTextRun(&font_size_rpr, "字体大小\n");
  Style font_color_rpr;
  font_color_rpr.SetForegroundColor(TTColor(0xFF00FF00));
  paragraph.AddTextRun(&font_color_rpr, "字体颜色\n");
  Style font_background_color_rpr;
  font_background_color_rpr.SetBackgroundColor(TTColor(0xFFFF0000));
  paragraph.AddTextRun(&font_background_color_rpr, "字体背景颜色\n");
  Style font_weight_rpr;
  font_weight_rpr.SetBold(true);
  paragraph.AddTextRun(&font_weight_rpr, "字体粗细\n");
  // TODO(hfuttyh): italic not working
  Style font_italic_rpr;
  font_italic_rpr.SetItalic(true);
  paragraph.AddTextRun(&font_italic_rpr, "斜体字\n");
  Style decorator_rpr;
  decorator_rpr.SetDecorationType(DecorationType::kUnderLine);
  decorator_rpr.SetDecorationColor(TTColor(0xFF00FF00));
  paragraph.AddTextRun(&decorator_rpr, "下划线\n");
  Style text_stroke;
  paragraph.AddTextRun(&text_stroke, "文本描边:");
  text_stroke.SetTextStrokeStyle(TTColor::GREEN, 0.5);
  paragraph.AddTextRun(&text_stroke, "中国人, abc\n");
  DrawParagraph(canvas, paragraph, width);
}
void ParagraphTest::TestCRLF(ICanvasHelper* canvas, float width) const {
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  Style default_rpr;
  default_rpr.SetTextSize(24);
  paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
  paragraph.AddTextRun(nullptr, "硬换行\\n\n软换行\\r\r四五六");
  DrawParagraph(canvas, paragraph, width);
}

void ParagraphTest::TestLineBreak(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(24);
  auto paragraph_ptr = Paragraph::Create();
  auto& paragraph = *paragraph_ptr;
  paragraph.GetParagraphStyle().SetDefaultStyle(style);
  const auto* str = "thisisalongword";
  const int times = static_cast<int>(width / 24 / 9);
  std::string string;
  for (int i = 0; i < times; i++) {
    string += str;
    string += " ";
  }
  paragraph.AddTextRun(nullptr, string.c_str(),
                       static_cast<uint32_t>(string.length()));
  paragraph.AddTextRun(nullptr, string.c_str(),
                       static_cast<uint32_t>(string.length()));
  paragraph.AddTextRun(nullptr, string.c_str(),
                       static_cast<uint32_t>(string.length()));
  DrawParagraph(canvas, paragraph, width);
}
void ParagraphTest::TestLayoutMode(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(48);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  const auto* str = "根据《Global Change Biology》上的一项研究🇨🇳";
  para->AddTextRun(nullptr, str);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost);
  canvas->Translate(0, 200);
  DrawParagraph(canvas, *para, std::numeric_limits<float>::max(),
                LayoutMode::kAtMost);
  canvas->Translate(0, 200);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kCenter);
  DrawParagraph(canvas, *para, std::numeric_limits<float>::max(),
                LayoutMode::kAtMost);
}
void ParagraphTest::TestLayoutMode2(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kCenter);
  para->AddTextRun(
      nullptr,
      "根据《Global Change Biology》上的一项研究根据《Global Change "
      "Biology》上的一项研究根据《Global Change Biology》上的一项研究");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost);
  canvas->Translate(0, 200);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost);
}

void ParagraphTest::TestRTLText(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  //  const auto* str = "أنا معجب كبير!";
  const auto* str =
      "!أنا معجب Hello كبير\nאני מעריץ/ה גדול/ה!\nمیں بہت بڑا مداح ہوں!\nأنا "
      "معجب كبير!";
  std::string he = "אני מעריץ/ה גדול/ה!";
  std::string ur = "میں بہت بڑا مداح ہوں!";
  std::string arabic = "أنا معجب كبير!";
  std::string test_str =
      "هذا نص عربي لا معنى له ، فقط لأغراض الاختبار ، طالما أنه طويل بما فيه "
      "الكفاية ، فكل شيء آخر لا يهم";
  std::string cjk = "一段中文";
  std::string eng = "hello world";
  std::string emoji = "🤗🤗🤗";
  std::string cyrillic = "Кириллица";

  std::string para_content5 =
      "如果上面加上一行。This is a line of English "
      "text.\n上面再加上一行。\n\"[ Chapter 1 ]\"هذا نص عربي لا معنى له ، فقط "
      "لأغراض الاختبار ، طالما أنه طويل بما فيه الكفاية ، فكل شيء آخر لا "
      "يهم."
      "这一句的显示顺序有问题：为了测试所以加了一堆很长很长啥意义都没有的有的没"
      "的。ه ذ 123 ه فقر ةفي  ا للغ ة الع ربي "
      "ة.中间插入了一段阿拉伯文，阿拉伯文中间又插入了阿拉伯数字。(And there is "
      "also English.)\nما هي تجربة إدخال中文文本 الصينية ما هي تجربة "
      "إدخال中文文本 الصيني所以在这前面又插上一句123 ه فقر ةفي ا للغ";

  auto ppp = "عربي";

  auto multi_fallback = cjk + emoji + cyrillic + eng + cjk;

  para->AddTextRun(&style, str);
  DrawParagraph(canvas, *para, width);
}

void ParagraphTest::TestLineHeight(ICanvasHelper* canvas, float width) const {
  canvas->Save();
  Style style;
  style.SetTextSize(12);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  para->GetParagraphStyle().SetLineHeightInPxExact(5);
  auto shape = std::make_shared<TestShape>(10, 10);
  para->AddShapeRun(&style, shape, false);
  para->AddTextRun(
      nullptr,
      "根据《Global Change Biology》上的一项研究根据《Global Change "
      "Biology》上的一项研究根据《Global Change Biology》上的一项研究");
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxExact(10);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxExact(20);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(5);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(10);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(20);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Restore();
  canvas->Save();
  canvas->Translate(width, 0);
  auto image = std::make_unique<TestShape>(20, 20);
  para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  para->GetParagraphStyle().SetLineHeightInPxExact(5);
  para->AddTextRun(nullptr, "根据《Global Change Biology》上的一项研究");
  para->AddShapeRun(nullptr, std::move(image), false);
  para->AddTextRun(nullptr, "根据《Global Change Biology》上的一项研究");
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxExact(10);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxExact(20);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(5);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(10);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxAtLeast(20);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Restore();
}

void ParagraphTest::TestFontCollection(ICanvasHelper* canvas,
                                       float width) const {
#ifndef ANDROID
  FontStyle font_style(FontStyle::Weight::kNormal_Weight,
                       FontStyle::Width::kNormal_Width,
                       FontStyle::Slant::kUpright_Slant);
  auto default_font_mgr = font_collection_->GetDefaultFontManager();
  auto asset_font_mgr = font_collection_->GetAssetFontManager();
  auto menlo_typeface = default_font_mgr->matchFamilyStyle("Menlo", font_style);
  auto font_mgr = std::make_shared<TestFontMgr>(std::vector{menlo_typeface});
  font_collection_->SetDefaultFontManager(font_mgr);

  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);

  std::string para_content5 = "This is a FontCollection Test";

  para->AddTextRun(&style, para_content5.c_str());
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite,
                LayoutMode::kDefinite);
  canvas->Translate(0, 200);

  auto helvetcia_typeface =
      default_font_mgr->matchFamilyStyle("Helvetcia", font_style);
  auto new_asset_font_mgr =
      std::make_shared<TestFontMgr>(std::vector{helvetcia_typeface});
  font_collection_->SetAssetFontManager(new_asset_font_mgr);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite,
                LayoutMode::kDefinite);
  font_collection_->SetDefaultFontManager(default_font_mgr);
  font_collection_->SetAssetFontManager(asset_font_mgr);
#endif
}

void ParagraphTest::TestLineHeightOverride(ICanvasHelper* canvas,
                                           float width) const {
  canvas->Save();
  Style style;
  style.SetTextSize(12);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  para->GetParagraphStyle().SetLineHeightOverride(true);
  para->GetParagraphStyle().SetLineHeightInPercent(1);
  para->AddTextRun(
      nullptr,
      "根据《Global Change Biology》上的一项研究根据《Global Change "
      "Biology》上的一项研究根据《Global Change Biology》上的一项研究");
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightOverride(false);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Translate(0, 100);
  para->GetParagraphStyle().SetLineHeightInPxExact(12);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite);
  canvas->Restore();
}
void ParagraphTest::TestEmoji(ICanvasHelper* canvas, float width) const {
  const char* content = "🤗🤗🤗测试中文能看到中文吗🤗🤗🤗";
  TestTTParagraph(canvas, content, width);
}

void ParagraphTest::TestIntrinsicWidth(ICanvasHelper* canvas,
                                       float width) const {
  const char* content = "测试文本一\n测 试 文 本 二\n";
  const char* content2 = "测 试 文 本 二\n";
  TextLayout layout(font_collection_, shaper_type_);
  TTTextContext context;
  context.Reset();
  auto paragraph = Paragraph::Create();
  ParagraphStyle pstyle;
  pstyle.SetHorizontalAlign(ParagraphHorizontalAlignment::kRight);
  paragraph->SetParagraphStyle(&pstyle);
  paragraph->AddTextRun(nullptr, content);
  auto page_ptr = std::make_unique<LayoutRegion>(
      width, 500, LayoutMode::kAtMost, LayoutMode::kAtMost);
  layout.Layout(paragraph.get(), page_ptr.get(), context);

  auto paragraph2 = Paragraph::Create();
  ParagraphStyle pstyle2;
  pstyle2.SetHorizontalAlign(ParagraphHorizontalAlignment::kRight);
  paragraph2->SetParagraphStyle(&pstyle2);
  paragraph2->AddTextRun(nullptr, content2);
  auto page_ptr2 = std::make_unique<LayoutRegion>(
      width, 500, LayoutMode::kAtMost, LayoutMode::kAtMost);
  context.Reset();
  layout.Layout(paragraph2.get(), page_ptr2.get(), context);
  auto width1 = paragraph->GetMaxIntrinsicWidth();
  auto width2 = paragraph2->GetMaxIntrinsicWidth();
  TTASSERT(width1 == width2);
}

void ParagraphTest::TestLongestLine(ICanvasHelper* canvas, float width) const {
  const char* content = "测试文本一\n测 试 文 本 二\n";
  const char* content2 = "测 试 文 本 二\n";
  TextLayout layout(font_collection_, shaper_type_);
  auto paragraph = Paragraph::Create();
  ParagraphStyle pstyle;
  pstyle.SetHorizontalAlign(ParagraphHorizontalAlignment::kRight);
  paragraph->SetParagraphStyle(&pstyle);
  paragraph->AddTextRun(nullptr, content);
  auto page_ptr = std::make_unique<LayoutRegion>(
      width, 500, LayoutMode::kAtMost, LayoutMode::kAtMost);
  TTTextContext context;
  layout.Layout(paragraph.get(), page_ptr.get(), context);

  auto paragraph2 = Paragraph::Create();
  ParagraphStyle pstyle2;
  pstyle2.SetHorizontalAlign(ParagraphHorizontalAlignment::kLeft);
  paragraph2->SetParagraphStyle(&pstyle2);
  paragraph2->AddTextRun(nullptr, content2);
  auto page_ptr2 = std::make_unique<LayoutRegion>(
      width, 500, LayoutMode::kAtMost, LayoutMode::kAtMost);
  context.Reset();
  layout.Layout(paragraph2.get(), page_ptr2.get(), context);
  auto width1 = page_ptr->GetLayoutedWidth();
  auto width2 = page_ptr2->GetLayoutedWidth();
  TTASSERT(width1 == width2);
}
void ParagraphTest::TestAlignment(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kRight);

  std::string para_content5 = R"(如果上面加上一行。
This is a line of English
下面是一段比较长的中文文本，并且会在段中间换行，用来看多行文本的对齐效果。
The following is a relatively long English text, and it will wrap in the middle of the paragraph content.
)";
  para->AddTextRun(&style, para_content5.c_str());
  DrawParagraph(canvas, *para, width);
  canvas->Translate(0, 300);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kJustify);
  DrawParagraph(canvas, *para, width);
}
void ParagraphTest::TestBaselineOffset(ICanvasHelper* canvas,
                                       float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kRight);
  para->GetParagraphStyle().SetLineHeightOverride(true);
  para->GetParagraphStyle().SetLineHeightInPercent(1.3);

  std::string para_content5 = "如果上面加上一行。";
  para->AddTextRun(&style, para_content5.c_str());
  style.SetBaselineOffset(-15);
  para->ApplyStyleInRange(style, 3, 1);
  style.SetBaselineOffset(15);
  para->ApplyStyleInRange(style, 4, 1);
  DrawParagraph(canvas, *para, width);
}
void ParagraphTest::TestPieceDraw(ICanvasHelper* canvas, float width) const {
  Style style;
  style.SetTextSize(24);
  style.SetDecorationStyle(LineType::kSolid);
  style.SetDecorationType(DecorationType::kLineThrough);
  style.SetDecorationThicknessMultiplier(1);
  style.SetDecorationColor(TTColor(TTColor::BLACK));
  style.SetBackgroundColor(TTColor(TTColor::GREEN));
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(
      nullptr,
      "根据《Global Change Biology》上的一项研究根据《Global Change "
      "Biology》上的一项研究根据《Global Change Biology》上的一项研究");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite, 0, 20);
  canvas->Translate(0, 100);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite, 20, 40);
  canvas->Translate(0, 100);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite, 40, 100);
  canvas->Translate(0, 100);
  ParagraphStyle& paragraph_style = para->GetParagraphStyle();
  paragraph_style.SetWriteDirection(WriteDirection::kRTL);
  paragraph_style.SetMaxLines(1);
  paragraph_style.SetEllipsis("...");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite, 0, 10);
}

void ParagraphTest::TestWordBoundary(ICanvasHelper* canvas, float width) const {
  {
    const char* text =
        "12345  67890 12345 67890 12345 67890 12345 "
        "67890 12345 67890 12345 67890 12345";
    const size_t len = strlen(text);
    Style style;
    auto para = Paragraph::Create();
    para->GetParagraphStyle().SetDefaultStyle(style);
    para->AddTextRun(nullptr, text);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite, 0, 20);
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(0);
      TTASSERT(word.first == 0 && word.second == 5);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(1);
      TTASSERT(word.first == 0 && word.second == 5);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(2);
      TTASSERT(word.first == 0 && word.second == 5);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(3);
      TTASSERT(word.first == 0 && word.second == 5);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(4);
      TTASSERT(word.first == 0 && word.second == 5);
    }
    {
      // expected value is (5, 7)
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(5);
      TTASSERT(word.first == 5 && word.second == 6);
    }
    {
      // expected value is (5, 7)
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(6);
      TTASSERT(word.first == 6 && word.second == 7);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(7);
      TTASSERT(word.first == 7 && word.second == 12);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(8);
      TTASSERT(word.first == 7 && word.second == 12);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(9);
      TTASSERT(word.first == 7 && word.second == 12);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(10);
      TTASSERT(word.first == 7 && word.second == 12);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(11);
      TTASSERT(word.first == 7 && word.second == 12);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(12);
      TTASSERT(word.first == 12 && word.second == 13);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(13);
      TTASSERT(word.first == 13 && word.second == 18);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(30);
      TTASSERT(word.first == 30 && word.second == 31);
    }
    {
      std::pair<uint32_t, uint32_t> word =
          para->GetWordBoundary(static_cast<uint32_t>(len) - 1);
      TTASSERT(word.first == len - 5 && word.second == len);
    }
  }
  {
    const char* text = "文字 引擎";
    Style style;
    auto para = Paragraph::Create();
    para->GetParagraphStyle().SetDefaultStyle(style);
    para->AddTextRun(nullptr, text);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite, 0, 20);
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(0);
      TTASSERT(word.first == 0 && word.second == 1);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(1);
      TTASSERT(word.first == 1 && word.second == 2);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(2);
      TTASSERT(word.first == 2 && word.second == 3);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(3);
      TTASSERT(word.first == 3 && word.second == 4);
    }
    {
      std::pair<uint32_t, uint32_t> word = para->GetWordBoundary(4);
      TTASSERT(word.first == 4 && word.second == 5);
    }
  }
}

void ParagraphTest::TestRunDelegateEllipsis(ICanvasHelper* canvas,
                                            float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(
      nullptr,
      "根据《Global Change Biology》上的一项研究根据《Global Change "
      "Biology》上的一项研究根据《Global Change Biology》上的一项研究");
  ParagraphStyle& paragraph_style = para->GetParagraphStyle();
  paragraph_style.SetMaxLines(1);
  paragraph_style.SetEllipsis("...");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
  auto shape = std::make_unique<TestShape>(96, 24);
  paragraph_style.SetEllipsis(std::move(shape));
  canvas->Translate(0, 100);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
}

void ParagraphTest::TestWhiteSpaceBreakLine(ICanvasHelper* canvas,
                                            float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(nullptr,
                   "一段文本一段文本一段文本一段文本                           "
                   "                                   ");
  para->AddTextRun(nullptr, "\n");
  para->AddTextRun(nullptr, "一段文本");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
}

void ParagraphTest::TestLayoutedWidth(ICanvasHelper* canvas,
                                      float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(nullptr,
                   "一段文本\n一段文本一段文本一段文本一段文本一段文本一段文本"
                   "一段文本一段文本一段文本一段文本一段文本");
  para->GetParagraphStyle().SetHangingIndentInPx(48);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
  para->GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kCenter);
  canvas->Translate(0, 200);
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
}

void ParagraphTest::TestCharacterVerticalAlignment(ICanvasHelper* canvas,
                                                   float width) const {
  draw_page_bound_ = false;

  {
    Style style;
    style.SetTextSize(24);
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTextTop);
    auto para = Paragraph::Create();
    auto shape = std::make_shared<TestShape>(10, 10);
    para->AddShapeRun(&style, shape, false);
    style.SetTextSize(24);
    para->AddTextRun(&style, "文本内容顶部对齐");
    style.SetTextSize(48);
    para->AddTextRun(&style, "Large Text");
    para->GetParagraphStyle().SetLineHeightInPxExact(100);
    para->GetParagraphStyle().SetLineHeightOverride(true);
    canvas->Translate(0, 50);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
  }

  {
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    style.SetVerticalAlignment(CharacterVerticalAlignment::kMiddle);
    auto shape = std::make_shared<TestShape>(10, 10);
    para->AddShapeRun(&style, shape, false);
    style.SetTextSize(24);
    para->AddTextRun(&style, "文本内容居中对齐,");
    style.SetTextSize(48);
    para->AddTextRun(&style, "Large Text");
    para->GetParagraphStyle().SetLineHeightInPxExact(100);
    para->GetParagraphStyle().SetLineHeightOverride(true);
    canvas->Translate(0, 200);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
  }

  {
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTextBottom);
    auto shape = std::make_shared<TestShape>(10, 10);
    para->AddShapeRun(&style, shape, false);
    style.SetTextSize(24);
    para->AddTextRun(&style, "文本内容底部对齐,");
    style.SetTextSize(48);
    para->AddTextRun(&style, "Large Text");
    para->GetParagraphStyle().SetLineHeightInPxExact(100);
    para->GetParagraphStyle().SetLineHeightOverride(true);
    canvas->Translate(0, 200);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
  }
}

void ParagraphTest::TestParagraphVerticalAlignment(ICanvasHelper* canvas,
                                                   float width) const {
  draw_page_bound_ = false;
  width = 1000;
  canvas->Translate(0, 10);
  auto y_step = 80;
  auto line_height = 50.f;

  auto _func = [&](ParagraphVerticalAlignment pva) {
    auto pva_text = "";
    switch (pva) {
      case ParagraphVerticalAlignment::kTop:
        pva_text = "PVA:kTop";
        break;
      case ParagraphVerticalAlignment::kCenter:
        pva_text = "PVA:kCenter";
        break;
      case ParagraphVerticalAlignment::kBottom:
        pva_text = "PVA:kBottom";
        break;
    }
    Style style;
    auto para = Paragraph::Create();
    para->GetParagraphStyle().SetVerticalAlign(pva);
    auto shape = std::make_shared<TestShape>(10, 10);
    para->AddShapeRun(&style, shape, false);
    style.SetTextSize(18);
    para->AddTextRun(&style, pva_text);
    para->AddTextRun(&style, ":");
    style.SetTextSize(24);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kBaseLine);
    para->AddTextRun(&style, "LargeText{");
    style.SetTextSize(12);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTop);
    style.SetBackgroundColor(TTColor(0x0F00FF00));
    para->AddTextRun(&style, "C:kTop");
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTextTop);
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    para->AddTextRun(&style, "C:kTextTop");
    style.SetVerticalAlignment(CharacterVerticalAlignment::kMiddle);
    style.SetBackgroundColor(TTColor(0x0F00FF00));
    para->AddTextRun(&style, "C:kMiddle");
    style.SetVerticalAlignment(CharacterVerticalAlignment::kBaseLine);
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    para->AddTextRun(&style, "C:kBaseLine");
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTextBottom);
    style.SetBackgroundColor(TTColor(0x0F00FF00));
    para->AddTextRun(&style, "C:kTextBottom");
    style.SetVerticalAlignment(CharacterVerticalAlignment::kBottom);
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    para->AddTextRun(&style, "C:kBottom");
    style.SetTextSize(24);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kBaseLine);
    style.SetBackgroundColor(TTColor(0x00FFFFFF));
    para->AddTextRun(&style, "}End");
    para->GetParagraphStyle().SetLineHeightInPxExact(line_height);
    para->GetParagraphStyle().SetLineHeightOverride(true);
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
  };

  { _func(ParagraphVerticalAlignment::kTop); }
  {
    canvas->Translate(0, y_step);
    _func(ParagraphVerticalAlignment::kCenter);
  }
  {
    canvas->Translate(0, y_step);
    _func(ParagraphVerticalAlignment::kBottom);
  }
}

void ParagraphTest::TestItalicFont(ICanvasHelper* canvas, float width) const {
  Style style;
  auto para = Paragraph::Create();
  FontDescriptor fd;
  fd.font_style_ = FontStyle::Italic();
  style.SetFontDescriptor(fd);
  style.SetTextSize(48);
  para->AddTextRun(&style, "Italic Large Text,");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
}

void ParagraphTest::TestCJKBreak(ICanvasHelper* canvas, float width) const {
  Style style;
  auto para = Paragraph::Create();
  style.SetTextSize(48);
  para->AddTextRun(&style,
                   "中文しばらくしてしばらくしてしばらくしてしばらくして");
  DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                LayoutMode::kIndefinite);
}

void ParagraphTest::TestAlignWithBBox(ICanvasHelper* canvas,
                                      float width) const {
  draw_page_bound_ = false;
  {
    canvas->Save();
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    ParagraphStyle para_style;
    para_style.EnableTextBounds(true);
    para->SetParagraphStyle(&para_style);
    FontDescriptor fd;
    fd.font_style_ = FontStyle::Normal();
    fd.font_family_list_.emplace_back("Inter");
    style.SetFontDescriptor(fd);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kMiddle);
    style.SetTextSize(24);
    para->AddTextRun(&style, "123");
    fd.font_family_list_[0] = "NotoSansCJK";
    style.SetTextSize(48);
    style.SetFontDescriptor(fd);
    para->AddTextRun(&style, "123");
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
    canvas->Restore();
  }
  {
    canvas->Save();
    canvas->Translate(300, 0);
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    ParagraphStyle para_style;
    para_style.EnableTextBounds(false);
    para->SetParagraphStyle(&para_style);
    FontDescriptor fd;
    fd.font_family_list_.emplace_back("Inter");
    fd.font_style_ = FontStyle::Normal();
    style.SetFontDescriptor(fd);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kMiddle);
    style.SetTextSize(24);
    para->AddTextRun(&style, "123");
    fd.font_family_list_[0] = "NotoSansCJK";
    style.SetTextSize(48);
    style.SetFontDescriptor(fd);
    para->AddTextRun(&style, "123");
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
    canvas->Restore();
  }
  {
    canvas->Save();
    canvas->Translate(0, 100);
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    ParagraphStyle para_style;
    para_style.EnableTextBounds(true);
    para->SetParagraphStyle(&para_style);
    FontDescriptor fd;
    fd.font_family_list_.emplace_back("Inter");
    fd.font_style_ = FontStyle::Normal();
    style.SetFontDescriptor(fd);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTop);
    style.SetTextSize(24);
    para->AddTextRun(&style, "123");
    fd.font_family_list_[0] = "NotoSansCJK";
    style.SetFontDescriptor(fd);
    style.SetTextSize(48);
    para->AddTextRun(&style, "123");
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
    canvas->Restore();
  }
  {
    canvas->Save();
    canvas->Translate(300, 100);
    Style style;
    style.SetBackgroundColor(TTColor(0x0F0000FF));
    auto para = Paragraph::Create();
    ParagraphStyle para_style;
    para_style.EnableTextBounds(false);
    para->SetParagraphStyle(&para_style);
    FontDescriptor fd;
    fd.font_family_list_.emplace_back("Inter");
    fd.font_style_ = FontStyle::Normal();
    style.SetFontDescriptor(fd);
    style.SetVerticalAlignment(CharacterVerticalAlignment::kTop);
    style.SetTextSize(24);
    para->AddTextRun(&style, "123");
    fd.font_family_list_[0] = "NotoSansCJK";
    style.SetFontDescriptor(fd);
    style.SetTextSize(48);
    para->AddTextRun(&style, "123");
    DrawParagraph(canvas, *para, width, LayoutMode::kAtMost,
                  LayoutMode::kIndefinite);
    canvas->Restore();
  }
}
void ParagraphTest::TestModifyHAlignAfterLayout(ICanvasHelper* canvas,
                                                float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(nullptr, "一段文本");
  Style y_offset = style;
  y_offset.SetBaselineOffset(10);
  para->AddTextRun(&y_offset, "一段文本");
  para->AddTextRun(nullptr,
                   "一段文本一段文本一段文本一段文本一段文本一段文本一段文本"
                   "一段文本一段文本一段文本一段文本一段文本");
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite,
                LayoutMode::kIndefinite);
  canvas->Translate(0, 200);
  TextLayout layout(font_collection_, shaper_type_);
  auto page_ptr = std::make_unique<LayoutRegion>(
      width, 500, LayoutMode::kDefinite, LayoutMode::kAtMost);
  TTTextContext context;
  context.SetSkipSpacingBeforeFirstLine(false);
  context.SetLastLineCanOverflow(false);
  auto& page = *page_ptr;
  layout.Layout(para.get(), &page, context);
  page.GetLine(0)->ModifyHorizontalAlignment(
      ParagraphHorizontalAlignment::kJustify);
  page.GetLine(1)->ModifyHorizontalAlignment(
      ParagraphHorizontalAlignment::kCenter);
  page.GetLine(2)->ModifyHorizontalAlignment(
      ParagraphHorizontalAlignment::kRight);
  LayoutDrawer drawer(canvas);
  drawer.DrawLayoutPage(&page);
}

void ParagraphTest::TestApplyStyleInRange(ICanvasHelper* canvas,
                                          float width) const {
  Style style;
  style.SetTextSize(24);
  auto para = Paragraph::Create();
  para->GetParagraphStyle().SetDefaultStyle(style);
  para->AddTextRun(nullptr, "一段文本");
  Style y_offset = style;
  y_offset.SetBaselineOffset(10);
  para->AddTextRun(&y_offset, "一段文本");
  para->AddTextRun(nullptr,
                   "一段文本一段文本一段文本一段文本一段文本一段文本一段文本"
                   "一段文本一段文本一段文本一段文本一段文本");
  Style style1;
  style1.SetForegroundColor(TTColor(0xffff0000));
  para->ApplyStyleInRange(style1, 10, 20);
  DrawParagraph(canvas, *para, width, LayoutMode::kDefinite,
                LayoutMode::kIndefinite);
}

#pragma clang diagnostic pop
