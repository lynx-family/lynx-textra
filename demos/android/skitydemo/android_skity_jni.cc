#include <android/log.h>
#include <jni.h>

#define REGISTER_NATIVES(class_name)                             \
  int register_skity_jni_##class_name(JNIEnv*);                  \
  if (auto rc = register_skity_jni_##class_name(env)) {          \
    __android_log_print(ANDROID_LOG_ERROR, "SkityJNI",           \
                        "Failed to load natives: " #class_name); \
    return rc;                                                   \
  }

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  REGISTER_NATIVES(Renderer)
  REGISTER_NATIVES(AndroidGLExample)
  return JNI_VERSION_1_6;
}
