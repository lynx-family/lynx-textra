#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/platform/skity/skity_font_manager.h>
#include <textra/platform/skity/skity_typeface_helper.h>
#include <textra/text_layout.h>

#include "android_skity_render.hpp"

using namespace tttext;
class AndroidGLExample : public Renderer {
 public:
  AndroidGLExample() {
    font_manager_ = std::make_shared<SkityFontManager>();
    collection_.SetDefaultFontManager(font_manager_);
    text_layout_ = std::make_unique<TextLayout>(&collection_, kSelfRendering);
    paragraph_ = Paragraph::Create();
    Style style;
    style.SetFontDescriptor({{""}, FontStyle::Normal(), 0});
    style.SetForegroundColor(TTColor::BLACK);
    style.SetTextSize(60);
    paragraph_->AddTextRun(&style, "测试字重, test font weight, 😄🇨🇳");
    auto region = std::make_unique<LayoutRegion>(1000, 1000);
    text_layout_->LayoutEx(paragraph_.get(), region.get(), context_);
    region_ = std::move(region);
  };
  ~AndroidGLExample() override = default;

 protected:
  void onDraw(skity::Canvas* canvas) override {
    SkityCanvasHelper canvas_helper(canvas);
    if (region_) {
      LayoutDrawer drawer(&canvas_helper);
      drawer.DrawLayoutPage(region_.get());
    }
  }

 private:
  TTTextContext context_;
  std::unique_ptr<TextLayout> text_layout_;
  FontManagerRef font_manager_;
  FontmgrCollection collection_;
  std::unique_ptr<Paragraph> paragraph_;
  std::unique_ptr<LayoutRegion> region_;
};

jlong AndroidGLExample_nativeCreate(JNIEnv* env, jobject thiz, jint width,
                                    jint height, jint density,
                                    jobject asset_manager) {
  auto render = new AndroidGLExample();

  render->init(width, height, density);

  return (jlong)render;
}

int register_skity_jni_AndroidGLExample(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {"nativeCreateGLRender", "(IIILandroid/content/res/AssetManager;)J",
       reinterpret_cast<void*>(AndroidGLExample_nativeCreate)},
  };

  const auto clazz = env->FindClass(SKITY_JNI_PKG "/SkityGLRender");
  return clazz ? env->RegisterNatives(clazz, methods, 1) : JNI_ERR;
}
