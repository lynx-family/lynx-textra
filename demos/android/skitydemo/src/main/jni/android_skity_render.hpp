#ifndef DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_
#define DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_

#include <utility>

#include "demos/capi/skity_capi_utils.hpp"

class Renderer {
 public:
  Renderer() = default;

  Renderer(skity_color4f color) : m_clear_color(std::move(color)) {}

  virtual ~Renderer() = default;

  void init(int w, int h, int d);

  void draw();

 protected:
  virtual void onDraw(skity_canvas canvas) = 0;

  skity_canvas get_canvas() const { return m_canvas; }

  int32_t getWidth() const { return m_width; }
  int32_t getHeight() const { return m_height; }

 private:
  void init_gl();

 private:
  ttoffice::tttext::skity_c::Context ctx_ = {};
  ttoffice::tttext::skity_c::Surface m_surface = {};
  skity_canvas m_canvas = nullptr;
  int32_t m_width = {};
  int32_t m_height = {};
  skity_color4f m_clear_color = {1.f, 1.f, 1.f, 1.f};
};

#endif  // DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_
