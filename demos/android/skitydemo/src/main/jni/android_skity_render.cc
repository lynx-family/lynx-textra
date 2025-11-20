#include "android_skity_render.hpp"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/log.h>
#include <jni.h>

#include <skity/gpu/gpu_context_gl.hpp>

static const char* kTAG = "SkityGL";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

void Renderer::init(int w, int h, int d) {
  init_gl();

  ctx_ = skity::GLContextCreate((void*)eglGetProcAddress);

  skity::GPUSurfaceDescriptorGL desc{};
  desc.backend = skity::GPUBackendType::kOpenGL;
  desc.width = w;
  desc.height = h;
  desc.content_scale = 1.f;
  desc.sample_count = 4;

  desc.surface_type = skity::GLSurfaceType::kFramebuffer;
  desc.gl_id = 0;
  desc.has_stencil_attachment = false;

  m_surface = ctx_->CreateSurface(&desc);

  m_width = w;
  m_height = h;
}

void Renderer::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_canvas = m_surface->LockCanvas();

  m_canvas->DrawColor(m_clear_color, skity::BlendMode::kSrc);

  onDraw(m_canvas);

  m_canvas->Flush();

  m_surface->Flush();

  m_canvas = nullptr;
}

void Renderer::init_gl() {
  glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b,
               m_clear_color.a);
  glClearStencil(0x0);
  glStencilMask(0xFF);
  glEnable(GL_STENCIL_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_SCISSOR_TEST);

  const char* version = (const char*)glGetString(GL_VERSION);

  LOGI("glVersion = %s", version);
}

void Renderer_Draw(JNIEnv* env, jobject, jlong handler) {
  auto render = (Renderer*)handler;

  render->draw();
}

void Renderer_Destroy(JNIEnv* env, jobject thiz, jlong handler) {
  auto render = (Renderer*)handler;

  delete render;
}

int register_skity_jni_Renderer(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {"nativeDraw", "(J)V", reinterpret_cast<void*>(Renderer_Draw)},
      {"nativeDestroy", "(J)V", reinterpret_cast<void*>(Renderer_Destroy)}};

  const auto clazz = env->FindClass(SKITY_JNI_PKG "/SkityRender");
  return clazz ? env->RegisterNatives(clazz, methods, 2) : JNI_ERR;
}
