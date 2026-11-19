// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Test-only TU: relink into a copy of the core so hidden shaping entry points
// can be measured. Not compiled into the production library or its C ABI.

#include <jni.h>
#include <textra/platform_helper.h>
#include <textra/text_layout.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "src/capi/textra_internal.h"
#include "src/textlayout/tt_shaper.h"

namespace {
using namespace ttoffice::tttext;
using Clock = std::chrono::steady_clock;
double Micros(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::micro>(b - a).count();
}
struct Case {
  const char* name;
  std::u32string text;
  bool rtl = false;
};
struct Slot {
  FontManagerRef manager;
  std::unique_ptr<FontmgrCollection> fonts;
  std::unique_ptr<TTShaper> shaper;
  TTTextContext context;
  double setup = 0;
  bool Create(bool use_capi, bool rounded) {
    auto start = Clock::now();
    if (use_capi) {
      textra_font_manager_options_t options{};
      options.struct_size = sizeof(options);
      // Match the manager factory used by C layout initialization; measure
      // shaping without constructing a second TextLayout.
      manager = PlatformHelper::CreateFontManager(options);
    } else {
      manager = PlatformHelper::CreateFontManager();
    }
    if (!manager) return false;
    fonts = std::make_unique<FontmgrCollection>(manager);
    shaper = TTShaper::CreateShaper(fonts.get(), RenderingBackend::kAndroid);
    if (!shaper) return false;
    context.SetAndroidMetricsRound(rounded);
    shaper->SetContext(context);
    setup = Micros(start, Clock::now());
    return true;
  }
};
struct Sample {
  double shape = 0, cleanup = 0, advance = 0;
  uint32_t glyphs = 0;
  float ascent = 0, descent = 0;
};
Sample Measure(Slot& slot, const Case& input, int count, int mode,
               bool inspect = true) {
  ShapeStyle style(FontDescriptor{}, 24);
  const uint32_t length = static_cast<uint32_t>(input.text.size());
  const ShapeKey key(input.text.data(), length, &style, input.rtl,
                     RenderingBackend::kAndroid);
  slot.context.EnableFeature(kDisableShapeCache, mode != 2);
  Sample sample;
  for (int i = 0; i < count; ++i) {
    auto start = Clock::now();
    std::shared_ptr<ShapeResult> result;
    if (mode == 0) {
      result = std::make_shared<ShapeResult>(length, input.rtl);
      slot.shaper->OnShapeText(key, result.get());
    } else
      result =
          slot.shaper->ShapeText(input.text.data(), length, &style, input.rtl);
    auto shaped = Clock::now();
    sample.shape += Micros(start, shaped);
    if (!result || result->GlyphCount() == 0) {
      sample.shape = -1;
      return sample;
    }
    if (inspect && i == count - 1) {
      sample.glyphs = result->GlyphCount();
      for (uint32_t g = 0; g < sample.glyphs; ++g)
        sample.advance += result->Advances(g)[0];
      const auto& metrics = result->Font(0)->GetFontInfo(24);
      sample.ascent = metrics.GetAscent();
      sample.descent = metrics.GetDescent();
    }
    auto cleanup = Clock::now();
    result.reset();
    sample.cleanup += Micros(cleanup, Clock::now());
  }
  sample.shape /= count;
  sample.cleanup /= count;
  return sample;
}
bool Same(const Sample& a, const Sample& b) {
  return a.shape >= 0 && b.shape >= 0 && a.glyphs == b.glyphs &&
         std::abs(a.advance - b.advance) < 0.001 &&
         std::abs(a.ascent - b.ascent) < 0.001 &&
         std::abs(a.descent - b.descent) < 0.001;
}
void Values(std::string& json, const std::vector<double>& values) {
  json += '[';
  for (size_t i = 0; i < values.size(); ++i) {
    char b[64];
    snprintf(b, sizeof(b), "%s%.6f", i ? "," : "", values[i]);
    json += b;
  }
  json += ']';
}
}  // namespace
extern "C" JNIEXPORT jstring JNICALL
Java_com_lynx_textra_demo_CapiDemoActivity_nativeShapingBenchmark(
    JNIEnv* env, jclass, jint repetitions, jint workload_rotation) {
  const int rounds = std::max(3, std::min(31, repetitions));
  constexpr int batch = 512, warmup = 2048;

  auto before = Clock::now();
  auto status = textra_android_runtime_initialize(env);
  double initialization_us = Micros(before, Clock::now());
  if (status)
    return env->NewStringUTF("{\"error\":\"runtime initialization\"}");
  std::vector<Case> cases{
      {"latin_short", U"Hello office ffi world"},
      {"cjk_short", U"中文排版测试，性能对比与字体回退。"},
      {"mixed_256", U""},
      {"arabic", U"مرحبا بالعالم هذه فقرة لاختبار الأداء وتشكيل النص العربي",
       true},
      {"emoji", U"Font fallback 中文 🎉😀 世界🎉😀"}};
  while (cases[2].text.size() < 256)
    cases[2].text += U"Hello office ffi 世界，测试字体 fallback. ";
  cases[2].text.resize(256);
  // Warm the entire Java/runtime path before any timed workload. Per-case
  // warmup alone leaves the first case sensitive to process-start/JIT work.
  bool prewarm_valid = true;
  {
    Slot cpp[2], capi[2];
    for (int policy = 0; policy < 2; ++policy)
      prewarm_valid &= cpp[policy].Create(false, policy) &&
                       capi[policy].Create(true, policy);
    if (!prewarm_valid)
      return env->NewStringUTF("{\"error\":\"prewarm setup\"}");
    auto until = Clock::now() + std::chrono::seconds(2);
    do {
      for (int policy = 0; policy < 2; ++policy)
        for (const auto& input : cases)
          prewarm_valid &= Same(Measure(cpp[policy], input, 32, 1),
                                Measure(capi[policy], input, 32, 1));
    } while (Clock::now() < until);
  }
  TextLayout::ClearShapeCache();
  const size_t rotation = static_cast<size_t>((workload_rotation % 5 + 5) % 5);
  std::rotate(cases.begin(), cases.begin() + rotation, cases.end());
  std::string json =
      "{\"version\":6,\"global_warmup_ms\":2000,\"workload_rotation\":" +
      std::to_string(rotation) + ",\"rounds\":" + std::to_string(rounds) +
      ",\"batch\":" + std::to_string(batch) +
      ",\"warmup\":" + std::to_string(warmup) +
      ",\"unit\":\"microseconds_per_call\",\"cache_miss\":\"explicitly "
      "disabled\",\"compiler\":\"same release core flags, native I* factories "
      "via C++ and C owner initialization\",\"initialization_us\":" +
      std::to_string(initialization_us) + ",\"rows\":[";
  bool valid = prewarm_valid;
  int row = 0;
  for (bool rounded : {false, true})
    for (const auto& input : cases) {
      for (int mode = 0; mode < 4; ++mode) {
        std::vector<double> cpp, capi, cpp_cleanup, capi_cleanup, cpp_setup,
            capi_setup;
        bool equal = true;
        if (mode == 3) {
          for (int r = 0; r < rounds; ++r) {
            Slot a, b;
            bool ready = a.Create(false, rounded) && b.Create(true, rounded);
            if (!ready) {
              equal = false;
              break;
            }
            Sample x, y;
            if (r % 2 == 0) {
              x = Measure(a, input, 1, 1);
              y = Measure(b, input, 1, 1);
            } else {
              y = Measure(b, input, 1, 1);
              x = Measure(a, input, 1, 1);
            }
            equal &= Same(x, y);
            cpp.push_back(x.shape);
            capi.push_back(y.shape);
            cpp_setup.push_back(a.setup);
            capi_setup.push_back(b.setup);
            cpp_cleanup.push_back(x.cleanup);
            capi_cleanup.push_back(y.cleanup);
          }
        } else {
          Slot a, b;
          bool ready = a.Create(false, rounded) && b.Create(true, rounded);
          if (!ready) {
            equal = false;
            continue;
          }
          equal &= Same(Measure(a, input, warmup, mode),
                        Measure(b, input, warmup, mode));
          for (int r = 0; r < rounds; ++r) {
            Sample x, y;
            for (int i = 0; i < batch; ++i) {
              Sample first, second;
              if ((r + i + row) % 2 == 0) {
                first = Measure(a, input, 1, mode, false);
                second = Measure(b, input, 1, mode, false);
              } else {
                second = Measure(b, input, 1, mode, false);
                first = Measure(a, input, 1, mode, false);
              }
              equal &= first.shape >= 0 && second.shape >= 0;
              x.shape += first.shape;
              y.shape += second.shape;
              x.cleanup += first.cleanup;
              y.cleanup += second.cleanup;
            }
            cpp.push_back(x.shape / batch);
            capi.push_back(y.shape / batch);
            cpp_cleanup.push_back(x.cleanup / batch);
            capi_cleanup.push_back(y.cleanup / batch);
          }
        }
        TextLayout::ClearShapeCache();
        valid &= equal;
        const char* modes[]{"platform_shape", "engine_cache_miss",
                            "engine_cache_hit", "first_shape_fresh_manager"};
        if (row++) json += ',';
        json += "{\"case\":\"";
        json += input.name;
        json += "\",\"codepoints\":" + std::to_string(input.text.size()) +
                ",\"metrics_round\":" +
                (rounded ? std::string("true") : std::string("false")) +
                ",\"mode\":\"" + modes[mode] + "\",\"results_equal\":" +
                (equal ? std::string("true") : std::string("false")) +
                ",\"cpp\":";
        Values(json, cpp);
        json += ",\"capi\":";
        Values(json, capi);
        json += ",\"cpp_cleanup\":";
        Values(json, cpp_cleanup);
        json += ",\"capi_cleanup\":";
        Values(json, capi_cleanup);
        json += ",\"cpp_setup\":";
        Values(json, cpp_setup);
        json += ",\"capi_setup\":";
        Values(json, capi_setup);
        json += '}';
      }
    }
  TextLayout::ClearShapeCache();
  status = TEXTRA_STATUS_OK;
  json += "],\"results_equal\":" +
          (valid && status == 0 ? std::string("true") : std::string("false")) +
          ",\"cleanup_status\":" + std::to_string(status) + "}";
  return env->NewStringUTF(json.c_str());
}

// Native C API and C++ share the original Java I* implementation.
extern "C" JNIEXPORT jstring JNICALL
Java_com_lynx_textra_demo_CapiDemoActivity_nativeFontPolicyTest(JNIEnv* env,
                                                                jclass) {
  using namespace ttoffice::tttext;
  std::string json = "[";
  auto record = [&](const char* name, bool ok) {
    if (json.size() > 1) json += ',';
    json += "{\"name\":\"";
    json += name;
    json += "\",\"passed\":";
    json += ok ? "true}" : "false}";
  };
  auto status = textra_android_runtime_initialize(env);
  record("native_runtime_initialization", !status);
  Slot cpp, capi;
  bool ready = !status && cpp.Create(false, false) && capi.Create(true, false);
  record("native_managers_ready", ready);
  if (ready) {
    auto shape = [&](Slot& slot, const char32_t* text, uint32_t length,
                     float size, bool rounded) {
      slot.context.SetAndroidMetricsRound(rounded);
      ShapeStyle style(FontDescriptor{}, size);
      return slot.shaper->ShapeText(text, length, &style, false);
    };
    auto same = [](const std::shared_ptr<ShapeResult>& a,
                   const std::shared_ptr<ShapeResult>& b, float size) {
      if (!a || !b || a->GlyphCount() != b->GlyphCount()) return false;
      for (uint32_t i = 0; i < a->GlyphCount(); ++i) {
        if (a->Glyphs(i) != b->Glyphs(i) ||
            std::abs(a->Advances(i)[0] - b->Advances(i)[0]) > 0.001f)
          return false;
        auto& x = a->Font(i)->GetFontInfo(size);
        auto& y = b->Font(i)->GetFontInfo(size);
        if (std::abs(x.GetAscent() - y.GetAscent()) > 0.001f ||
            std::abs(x.GetDescent() - y.GetDescent()) > 0.001f)
          return false;
      }
      return true;
    };
    struct Text {
      const char* name;
      const char32_t* text;
      uint32_t length;
    };
    const Text samples[]{{"latin", U"office ffi", 10},
                         {"cjk", U"中文", 2},
                         {"supplementary", U"𠮷中文", 3},
                         {"mixed", U"office 中文", 9}};
    for (float size : {24.0f, 24.5f}) {
      for (bool rounded : {false, true}) {
        cpp.context.EnableFeature(kDisableShapeCache, true);
        capi.context.EnableFeature(kDisableShapeCache, true);
        for (const auto& text : samples) {
          auto a = shape(cpp, text.text, text.length, size, rounded);
          auto b = shape(capi, text.text, text.length, size, rounded);
          std::string name = std::string(text.name) +
                             (rounded ? "-rounded-" : "-raw-") +
                             std::to_string(size);
          record(name.c_str(), same(a, b, size));
        }
      }
    }
    cpp.context.EnableFeature(kDisableShapeCache, false);
    capi.context.EnableFeature(kDisableShapeCache, false);
    TextLayout::ClearShapeCache();
    auto raw = shape(capi, U"cached 中文", 9, 24.5f, false);
    auto rounded = shape(capi, U"cached 中文", 9, 24.5f, true);
    auto hit = shape(capi, U"cached 中文", 9, 24.5f, true);
    record("shape_cache_metrics_modes", raw && rounded && raw != rounded);
    record("shape_cache_reuses_native_faces", rounded && hit == rounded);
    textra_font_descriptor_t* descriptor = nullptr;
    status = textra_font_descriptor_create(&descriptor);
    auto cls = env->FindClass("android/graphics/Typeface");
    auto field = cls ? env->GetStaticFieldID(cls, "DEFAULT",
                                             "Landroid/graphics/Typeface;")
                     : nullptr;
    auto font = field ? env->GetStaticObjectField(cls, field) : nullptr;
    textra_typeface_options_t fo{sizeof(fo), TEXTRA_RENDERING_BACKEND_ANDROID,
                                 0, font};
    if (!status) status = textra_font_descriptor_set_typeface(descriptor, &fo);
    bool imported =
        !status && descriptor && descriptor->impl.platform_font_ != 0;
    record("native_typeface_import", imported);
    if (imported) {
      ShapeStyle explicit_style(descriptor->impl, 24);
      auto shaped =
          capi.shaper->ShapeText(U"native", 6, &explicit_style, false);
      record("native_typeface_shaping", shaped && shaped->GlyphCount() == 6);
    }
    if (font) env->DeleteLocalRef(font);
    if (cls) env->DeleteLocalRef(cls);
    textra_font_descriptor_destroy(descriptor);
  }
  TextLayout::ClearShapeCache();
  json += ']';
  return env->NewStringUTF(json.c_str());
}
