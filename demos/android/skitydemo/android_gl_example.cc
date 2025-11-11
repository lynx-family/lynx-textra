#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "android_skity_render.hpp"

void draw_canvas(skity::Canvas* canvas);

class AndroidGLExample : public Renderer {
 public:
  AndroidGLExample() = default;
  ~AndroidGLExample() override = default;

 protected:
  void onDraw(skity::Canvas* canvas) override { draw_canvas(canvas); }
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
