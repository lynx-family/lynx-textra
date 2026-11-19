// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#include <atomic>
#include <string>

#include "src/ports/native/java/java_canvas_helper.h"
#include "src/ports/native/java/java_font_manager.h"
#include "src/ports/native/java/tttext_jni_proxy.h"
#include "src/ports/native/native_helpers.h"
#include "src/ports/shaper/java/java_utils.h"

namespace ttoffice {
namespace tttext {
namespace native {
namespace {
// Only target binding and replay are added. Every drawing operation uses the
// original C++ command writer and original Java decoder unchanged.
class NativeJavaCanvas final : public JavaCanvasHelper {
 public:
  NativeJavaCanvas(JNIEnv* env, jobject target, jobject decoder,
                   jmethodID replay, std::shared_ptr<JavaFontManager> manager)
      : target_(env, target),
        decoder_(env, decoder),
        replay_(replay),
        manager_(std::move(manager)) {}
  void StartPaint() override { ClearBuffer(); }
  void EndPaint() override {
    auto* env = TTTextJNIProxy::GetCurrentJNIEnv();
    if (!env) return;
    auto bytes = CopyBufferToJava(env, nullptr);
    if (bytes && !env->ExceptionCheck())
      env->CallVoidMethod(decoder_.get(), replay_, target_.get(), bytes);
    if (bytes) env->DeleteLocalRef(bytes);
    ClearException(env);
  }

 private:
  ScopedGlobalRef target_, decoder_;
  jmethodID replay_;
  std::shared_ptr<JavaFontManager> manager_;
};

// Resolve through the original manager's class loader, including attached
// native threads where FindClass cannot see application classes directly.
jclass CanvasClass(JNIEnv* env, jobject manager) {
  auto object_class = env->GetObjectClass(manager);
  auto class_class = env->FindClass("java/lang/Class");
  if (!object_class || !class_class) {
    if (object_class) env->DeleteLocalRef(object_class);
    if (class_class) env->DeleteLocalRef(class_class);
    ClearException(env);
    return nullptr;
  }
  auto get_loader = env->GetMethodID(class_class, "getClassLoader",
                                     "()Ljava/lang/ClassLoader;");
  auto loader =
      get_loader ? env->CallObjectMethod(object_class, get_loader) : nullptr;
  env->DeleteLocalRef(object_class);
  env->DeleteLocalRef(class_class);
  bool failed = ClearException(env);
  if (!loader || failed) {
    if (loader) env->DeleteLocalRef(loader);
    return nullptr;
  }
  auto loader_class = env->GetObjectClass(loader);
  auto load = env->GetMethodID(loader_class, "loadClass",
                               "(Ljava/lang/String;)Ljava/lang/Class;");
  auto name = env->NewStringUTF("com.lynx.textra.JavaCanvasHelper");
  auto result =
      load && name
          ? static_cast<jclass>(env->CallObjectMethod(loader, load, name))
          : nullptr;
  if (name) env->DeleteLocalRef(name);
  env->DeleteLocalRef(loader_class);
  env->DeleteLocalRef(loader);
  if (ClearException(env)) {
    if (result) env->DeleteLocalRef(result);
    return nullptr;
  }
  return result;
}
}  // namespace

FontManagerRef AndroidFontManager() {
  return TTTextJNIProxy::GetInstance().GetDefaultFontManager();
}
std::unique_ptr<ICanvasHelper> AndroidCanvas(void* target,
                                             const FontManagerRef& fonts) {
  auto manager = std::static_pointer_cast<JavaFontManager>(
      fonts ? fonts : AndroidFontManager());
  auto* env = TTTextJNIProxy::GetCurrentJNIEnv();
  if (!manager || !env || !target) return nullptr;
  auto cls = CanvasClass(env, manager->GetInstance());
  if (!cls) return nullptr;
  auto init =
      env->GetMethodID(cls, "<init>", "(Lcom/lynx/textra/JavaFontManager;)V");
  auto replay =
      env->GetMethodID(cls, "drawBuffer", "(Landroid/graphics/Canvas;[B)V");
  auto decoder = init && replay && !env->ExceptionCheck()
                     ? env->NewObject(cls, init, manager->GetInstance())
                     : nullptr;
  env->DeleteLocalRef(cls);
  if (ClearException(env) || !decoder) {
    if (decoder) env->DeleteLocalRef(decoder);
    return nullptr;
  }
  auto result = std::make_unique<NativeJavaCanvas>(
      env, static_cast<jobject>(target), decoder, replay, manager);
  env->DeleteLocalRef(decoder);
  if (ClearException(env)) return nullptr;
  return result;
}
TypefaceRef AndroidTypeface(const void* font, const FontManagerRef& fonts) {
  auto manager = std::static_pointer_cast<JavaFontManager>(fonts);
  auto* env = TTTextJNIProxy::GetCurrentJNIEnv();
  if (!env || !manager || !font) return nullptr;
  auto cls = env->GetObjectClass(manager->GetInstance());
  auto register_font =
      env->GetMethodID(cls, "CreateOrRegisterTypeface",
                       "(Landroid/graphics/Typeface;Ljava/lang/String;IZ)Lcom/"
                       "lynx/textra/JavaTypeface;");
  static std::atomic<uint64_t> next_font{0};
  auto family =
      std::string("__textra_native_") + std::to_string(next_font.fetch_add(1));
  auto name = env->NewStringUTF(family.c_str());
  auto instance = register_font && name && !env->ExceptionCheck()
                      ? env->CallObjectMethod(
                            manager->GetInstance(), register_font,
                            static_cast<jobject>(const_cast<void*>(font)), name,
                            FontStyle::Normal().GetWeight(), JNI_FALSE)
                      : nullptr;
  if (name) env->DeleteLocalRef(name);
  env->DeleteLocalRef(cls);
  if (ClearException(env) || !instance) {
    if (instance) env->DeleteLocalRef(instance);
    return nullptr;
  }
  auto face_class = env->GetObjectClass(instance);
  auto index = env->GetFieldID(face_class, "mIndex", "I");
  auto value = index ? env->GetIntField(instance, index) : 0;
  env->DeleteLocalRef(face_class);
  env->DeleteLocalRef(instance);
  if (ClearException(env) || !value) return nullptr;
  FontDescriptor descriptor;
  descriptor.platform_font_ = static_cast<uint64_t>(value);
  return manager->matchTypeface(descriptor);
}
}  // namespace native
}  // namespace tttext
}  // namespace ttoffice
