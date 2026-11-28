// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

#include <cstdio>
#include <cstdlib>

#include "src/capi/textra_internal.h"
#include "src/textlayout/single_line_paragraph_impl.h"
using namespace ttoffice::tttext;
#define CHECK(x)                                      \
  do {                                                \
    if (!(x)) {                                       \
      std::fprintf(stderr, "%d: %s\n", __LINE__, #x); \
      std::abort();                                   \
    }                                                 \
  } while (0)
#define OK(x) CHECK((x) == TEXTRA_STATUS_OK)
namespace {
int destroyed = 0, calls = 0;
void TEXTRA_CALL Destroy(void*, void*) { ++destroyed; }
textra_status_t TEXTRA_CALL Draw(void*, void*) {
  ++calls;
  return TEXTRA_STATUS_INTERNAL_ERROR;
}
textra_status_t TEXTRA_CALL FontStyleCallback(void*, void*,
                                              textra_font_style_t* out) {
  *out = {TEXTRA_FONT_WEIGHT_NORMAL, TEXTRA_FONT_WIDTH_NORMAL,
          TEXTRA_FONT_SLANT_UPRIGHT};
  return TEXTRA_STATUS_OK;
}
textra_status_t TEXTRA_CALL Advance(void*, void*, uint16_t, float, float*) {
  return TEXTRA_STATUS_INTERNAL_ERROR;
}
struct FontSources {
  std::vector<int> order;
  std::string last_family;
  int destroyed = 0;
};
void TEXTRA_CALL DestroySource(void* userdata, void*) {
  ++static_cast<FontSources*>(userdata)->destroyed;
}
textra_status_t TEXTRA_CALL MatchSource(void* userdata, void* manager,
                                        const char* family,
                                        const textra_font_style_t*,
                                        void** out) {
  auto& state = *static_cast<FontSources*>(userdata);
  state.order.push_back(*static_cast<int*>(manager));
  state.last_family = family;
  *out = state.last_family == "copied-family" ? manager : nullptr;
  return TEXTRA_STATUS_OK;
}
void TestFontSources() {
  FontSources state;
  int objects[]{0, 1, 2, 3};
  textra_backend_font_manager_api_t api{};
  api.struct_size = sizeof(api);
  api.version = TEXTRA_FONT_MANAGER_API_VERSION;
  api.destroy = DestroySource;
  api.match_family_style = MatchSource;
  api.legacy_make_typeface = MatchSource;
  textra_backend_typeface_api_t faces{};
  faces.struct_size = sizeof(faces);
  faces.version = TEXTRA_TYPEFACE_API_VERSION;
  // The test face borrows its manager token.
  faces.destroy = [](void*, void*) {};
  faces.font_style = FontStyleCallback;
  std::array<textra_font_manager_options_t, 4> configs;
  for (int i = 0; i < 4; ++i)
    configs[i] = {sizeof(configs[i]), &objects[i], &api, &faces, &state};
  char family[] = "copied-family";
  textra_text_layout_options_t options{};
  options.struct_size = sizeof(options);
  options.backend = TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT;
  options.default_font_manager = &configs[0];
  options.asset_font_manager = &configs[1];
  options.dynamic_font_manager = &configs[2];
  options.test_font_manager = &configs[3];
  options.default_font_family = family;
  options.default_font_family_length = sizeof(family) - 1;
  textra_text_layout_t* layout = nullptr;
  // Last-slot failure must not release any earlier host reference.
  configs[3].struct_size = 0;
  CHECK(textra_text_layout_create(&options, &layout) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(!layout && state.destroyed == 0);
  configs[3].struct_size = sizeof(configs[3]);
  // Failed initialization leaves the exact same references reusable.
  OK(textra_text_layout_create(&options, &layout));
  family[0] = 'X';
  api = {};  // No table or options pointer may be retained.
  configs = {};
  FontDescriptor descriptor;
  descriptor.font_family_list_.push_back("missing-family");
  CHECK(layout->fonts.findTypefaces(descriptor).size() == 1);
  CHECK(state.order.size() >= 4);
  CHECK((std::vector<int>(state.order.begin(), state.order.begin() + 4) ==
         std::vector<int>{2, 1, 3, 0}));
  CHECK(state.last_family == "copied-family");
  const auto queried = state.order.size();
  CHECK(layout->fonts.findTypefaces(descriptor).size() == 1);
  CHECK(state.order.size() == queried);
  OK(textra_text_layout_clear_font_cache(layout));
  CHECK(layout->fonts.findTypefaces(descriptor).size() == 1);
  CHECK(state.order.size() > queried);
  auto retained = layout->fonts.GetDefaultFontManager();
  textra_text_layout_destroy(layout);
  CHECK(state.destroyed == 3);
  retained = {};
  CHECK(state.destroyed == 4);

  // A short record containing only struct_size selects native defaults.
  textra_font_manager_options_t native{};
  native.struct_size = sizeof(native.struct_size);
  options = {};
  options.struct_size = sizeof(options);
  options.default_font_manager = &native;
  options.disable_font_fallback = TEXTRA_TRUE;
  OK(textra_text_layout_create(&options, &layout));
  CHECK(!layout->fonts.fontFallbackEnabled());
  CHECK(layout->fonts.getFontManagersCount() == 0);
  textra_text_layout_destroy(layout);
}

void TestSharedShaperCache() {
  textra_text_layout_options_t options{};
  options.struct_size = sizeof(options);
  textra_text_layout_t* layout = nullptr;
  OK(textra_text_layout_create(&options, &layout));
  textra_text_context_t *first = nullptr, *second = nullptr;
  OK(textra_text_context_create(&first));
  OK(textra_text_context_create(&second));
  textra_shaper_cache_t* cache = nullptr;
  OK(textra_shaper_cache_create(TEXTRA_FALSE, &cache));
  OK(textra_text_context_set_shaper_cache(first, cache));
  OK(textra_text_context_set_shaper_cache(second, cache));
  textra_shaper_cache_destroy(cache);

  textra_paragraph_options_t single{sizeof(single),
                                    TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE};
  textra_paragraph_t* paragraph = nullptr;
  OK(textra_paragraph_create(&single, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, nullptr, "cache", 5));
  auto shape = [&](textra_text_context_t* context) {
    OK(textra_text_context_reset(context));
    OK(textra_text_layout_layout_single_line(layout, paragraph, context));
    return static_cast<SingleLineParagraphImpl*>(paragraph->impl.get())->shaped;
  };
  auto result = shape(first);
  CHECK(result && shape(second) == result);
  OK(textra_text_context_clear_font_cache(first));
  CHECK(shape(second) == result);
  OK(textra_text_context_clear_shape_cache(first));
  CHECK(shape(second) != result);

  textra_paragraph_destroy(paragraph);
  textra_text_context_destroy(second);
  textra_text_context_destroy(first);
  textra_text_layout_destroy(layout);
}
}  // namespace
int main() {
  TestFontSources();
  TestSharedShaperCache();
  auto manager = PlatformHelper::CreateFontManager(RenderingBackend::kMacOS);
  CHECK(manager);
  CHECK(PlatformHelper::GetDefaultBackend() == RenderingBackend::kMacOS);
  CHECK(PlatformHelper::CreateFontManager());
  CHECK(!PlatformHelper::CreateFontManager(RenderingBackend::kInvalid));
  CHECK(!PlatformHelper::CreateFontManager(RenderingBackend::kAndroid));
  CHECK(!PlatformHelper::CreateFontManager(RenderingBackend::kSkity));
  auto font = CTFontCreateWithName(CFSTR("Helvetica"), 20, nullptr);
  auto face = PlatformHelper::CreateTypefaceHelper(RenderingBackend::kMacOS,
                                                   font, manager);
  CHECK(face && face->GetFontInfo(20).GetAscent() < 0);
  textra_font_descriptor_t* descriptor = nullptr;
  textra_font_manager_options_t mo{};
  mo.struct_size = sizeof(mo);
  CHECK(PlatformHelper::CreateFontManager(mo));
  OK(textra_font_descriptor_create(&descriptor));
  textra_typeface_options_t fo{sizeof(fo), 0, 0, font};
  CHECK(PlatformHelper::CreateTypefaceHelper(font));
  CHECK(PlatformHelper::CreateTypefaceHelper(fo));
  OK(textra_font_descriptor_set_typeface(descriptor, &fo));
  // Imported faces and later matches must share one manager/ID sequence.
  auto sibling =
      descriptor->typeface->manager->createTypefaceFromPlatformFont(font);
  CHECK(sibling &&
        sibling->GetUniqueId() != descriptor->typeface->impl->GetUniqueId());
  CFRelease(font);
  CHECK(descriptor->typeface->impl->GetFontInfo(20).GetDescent() >= 0);
  // A native layout can create its default font manager without host callbacks.
  textra_text_layout_options_t layout_options{};
  layout_options.struct_size = sizeof(layout_options);
  // backend=0 resolves to the compiled native default.
  textra_text_layout_t* layout = nullptr;
  OK(textra_text_layout_create(&layout_options, &layout));
  textra_text_layout_destroy(layout);
  layout_options.default_font_manager = &mo;
  OK(textra_text_layout_create(&layout_options, &layout));
  textra_text_layout_destroy(layout);
  unsigned char pixels[64 * 64 * 4]{};
  auto colors = CGColorSpaceCreateDeviceRGB();
  auto target = CGBitmapContextCreate(pixels, 64, 64, 8, 64 * 4, colors,
                                      kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(colors);
  CHECK(target);
  textra_canvas_options_t native_canvas{sizeof(native_canvas), 0, 0, target};
  CHECK(PlatformHelper::CreateCanvasHelper(target));
  CHECK(PlatformHelper::CreateCanvasHelper(native_canvas));
  textra_layout_drawer_options_t native_draw{sizeof(native_draw),
                                             &native_canvas};
  textra_layout_drawer_t* drawer = nullptr;
  OK(textra_layout_drawer_create(&native_draw, &drawer));
  drawer->canvas->impl->DrawColor(0xffff0000u);
  CHECK(pixels[0] == 255 && pixels[1] == 0);
  textra_layout_drawer_destroy(drawer);
  // Native CGContext is borrowed and remains usable after adapter destruction.
  CGContextSetRGBFillColor(target, 0, 1, 0, 1);
  CGContextFillRect(target, CGRectMake(0, 0, 64, 64));
  CHECK(pixels[0] == 0 && pixels[1] == 255);
  CGContextRelease(target);
  textra_font_descriptor_destroy(descriptor);

  textra_backend_canvas_api_t canvas_api{};
  canvas_api.struct_size = sizeof(canvas_api);
  canvas_api.version = TEXTRA_CANVAS_API_VERSION;
  canvas_api.destroy = Destroy;
  canvas_api.start_paint = Draw;
  canvas_api.save = Draw;
  canvas_api.restore = Draw;
  canvas_api.end_paint = Draw;
  textra_canvas_options_t canvas_options{sizeof(canvas_options),
                                         TEXTRA_RENDERING_BACKEND_MACOS, 0,
                                         &calls, &canvas_api};
  textra_layout_drawer_options_t draw_options{sizeof(draw_options),
                                              &canvas_options};
  CHECK(textra_layout_drawer_create(&draw_options, &drawer) ==
        TEXTRA_STATUS_UNSUPPORTED);
  CHECK(!drawer && destroyed == 0);
  canvas_options.backend = TEXTRA_RENDERING_BACKEND_SKITY;
  OK(textra_layout_drawer_create(&draw_options, &drawer));
  drawer->canvas->impl->StartPaint();
  drawer->canvas->impl->Save();
  drawer->canvas->impl->Restore();
  drawer->canvas->impl->EndPaint();
  CHECK(calls == 4);  // C++ void behavior does not propagate callback statuses.
  textra_layout_drawer_destroy(drawer);
  CHECK(destroyed == 1);
  textra_backend_typeface_api_t face_api{};
  face_api.struct_size = sizeof(face_api);
  face_api.version = TEXTRA_TYPEFACE_API_VERSION;
  face_api.destroy = Destroy;
  face_api.font_style = FontStyleCallback;
  face_api.horizontal_advance = Advance;
  textra_typeface_options_t callback_face{sizeof(callback_face),
                                          TEXTRA_RENDERING_BACKEND_SKITY, 0,
                                          &calls, &face_api};
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_set_typeface(descriptor, &callback_face));
  CHECK(descriptor->typeface->impl->GetHorizontalAdvance(1, 20) == 0);
  textra_font_descriptor_destroy(descriptor);
  CHECK(destroyed == 2);
  std::puts(
      "Native I* factories, native ownership and self-render callback behavior "
      "passed");
}
