// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "test_src.h"

#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include "test_utils.h"

namespace ttoffice {
namespace tttext {
void ParagraphStyleTestClass::InitiateDefaultParagraph(TTParagraph& paragraph) {
  std::string default_text_str =
      "ld: warning: direct access in function "
      "'GrResourceCache::getNextTimestamp()' from file "
      "'/Users/user/project_android/textlayout/libs/x86_64/"
      "libskia.a(gpu.GrResourceCache.o)' to global weak symbol "
      "'GrResourceCache::CompareTimestamp(GrGpuResource* const&, "
      "GrGpuResource* "
      "const&)' from file "
      "'CMakeFiles/sk_app_demo.dir/__/__/include/skia/tools/sk_app/"
      "GLWindowContext.cpp.o' means the weak symbol cannot be overridden at "
      "runtime. This was likely caused by different translation units being "
      "compiled with different visibility settings."
      "沙漠，主要是指地面完全被沙子覆盖，并且动植物非常稀少，雨水稀少，空气很"
      "干燥的地区。沙漠的地表覆盖的是一层很厚的细沙状沙子，跟沙滩上沙子不同的"
      "是，一个是风的长期作用，而另一个则是水的长期作用。沙漠的地表会随着风自"
      "己变化和移动，沙丘会向前层层推进，最终变化成不同的形态。";
  Style r_pr;
  r_pr.SetTextSize(24);
  paragraph.GetParagraphStyle().SetDefaultStyle(r_pr);
  paragraph.AddTextRun(&r_pr, default_text_str.c_str());
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::LineGapTest(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetLineSpaceAfterPx(10);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::LineHeightTest1(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetLineHeightInPxExact(30);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::LineHeightTest2(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetLineHeightInPercent(3.0);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::VerticalAlignmentTest1(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetVerticalAlign(
      ParagraphVerticalAlignment::kTop);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::VerticalAlignmentTest2(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetVerticalAlign(
      ParagraphVerticalAlignment::kBottom);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::VerticalAlignmentTest3(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetVerticalAlign(
      ParagraphVerticalAlignment::kCenter);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::VerticalAlignmentTest4(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetVerticalAlign(
      ParagraphVerticalAlignment::kCenter);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::HorizontalAlignmentTest1(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kCenter);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::HorizontalAlignmentTest2(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kDistributed);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::HorizontalAlignmentTest3(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kJustify);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::HorizontalAlignmentTest4(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kLeft);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> ParagraphStyleTestClass::HorizontalAlignmentTest5(
    TTParagraph& paragraph, float width, float height) {
  InitiateDefaultParagraph(paragraph);
  paragraph.GetParagraphStyle().SetHorizontalAlign(
      ParagraphHorizontalAlignment::kRight);
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}

std::unique_ptr<LayoutRegion> TextTestClass::FontStyleTest(
    TTParagraph& paragraph, float width, float height) {
  Style default_rpr;
  default_rpr.SetTextSize(24);
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
  // TODO: Italic not working
  Style font_italic_rpr;
  font_italic_rpr.SetItalic(true);
  paragraph.AddTextRun(&font_italic_rpr, "斜体字\n");
  Style decorator_rpr;
  decorator_rpr.SetDecorationType(DecorationType::kUnderLine);
  decorator_rpr.SetDecorationColor(TTColor(0xFF00FF00));
  paragraph.AddTextRun(&decorator_rpr, "下划线\n");
  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> TextTestClass::WordBreakTypeTest1(
    TTParagraph& paragraph, float width, float height) {
  Style word_break_all_rpr;
  word_break_all_rpr.SetTextSize(24);
  word_break_all_rpr.SetWordBreak(ttoffice::tttext::WordBreakType::kBreakAll);
  paragraph.AddTextRun(
      &word_break_all_rpr,
      "This is a long and "
      " Honorificabilitudinitatibus califragilisticexpialidocious "
      "Taumatawhakatangihangakoauauot amateaturipukakapikimaun "
      "gahoronukupokaiwhe "
      "nuakitanatahu"
      "\n");

  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> TextTestClass::WordBreakTypeTest2(
    TTParagraph& paragraph, float width, float height) {
  Style word_break_all_rpr;
  word_break_all_rpr.SetTextSize(24);
  word_break_all_rpr.SetWordBreak(ttoffice::tttext::WordBreakType::kBreakAll);
  paragraph.AddTextRun(
      &word_break_all_rpr,
      "This is a long and "
      " Honorificabilitudinitatibus califragilisticexpialidocious "
      "Taumatawhakatangihangakoauauot amateaturipukakapikimaun "
      "gahoronukupokaiwhe "
      "nuakitanatahu"
      "\n");

  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
std::unique_ptr<LayoutRegion> TextTestClass::SubSupTest(TTParagraph& paragraph,
                                                        float width,
                                                        float height) {
  Style run_style;
  run_style.SetTextSize(24);
  paragraph.AddTextRun(&run_style, "中国中国中国中国");
  run_style.SetTextScale(0.58);
  run_style.SetVerticalAlignment(
      ttoffice::tttext::CharacterVerticalAlignment::kSuperScript);
  paragraph.AddTextRun(&run_style, "你好");
  run_style.SetVerticalAlignment(
      ttoffice::tttext::CharacterVerticalAlignment::kSubScript);
  paragraph.AddTextRun(&run_style, "你好");
  run_style.SetTextScale(1.0f);
  run_style.SetVerticalAlignment(
      ttoffice::tttext::CharacterVerticalAlignment::kBaseLine);
  paragraph.AddTextRun(&run_style, "正常正常正常正常正常正常正常正常正常");

  TTTextContext context;
  auto layout_page = std::make_unique<LayoutRegion>(width, height);
  TextLayout layout(TestUtils::getRealShaper());
  layout.Layout(&paragraph, layout_page.get(), context);
  return layout_page;
}
// std::unique_ptr<LayoutRegion> ShapeTestClass::SmallShapeTest(
//     TTParagraph& paragraph, float width, float height,
//     std::unique_ptr<RunDelegate> shape) {
//   //   Style default_rpr;
//   default_rpr.SetTextSize(24);
//   paragraph.GetParagraphStyle().SetDefaultStyle(default_rpr);
//   paragraph.AddTextRun("一二三");
//   paragraph.AddShapeRun({}, std::move(shape), true);
//   paragraph.AddTextRun("四五六");
//   TextLayout layout(TestUtils::getRealShaper());
//   auto layout_page = std::make_unique<LayoutRegion>(width, height);
//   layout.Layout(&paragraph, layout_page.get());
//   return layout_page;
// }
}  // namespace tttext
}  // namespace ttoffice
