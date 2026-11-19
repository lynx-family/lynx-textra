#ifndef DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_
#define DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_

#include <skity/gpu/gpu_context.hpp>
#include <skity/skity.hpp>

class Renderer {
 public:
  Renderer() = default;

  Renderer(skity::Vec4 color) : m_clear_color(std::move(color)) {}

  virtual ~Renderer() = default;

  void init(int w, int h, int d);

  void draw();

 protected:
  virtual void onDraw(skity::Canvas* canvas) = 0;

  skity::Canvas* get_canvas() const { return m_canvas; }

  int32_t getWidth() const { return m_width; }
  int32_t getHeight() const { return m_height; }

 private:
  void init_gl();

 private:
  std::unique_ptr<skity::GPUContext> ctx_ = {};
  std::unique_ptr<skity::GPUSurface> m_surface = {};
  skity::Canvas* m_canvas = nullptr;
  int32_t m_width = {};
  int32_t m_height = {};
  skity::Vec4 m_clear_color = {1.f, 1.f, 1.f, 1.f};
};

#endif  // DEMOS_ANDROID_SKITYDEMO_SRC_MAIN_JNI_ANDROID_SKITY_RENDER_HPP_
