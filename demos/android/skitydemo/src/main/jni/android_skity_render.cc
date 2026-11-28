#include "android_skity_render.hpp"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/log.h>
#include <jni.h>

#include <cstdlib>

static const char* kTAG = "SkityGL";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

void Renderer::init(int w, int h, int d) {
  init_gl();

  skity_context context = nullptr;
  if (skity_context_create_gl(
          [](const char* name) -> void* {
            return reinterpret_cast<void*>(eglGetProcAddress(name));
          },
          &context) != SKITY_SUCCESS)
    std::abort();
  ctx_ = ttoffice::tttext::skity_c::Take(context, skity_context_destroy);

  skity_surface_create_info_gl gl{};
  gl.s_type = SKITY_STRUCTURE_TYPE_SURFACE_CREATE_INFO_GL;
  gl.surface_type = SKITY_GL_SURFACE_TYPE_FRAMEBUFFER;
  skity_surface_create_info desc{};
  desc.s_type = SKITY_STRUCTURE_TYPE_SURFACE_CREATE_INFO;
  desc.p_next = &gl;
  desc.width = w;
  desc.height = h;
  desc.content_scale = 1.f;
  desc.sample_count = 4;

  gl.gl_id = 0;
  gl.has_stencil = false;

  skity_surface surface = nullptr;
  if (skity_surface_create(ctx_.get(), &desc, &surface) != SKITY_SUCCESS)
    std::abort();
  m_surface = ttoffice::tttext::skity_c::Take(surface, skity_surface_destroy);

  m_width = w;
  m_height = h;
}

void Renderer::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_canvas = skity_surface_lock_canvas(m_surface.get(), 0);

  skity_canvas_draw_color4f(m_canvas, m_clear_color, SKITY_BLEND_MODE_SRC);

  onDraw(m_canvas);

  skity_canvas_flush(m_canvas);

  skity_surface_flush(m_surface.get());

  m_canvas = nullptr;
}

void Renderer::init_gl() {
  glClearColor(m_clear_color.x, m_clear_color.y, m_clear_color.z,
               m_clear_color.w);
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
