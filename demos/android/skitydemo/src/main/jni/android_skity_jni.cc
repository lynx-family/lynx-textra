#include <android/log.h>
#include <jni.h>

#include <skity/skity.hpp>

#define REGISTER_NATIVES(class_name)                             \
  int register_skity_jni_##class_name(JNIEnv*);                  \
  if (auto rc = register_skity_jni_##class_name(env)) {          \
    __android_log_print(ANDROID_LOG_ERROR, "SkityJNI",           \
                        "Failed to load natives: " #class_name); \
    return rc;                                                   \
  }

void skity_log_i(const char* msg) {
  __android_log_print(ANDROID_LOG_INFO, "Skity", "%s", msg);
}

void skity_log_d(const char* msg) {
  __android_log_print(ANDROID_LOG_DEBUG, "Skity", "%s", msg);
}

void skity_log_e(const char* msg) {
  __android_log_print(ANDROID_LOG_ERROR, "Skity", "%s", msg);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  REGISTER_NATIVES(Renderer)
  REGISTER_NATIVES(AndroidGLExample)
  // REGISTER_NATIVES(AndroidGLFrameExample)

  skity::Logger::CustomLogger logger{};
  logger.log_i = &skity_log_i;
  logger.log_d = &skity_log_d;
  logger.log_e = &skity_log_e;

  skity::Logger::RegisteLog(&logger);

  return JNI_VERSION_1_6;
}
