// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef TEST_TEST_UTILS_H
#define TEST_TEST_UTILS_H

#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>
#include <textra/text_line.h>
#include <textra/tt_color.h>

#include "demos/darwin/macos/glfw/skity_adaptor.h"
#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "skity/codec/codec.hpp"
#include "src/ports/shaper/skshaper/shaper_skshaper.h"
#include "src/textlayout/paragraph_impl.h"
#include "third_party/skity/include/skity/graphic/alpha_type.hpp"
#include "third_party/skity/include/skity/graphic/bitmap.hpp"
#include "third_party/skity/include/skity/graphic/color.hpp"
#include "third_party/skity/include/skity/graphic/color_type.hpp"
#include "third_party/skity/include/skity/render/canvas.hpp"

using namespace ttoffice::tttext;
#define CANVAS_WIDTH 500
#define CANVAS_HEIGHT 1000

class TestShapingResultReader final
    : public tttext::PlatformShapingResultReader {
 public:
  TestShapingResultReader(uint32_t glyph_count) {
    glyphs_.resize(glyph_count);
    advances_.resize(glyph_count);
    positions_.resize(glyph_count);
  }
  uint32_t GlyphCount() const override { return glyphs_.size(); }
  uint32_t TextCount() const override { return glyphs_.size(); }
  GlyphID ReadGlyphID(uint32_t idx) const override { return glyphs_.at(idx); }
  float ReadAdvanceX(uint32_t idx) const override {
    return advances_.at(idx)[0];
  }
  float ReadAdvanceY(uint32_t idx) const override {
    return advances_.at(idx)[1];
  }
  float ReadPositionX(uint32_t idx) const override {
    return positions_.at(idx)[0];
  }
  float ReadPositionY(uint32_t idx) const override {
    return positions_.at(idx)[1];
  }
  uint32_t ReadIndices(uint32_t idx) const override { return idx; }
  TypefaceRef ReadFontId(uint32_t idx) const override { return font_; }
  std::vector<std::array<float, 2>> advances_;
  std::vector<std::array<float, 2>> positions_;
  std::vector<GlyphID> glyphs_;
  TypefaceRef font_;
};

class TestShaper : public TTShaper {
 public:
  explicit TestShaper(FontmgrCollection font_collection) noexcept
      : TTShaper(std::move(font_collection)) {}

  void OnShapeText(const ShapeKey& key, ShapeResult* result) const override {
    const auto& text = key.text_;
    const auto& style = key.style_;
    size_t length = text.length();
    TestShapingResultReader reader(length);
    for (size_t i = 0; i < length; i++) {
      reader.glyphs_[i] = 0xFF;  // FIXME: hard-coded, glyph id zero usually
                                 // represents invalid glyph
      reader.advances_[i] = {1 * style.GetFontSize(),
                             0};  // FIXME: hard-coded [x, y]
    }
    reader.font_ = font_collection_.defaultFallback();
    result->AppendPlatformShapingResult(reader);
  }
};

class TestUtils {
 private:
  TestUtils() = default;
  ~TestUtils() = default;
  TestUtils(const TestUtils&);
  TestUtils& operator=(const TestUtils&);

 public:
  static TestUtils& getInstance() {
    static TestUtils instance;
    return instance;
  }
  // Get a dummy shaper that returns hard-coded values, used for unit test that
  // doesn't depend on shaper results.
  static std::unique_ptr<TTShaper> getTestShaper() {
    return std::make_unique<TestShaper>(getFontmgrCollection());
  }
  // Get a working shaper, usually used in image comparison tests.
  static std::unique_ptr<ShaperSkShaper> getRealShaper() {
    return std::make_unique<ShaperSkShaper>(getFontmgrCollection());
  }
  static FontmgrCollection& getFontmgrCollection() {
    static FontmgrCollection font_collection = []() {
      auto font_manager = std::make_shared<SkityTestFontManager>();
      return FontmgrCollection(font_manager);
    }();
    return font_collection;
  }
  static void DrawLayoutPage(LayoutRegion* page, std::string out_file_name) {
    skity::Bitmap bitmap(page->GetPageWidth(), page->GetPageHeight(),
                         skity::AlphaType::kPremul_AlphaType,
                         skity::ColorType::kRGBA);
    std::unique_ptr<skity::Canvas> canvas =
        skity::Canvas::MakeSoftwareCanvas(&bitmap);
    canvas->DrawColor(skity::Color_WHITE);
    SkityCanvasHelper canvas_helper(canvas.get());
    DrawLayoutRegionOnCanvas(canvas_helper, *page);
    std::shared_ptr<skity::Codec> codec = skity::Codec::MakePngCodec();
    // pixmap_swizzle_rb(pixmap.get());
    std::shared_ptr<skity::Data> data = codec->Encode(bitmap.GetPixmap().get());
    data->WriteToFile(out_file_name.c_str());
  }

  static void DrawLayoutRegionOnCanvas(ICanvasHelper& canvas_helper,
                                       LayoutRegion& region) {
    LayoutDrawer drawer(&canvas_helper);
    drawer.DrawLayoutPage(&region);
  }

  static void DrawLineRectOnCanvas(ICanvasHelper& canvas_helper,
                                   LayoutRegion& region) {
    for (uint32_t i = 0; i < region.GetLineCount(); i++) {
      TextLine* line = region.GetLine(i);
      float bounding_rect[4];
      line->GetBoundingRectForLine(bounding_rect);
      auto painter = canvas_helper.CreatePainter();
      painter->SetColor(TTColor::GREEN());
      painter->SetFillStyle(FillStyle::kStroke);
      painter->SetStrokeWidth(1);
      canvas_helper.DrawRect(bounding_rect[0], bounding_rect[1],
                             bounding_rect[0] + bounding_rect[2],
                             bounding_rect[1] + bounding_rect[3],
                             painter.get());
    }
  }

  static std::unique_ptr<ParagraphImpl> GenerateParagraph(
      std::string& text, ParagraphHorizontalAlignment align) {
    auto paragraph = std::make_unique<ParagraphImpl>();
    //    paragraph->GetParagraphStyle().SetHorizontalAlign(align);
    Style r_pr;
    r_pr.SetTextSize(24);
    paragraph->GetParagraphStyle().SetDefaultStyle(r_pr);
    paragraph->AddTextRun(&r_pr, text.c_str());
    return paragraph;
  }
  static std::unique_ptr<ParagraphImpl> GenerateDefaultParagraphTestCase() {
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

    auto para = GenerateParagraph(default_text_str,
                                  ParagraphHorizontalAlignment::kLeft);
    return para;
  }
  static std::unique_ptr<LayoutRegion> SimpleLayoutParagraphByWidth(
      Paragraph* paragraph, float width,
      LayoutMode width_mode = LayoutMode::kDefinite) {
    TextLayout layout(getTestShaper());
    auto region = std::make_unique<LayoutRegion>(
        width, std::numeric_limits<float>::max(), width_mode);
    TTTextContext context;
    layout.LayoutEx(paragraph, region.get(), context);
    return region;
  }
};
#endif  // TEST_TEST_UTILS_H
