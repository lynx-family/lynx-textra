// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Test-only stdin/stdout adapter; mobile hosts must initialize their runtime.
#include "test/perf/shaping_benchmark.h"

#include <textra/fontmgr_collection.h>
#include <textra/platform_helper.h>
#include <textra/tttext_context.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "src/textlayout/tt_shaper.h"
#ifdef TEXTRA_PERF_SKITY
#include <textra/platform/skity/skity_typeface_helper.h>
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#include <sys/sysctl.h>

#include "src/ports/native/ios/typeface_coretext.h"
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#endif

namespace {
using namespace ttoffice::tttext;  // NOLINT(build/namespaces)
using Clock = std::chrono::steady_clock;
using Writer = rapidjson::Writer<rapidjson::StringBuffer>;
uint64_t Mix(uint64_t h, uint64_t value) {
  return (h ^ value) * 1099511628211ull;
}
double Micros(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::micro>(b - a).count();
}
void String(Writer* out, const char* key, const std::string& value) {
  out->Key(key);
  out->String(value.c_str());
}
#ifdef TEXTRA_PERF_SKITY
class FixedFonts final : public IFontManager {
 public:
  explicit FixedFonts(std::vector<TypefaceRef> faces)
      : faces_(std::move(faces)) {}
  size_t Index(const TypefaceRef& face) const {
    const auto found = std::find(faces_.begin(), faces_.end(), face);
    return static_cast<size_t>(found - faces_.begin());
  }
  int countFamilies() const override { return static_cast<int>(faces_.size()); }
  TypefaceRef matchFamilyStyle(const char[], const FontStyle&) override {
    return faces_.front();
  }
  TypefaceRef matchFamilyStyleCharacter(const char[], const FontStyle&,
                                        const char*[], int,
                                        uint32_t ch) override {
    for (const auto& face : faces_) {
      if (face->UnicharToGlyph(ch)) return face;
    }
    return nullptr;
  }
  TypefaceRef makeFromFile(const char[], int) override { return nullptr; }
  TypefaceRef legacyMakeTypeface(const char[], FontStyle) const override {
    return faces_.front();
  }

 private:
  std::vector<TypefaceRef> faces_;
};
#endif
}  // namespace

int RunShapingBenchmark(const std::string& input,
                        const std::string& metadata_text,
                        std::string* response) {
  if (!response) return 2;
  rapidjson::Document metadata;
  metadata.Parse(metadata_text.c_str());
  if (metadata.HasParseError() || !metadata.IsObject()) return 2;
  for (const auto* key : {"revision", "dependencies", "benchmark"}) {
    if (!metadata.HasMember(key) || !metadata[key].IsString()) return 2;
  }
  rapidjson::Document request;
  request.Parse(input.c_str());
  if (request.HasParseError() || !request.IsObject()) return 2;
  for (const auto* key : {"backend", "request_id", "revision"}) {
    if (!request.HasMember(key) || !request[key].IsString()) return 2;
  }
  for (const auto* key : {"warmup", "batches", "iterations"}) {
    if (!request.HasMember(key) || !request[key].IsInt() ||
        request[key].GetInt() < 1 || request[key].GetInt() > 100000)
      return 2;
  }
  if (!request.HasMember("schema") || request["schema"] != 1 ||
      !request.HasMember("cache_disabled") ||
      !request["cache_disabled"].IsTrue() || !request.HasMember("codepoints") ||
      !request["codepoints"].IsArray() || request["codepoints"].Empty() ||
      !request.HasMember("fonts") || !request["fonts"].IsArray() ||
      !request.HasMember("case") || !request["case"].IsObject())
    return 2;
  const auto& item = request["case"];
  if (!item.HasMember("id") || !item["id"].IsString() ||
      !item.HasMember("primary") || !item["primary"].IsString() ||
      !item.HasMember("rtl") || !item["rtl"].IsBool())
    return 2;
  const std::string backend_name = request["backend"].GetString();
  RenderingBackend backend = RenderingBackend::kInvalid;
  if (backend_name == "skity") backend = RenderingBackend::kSkity;
  if (backend_name == "macos") backend = RenderingBackend::kMacOS;
  if (backend_name == "ios") backend = RenderingBackend::kIOS;
  if (backend_name == "android") backend = RenderingBackend::kAndroid;
  if (backend_name == "harmony") backend = RenderingBackend::kOHOS;
  if (backend == RenderingBackend::kInvalid ||
      !PlatformHelper::IsBackendAvailable(backend))
    return 4;
#ifndef TEXTRA_PERF_SKITY
  if (backend == RenderingBackend::kSkity) return 4;
#endif
  FontManagerRef manager;
  uint64_t font_hash = 1469598103934665603ull;
#ifdef TEXTRA_PERF_SKITY
  if (backend == RenderingBackend::kSkity) {
    if (!metadata.HasMember("fonts") || !metadata["fonts"].IsString()) return 2;
    static std::vector<std::string> loaded_paths;
    static std::vector<TypefaceRef> loaded_faces;
    static std::string loaded_digest;
    std::vector<std::string> paths;
    for (const auto& path : request["fonts"].GetArray()) {
      if (!path.IsString()) return 2;
      paths.emplace_back(path.GetString());
      for (const char* ch = path.GetString(); *ch; ++ch) {
        font_hash = Mix(font_hash, static_cast<unsigned char>(*ch));
      }
    }
    if (paths != loaded_paths ||
        loaded_digest != metadata["fonts"].GetString()) {
      std::vector<TypefaceRef> faces;
      for (const auto& path : paths) {
        const auto face = skity::Typeface::MakeFromFile(path.c_str());
        if (!face) return 3;
        faces.push_back(
            std::make_shared<skity::textlayout::SkityTypefaceHelper>(face));
      }
      loaded_faces = std::move(faces);
      loaded_paths = paths;
      loaded_digest = metadata["fonts"].GetString();
    }
    auto faces = loaded_faces;
    if (faces.empty()) return 3;
    if (std::string(request["case"]["primary"].GetString()) == "han") {
      if (faces.size() < 2) return 3;
      std::swap(faces[0], faces[1]);
    }
    manager = std::make_shared<FixedFonts>(std::move(faces));
  }
#endif
  if (!manager) manager = PlatformHelper::CreateFontManager(backend);
  if (!manager) return 4;
  FontmgrCollection fonts(manager);
  auto shaper = TTShaper::CreateShaper(&fonts, backend);
  if (!shaper) return 4;
  TTTextContext context;
  context.EnableFeature(FeatureOption::kDisableShapeCache, true);
  shaper->SetContext(context);
  ShapeStyle style(FontDescriptor{}, 16);
  std::u32string text;
  for (const auto& cp : request["codepoints"].GetArray()) {
    if (!cp.IsUint() || cp.GetUint() > 0x10FFFF ||
        (cp.GetUint() >= 0xD800 && cp.GetUint() <= 0xDFFF))
      return 2;
    text.push_back(static_cast<char32_t>(cp.GetUint()));
  }
  const auto shape = [&]() {
    return shaper->ShapeText(text.data(), static_cast<uint32_t>(text.size()),
                             &style, request["case"]["rtl"].GetBool());
  };
  auto first = shape();
  if (!first || first->GlyphCount() == 0 || first->CharCount() != text.size())
    return 5;
  uint64_t signature = 1469598103934665603ull;
  uint32_t missing = 0;
  signature = Mix(signature, first->CharCount());
  for (uint32_t i = 0; i < first->CharCount(); ++i) {
    signature = Mix(signature, first->CharToGlyph(i));
  }
  std::vector<const ITypefaceHelper*> font_order;
  for (uint32_t i = 0; i < first->GlyphCount(); ++i) {
    signature = Mix(signature, first->Glyphs(i));
    signature = Mix(signature, first->GlyphToChar(i));
    signature =
        Mix(signature,
            static_cast<uint64_t>(std::llround(first->Advances(i)[0] * 65536)));
    signature =
        Mix(signature,
            static_cast<uint64_t>(std::llround(first->Advances(i)[1] * 65536)));
    for (int axis = 0; axis < 2; ++axis) {
      signature = Mix(signature, static_cast<uint64_t>(std::llround(
                                     first->Positions(i)[axis] * 65536)));
    }
    const auto& face = first->Font(i);
    if (!face) return 5;
    // Include font assignment without hashing process-local pointer/ID values.
    auto found = std::find(font_order.begin(), font_order.end(), face.get());
    if (found == font_order.end()) {
      font_order.push_back(face.get());
      found = font_order.end() - 1;
    }
    signature =
        Mix(signature, static_cast<uint64_t>(found - font_order.begin()));
#ifdef __APPLE__
    if (backend == RenderingBackend::kMacOS ||
        backend == RenderingBackend::kIOS) {
      auto ct = std::static_pointer_cast<TypefaceCoreText>(face);
      auto name = CTFontCopyPostScriptName(ct->GetFontRef());
      if (!name) return 5;
      for (CFIndex j = 0; j < CFStringGetLength(name); ++j) {
        signature = Mix(signature, CFStringGetCharacterAtIndex(name, j));
      }
      CFRelease(name);
    }
#endif
#ifdef TEXTRA_PERF_SKITY
    if (backend == RenderingBackend::kSkity) {
      signature = Mix(signature, static_cast<uint64_t>(face->GetFontIndex()));
      // FixedFonts preserves the input-file order as stable font identity.
      auto fixed = std::static_pointer_cast<FixedFonts>(manager);
      signature = Mix(signature, fixed->Index(face));
    }
#endif
    missing += first->Glyphs(i) == 0;
  }
  const auto glyphs = first->GlyphCount();
  first.reset();
  for (int i = 0; i < request["warmup"].GetInt(); ++i) {
    if (!shape()) return 5;
  }
  rapidjson::StringBuffer buffer;
  Writer out(buffer);
  out.StartObject();
  out.Key("schema");
  out.Int(1);
  String(&out, "revision", metadata["revision"].GetString());
  String(&out, "request_id", request["request_id"].GetString());
  String(&out, "case", request["case"]["id"].GetString());
  out.Key("cache_disabled");
  out.Bool(true);
  out.Key("identity");
  out.StartObject();
#ifdef _WIN32
  char hostname[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD size = sizeof(hostname);
  if (!GetComputerNameA(hostname, &size)) return 6;
  String(&out, "device", hostname);
  String(&out, "os", "windows");
  String(&out, "arch", sizeof(void*) == 8 ? "64-bit" : "32-bit");
#else
  struct utsname info;
  if (uname(&info) != 0) return 6;
#if defined(__APPLE__) && !TARGET_OS_IPHONE
  // Hostnames can change with DNS/network changes. A boot-session fingerprint
  // remains stable during one comparison and invalidates a run after reboot.
  char boot[128];
  size_t boot_size = sizeof(boot);
  if (sysctlbyname("kern.bootsessionuuid", boot, &boot_size, nullptr, 0) != 0)
    return 6;
  uint64_t boot_hash = 1469598103934665603ull;
  for (size_t i = 0; i < boot_size; ++i)
    boot_hash = Mix(boot_hash, static_cast<unsigned char>(boot[i]));
  String(&out, "device", "mac-boot-" + std::to_string(boot_hash));
#else
  String(&out, "device", info.nodename);
#endif
  String(&out, "os", info.release);
  String(&out, "arch", info.machine);
#endif
  String(&out, "backend", backend_name);
  String(&out, "fonts",
         backend_name == "skity" ? std::string(metadata["fonts"].GetString()) +
                                       ":" + std::to_string(font_hash)
                                 : "system");
  String(&out, "dependencies", metadata["dependencies"].GetString());
  String(&out, "benchmark", metadata["benchmark"].GetString());
  out.EndObject();
  out.Key("output");
  out.StartObject();
  String(
      &out, "glyph_semantics",
      backend == RenderingBackend::kAndroid ? "utf16-text" : "font-glyph-ids");
  out.Key("glyphs");
  out.Uint(glyphs);
  out.Key("missing");
  out.Uint(missing);
  String(&out, "signature", std::to_string(signature));
  out.EndObject();
  out.Key("batches");
  out.StartArray();
  for (int batch = 0; batch < request["batches"].GetInt(); ++batch) {
    double shaping = 0, cleanup = 0, total = 0;
    const auto iterations = request["iterations"].GetInt();
    if (iterations < 1) return 2;
    for (int i = 0; i < iterations; ++i) {
      const auto a = Clock::now();
      auto result = shape();
      const auto b = Clock::now();
      if (!result) return 5;
      result.reset();
      const auto c = Clock::now();
      shaping += Micros(a, b);
      cleanup += Micros(b, c);
      total += Micros(a, c);
    }
    out.StartObject();
    out.Key("shape");
    out.Double(shaping / iterations);
    out.Key("cleanup");
    out.Double(cleanup / iterations);
    out.Key("total");
    out.Double(total / iterations);
    out.EndObject();
  }
  out.EndArray();
  out.EndObject();
  *response = buffer.GetString();
  return 0;
}

#ifndef TEXTRA_PERF_NO_MAIN
int main(int argc, char** argv) {
  const bool server = argc == 3 && std::string(argv[2]) == "--server";
  if (argc != 2 && !server) return 2;
  std::ifstream metadata_file(argv[1]);
  if (!metadata_file) return 2;
  const std::string metadata((std::istreambuf_iterator<char>(metadata_file)),
                             {});
  if (server) {
    std::string input;
    while (std::getline(std::cin, input)) {
      std::string response;
      const auto result = RunShapingBenchmark(input, metadata, &response);
      if (result) return result;
      std::cout << response << std::endl;
    }
    return 0;
  }
  const std::string input((std::istreambuf_iterator<char>(std::cin)), {});
  std::string response;
  const auto result = RunShapingBenchmark(input, metadata, &response);
  if (!result) std::cout << response << std::endl;
  return result;
}
#endif
