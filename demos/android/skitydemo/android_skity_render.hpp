#ifndef EXAMPLE_JNI_ANDROID_SKITY_RENDER_HPP
#define EXAMPLE_JNI_ANDROID_SKITY_RENDER_HPP

#include <skity/gpu/gpu_gl_context.hpp>
#include <skity/skity.hpp>

#define SKITY_DEFAULT_FONT "Roboto Mono Nerd Font Complete.ttf"

class Renderer {
 public:
  Renderer() = default;

  Renderer(glm::vec4 color) : m_clear_color(std::move(color)) {}

  virtual ~Renderer() = default;

  void init(int w, int h, int d);

  void draw();

 protected:
  virtual void onDraw(skity::Canvas* canvas) = 0;

  skity::Canvas* get_canvas() const { return m_canvas.get(); }

  int32_t getWidth() const { return m_width; }
  int32_t getHeight() const { return m_height; }

 private:
  void init_gl();

 private:
  std::unique_ptr<skity::GPUGLContext> ctx_ = {};
  std::unique_ptr<skity::Canvas> m_canvas = {};
  int32_t m_width = {};
  int32_t m_height = {};
  glm::vec4 m_clear_color = {1.f, 1.f, 1.f, 1.f};
};

#endif  // EXAMPLE_JNI_ANDROID_SKITY_RENDER_HPP
