#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

#include <ctime>

#include "android_skity_render.hpp"
#include "perf.hpp"

void render_frame_demo(
    skity::Canvas* canvas,
    const std::vector<std::shared_ptr<skity::Pixmap>>& images,
    skity::Typeface* typeface, skity::Typeface* emoji, float mx, float my,
    float width, float height, float t);

static double skity_get_time() {
  struct timespec res = {};
  clock_gettime(CLOCK_REALTIME, &res);

  return res.tv_sec + (double)res.tv_nsec / (double)1e9;
}

class AndroidGLFrameExample : public Renderer {
 public:
  AndroidGLFrameExample()
      : Renderer(glm::vec4{0.3f, 0.3f, 0.32f, 1.f}),
        fpsGraph(Perf::GRAPH_RENDER_FPS, "Frame Time"),
        cpuGraph(Perf::GRAPH_RENDER_MS, "CPU Time") {}

  ~AndroidGLFrameExample() override = default;

  void setTypeface(skity::Typeface* typeface, skity::Typeface* emoji) {
    m_render_typeface = typeface;
    m_emoji_typeface = emoji;
  }

  void setImages(const std::vector<std::shared_ptr<skity::Pixmap>>& images) {
    m_render_images = images;
  }

 protected:
  void onDraw(skity::Canvas* canvas) override {
    m_time = skity_get_time();

    if (m_prev_time == 0) {
      m_prev_time = m_time;
      m_start_time = m_time;
    }

    double dt = m_time - m_prev_time;
    m_prev_time = m_time;

    render_frame_demo(canvas, m_render_images, m_render_typeface,
                      m_emoji_typeface, 0.f, 0.f, getWidth(), getHeight(),
                      static_cast<float>(m_time - m_start_time));

    m_cpu_time = skity_get_time() - m_time;

    fpsGraph.RenderGraph(canvas, 5, 5);
    cpuGraph.RenderGraph(canvas, 5 + 200 + 5, 5);

    fpsGraph.UpdateGraph(dt);
    cpuGraph.UpdateGraph(m_cpu_time);
  }

 private:
  double m_time = {};
  double m_start_time = {};
  double m_prev_time = {};
  double m_cpu_time = {};
  skity::Typeface* m_render_typeface = {};
  skity::Typeface* m_emoji_typeface = {};
  std::vector<std::shared_ptr<skity::Pixmap>> m_render_images = {};
  Perf fpsGraph;
  Perf cpuGraph;
};

jlong AndroidGLFrameExample_Create(JNIEnv* env, jobject thiz, jint width,
                                   jint height, jint density, jobject images,
                                   jobject asset_manager) {
  auto render = new AndroidGLFrameExample();

  render->init(width, height, density);

  // init typeface
  auto am = AAssetManager_fromJava(env, asset_manager);
  {
    auto font_asset =
        AAssetManager_open(am, "Roboto-Regular.ttf", AASSET_MODE_BUFFER);

    const void* buf = AAsset_getBuffer(font_asset);
    ssize_t length = AAsset_getLength(font_asset);

    auto font_data = skity::Data::MakeWithCopy(buf, length);

    auto render_typeface = skity::Typeface::MakeFromData(font_data);

    auto emoji_asset =
        AAssetManager_open(am, "NotoEmoji-Regular.ttf", AASSET_MODE_BUFFER);

    auto emoji_buf = AAsset_getBuffer(emoji_asset);
    auto emoji_length = AAsset_getLength(emoji_asset);

    auto emoji_data = skity::Data::MakeWithCopy(emoji_buf, emoji_length);

    auto emoji_typeface = skity::Typeface::MakeFromData(emoji_data);

    render->setTypeface(render_typeface, emoji_typeface);

    AAsset_close(font_asset);
    AAsset_close(emoji_asset);
  }

  // images
  {
    auto list_class = env->GetObjectClass(images);
    auto size_method = env->GetMethodID(list_class, "size", "()I");
    auto get_method =
        env->GetMethodID(list_class, "get", "(I)Ljava/lang/Object;");

    std::vector<std::shared_ptr<skity::Pixmap>> skity_images = {};

    int size = env->CallIntMethod(images, size_method);

    for (int i = 0; i < size; i++) {
      auto bitmap = env->CallObjectMethod(images, get_method, i);

      AndroidBitmapInfo info;
      AndroidBitmap_getInfo(env, bitmap, &info);

      void* addr = nullptr;
      AndroidBitmap_lockPixels(env, bitmap, &addr);

      auto data = skity::Data::MakeWithCopy(addr, info.height * info.stride);

      skity_images.emplace_back(std::make_shared<skity::Pixmap>(
          data, info.stride, info.width, info.height));

      AndroidBitmap_unlockPixels(env, bitmap);
    }

    render->setImages(skity_images);
  }

  return (jlong)render;
}

int JNIEXPORT register_skity_jni_AndroidGLFrameExample(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {"nativeCreateGLFrameRender",
       "(IIILjava/util/List;Landroid/content/res/AssetManager;)J",
       reinterpret_cast<void*>(AndroidGLFrameExample_Create)},
  };

  const auto clazz = env->FindClass(SKITY_JNI_PKG "/SkityGLFrameRender");
  return clazz ? env->RegisterNatives(clazz, methods, 1) : JNI_ERR;
}
