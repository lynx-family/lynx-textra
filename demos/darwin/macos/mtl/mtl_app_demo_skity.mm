// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/fontmgr_collection.h>
#include <textra/platform/skity/skity_canvas_helper.h>

#include "demos/darwin/macos/ttreaderdemo/paragraph_test.h"
#include "mtl_app.h"
#include "skity_adaptor.h"
using namespace tttext;

class MTLExample : public example::MTLApp {
 public:
  MTLExample()
      : example::MTLApp(800, 600, "MTL Shape Example", {1.f, 1.f, 1.f, 1.f}),
        font_collection_(nullptr) {
    auto font_mgr = std::make_shared<SkityTestFontManager>();
    font_collection_.SetDefaultFontManager(font_mgr);
  }

  ~MTLExample() override = default;

 protected:
  void OnUpdate(float) override {
    GetCanvas()->Save();
    GetCanvas()->DrawColor(0xFFFFFF00);
    SkityCanvasHelper canvas(GetCanvas());
    canvas.DrawColor(0xFFFFFFFF);
    skity::Paint paint;
    paint.SetFillColor(1.0, 0, 0, 1.0f);
    paint.SetStyle(skity::Paint::kFill_Style);
    ParagraphTest paragraph_test_(kSelfRendering, &font_collection_);
    paragraph_test_.TestWithId(&canvas, width_, test_id);
    GetCanvas()->Restore();
    GetCanvas()->Flush();
  }

  void OnHandleKey(int key, int scancode, int action, int mods) override {
    switch (key) {
      case GLFW_KEY_RIGHT:
        test_id++;
        break;
      case GLFW_KEY_LEFT:
        test_id--;
        break;
      case GLFW_KEY_RIGHT_BRACKET:
        width_ += mods == GLFW_MOD_SHIFT ? 50 : 10;
        break;
      case GLFW_KEY_LEFT_BRACKET:
        width_ -= mods == GLFW_MOD_SHIFT ? 50 : 10;
        break;
      default:
        example::MTLApp::OnHandleKey(key, scancode, action, mods);
    }
    printf("test_id: %d\n", test_id);
  }

 private:
  uint32_t test_id = 42;
  float width_ = 500;
  FontmgrCollection font_collection_;
};

int main(int, const char**) {
  MTLExample app;
  app.Run();
  return 0;
}

// void draw_canvas(skity::Canvas* canvas);
//
// class Textlayoutdemo : public example::GLApp {
//  public:
//   Textlayoutdemo()
//       : example::GLApp(1000, 800, "GL Example", {1.f, 1.f, 1.f, 1.f}) {}
//
//   ~Textlayoutdemo() override = default;
//
//  private:
//   const char* para_text =
//       "(Q1)I have a test.so with some functions that I need to "
//       "use.I've surveyed for a while but no answers. Could "
//       "anyone have some advise of how to include a shared "
//       "library in gn file of chromium project? Many thanks. "
//       "春穿漕渠通渭。匈奴入上谷，殺略吏民。遣騎將軍公孫敖出代，輕車將軍公孫賀"
//       "出雲中，驍騎將軍李廣出鴈門，車騎將軍衞靑出上谷。衞青者，衞夫人子夫之弟"
//       "也。父鄭季，河東平陽人也。初，季與主家僮衞媪私通，生竒青，冐姓爲衞氏。"
//       "靑長姊君孺，即公孫賀妻也。嘗有相者曰：貴人也，當封侯。靑曰：人婢之生，"
//       "得無笞罵足矣，安得封侯乎？及子夫自平陽公主家僮得幸於上，立爲夫人。陳皇"
//       "后之大長公主捕囚靑，欲殺之。公孫敖爲騎，郞與壯士募靑。上聞，乃召靑爲建"
//       "章監侍中。子夫女弟貴，故與陳掌通。上乃召貴、掌及公孫敖、衞靑之寵始隆矣"
//       "。🎉";
//
//  protected:
//   void OnUpdate(float time) override {
//     SkityCanvasHelper canvas(GetCanvas());
//     canvas.DrawColor(0xFFFFFFFF);
//     skity::Paint paint;
//     paint.SetFillColor(1.0, 0, 0, 1.0f);
//     paint.SetStyle(skity::Paint::kFill_Style);
//     GetCanvas()->DrawRect(skity::Rect::MakeLTRB(200, 200, 300, 300), paint);
//     //    TestTTParagraph(&canvas, "中", 200);
//
//     // TestTTParagraph(&canvas, para_text, 200);
//     paragraph_test_.TestWithId(&canvas, width_, test_id);
//     // draw_paragraph(GetCanvas(), 200, 20, 300, 300);
//     GetCanvas()->Flush();
//   }
//
//   void OnStart() override {}
//
//  private:
//   uint32_t test_id = 21;
//   float width_ = 500;
//   ParagraphTest paragraph_test_;
// };
//
// std::unique_ptr<TTPainter> ParagraphTest::CreatePainter() {
//   return std::make_unique<SkityPainter>();
// }
//
// class SkityShaperHolder {
//  public:
//   SkityShaperHolder() {
//     auto tf_pingfang = TF_PINGFANG;
//     auto font_mgr = std::make_shared<TestFontMgr>(
// .     std::vector<std::shared_ptr<ITypefaceHelper>>{TFH_DEFAULT});
//     IFontCollection fc;
//     fc.SetDefaultFontManager(font_mgr);
//     shaper_ = std::make_unique<ShaperSkShaper>(fc);
//   }
//   ShaperSkShaper *GetShaper() {
//     return shaper_.get();
//   }
// private:
//   std::unique_ptr<ShaperSkShaper> shaper_;
// };
//
//
// TTShaper* ParagraphTest::GetShaper(){
//   static SkityShaperHolder holder;
//   return holder.GetShaper();
// }
//
// int main(int argc, const char** argv) {
//   Textlayoutdemo app;
//
//   app.Run();
//   return 0;
// }
