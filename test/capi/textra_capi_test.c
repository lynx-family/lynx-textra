// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <textra/capi/textra.h>
#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <TargetConditionals.h>
#include <sys/mman.h>
#include <unistd.h>
#if TARGET_OS_IPHONE
#define TEST_NATIVE_BACKEND TEXTRA_RENDERING_BACKEND_IOS
#define TEST_SELF_BACKEND TEXTRA_RENDERING_BACKEND_IOS_SELF_RENDERING
#else
#define TEST_NATIVE_BACKEND TEXTRA_RENDERING_BACKEND_MACOS
#define TEST_SELF_BACKEND TEXTRA_RENDERING_BACKEND_MACOS_SELF_RENDERING
#endif
#endif
#if defined(__APPLE__)
static textra_rendering_backend_t test_backend = TEST_NATIVE_BACKEND;
#define TEST_BACKEND test_backend
#endif

#define CHECK(condition)                                              \
  do {                                                                \
    if (!(condition)) {                                               \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      abort();                                                        \
    }                                                                 \
  } while (0)
#define OK(expression) CHECK((expression) == TEXTRA_STATUS_OK)

static void test_bounded_utf8_input(void) {
  // The invalid suffix is outside byte_count and must not be inspected.
  const char input[] = {'A', '\xf0', '\x9f', '\x98', '\x80', '\xff', 0};
  const char* text = input;
#if defined(__APPLE__)
  long page_size = sysconf(_SC_PAGESIZE);
  CHECK(page_size > 0);
  size_t page = (size_t)page_size;
  char* memory = (char*)mmap(NULL, page * 2, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANON, -1, 0);
  CHECK(memory != MAP_FAILED);
  CHECK(mprotect(memory + page, page, PROT_NONE) == 0);
  char* bounded = memory + page - 5;
  memcpy(bounded, input, 5);
  text = bounded;
#endif
  for (uint32_t type = TEXTRA_PARAGRAPH_TYPE_GENERAL;
       type <= TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE; ++type) {
    textra_paragraph_options_t options = {0};
    options.struct_size = sizeof(options);
    options.type = (textra_paragraph_type_t)type;
    textra_paragraph_t* paragraph = NULL;
    OK(textra_paragraph_create(&options, &paragraph));
    OK(textra_paragraph_add_text_run(paragraph, NULL, text, 5));
    uint32_t count = 0, size = 0;
    OK(textra_paragraph_get_char_count(paragraph, &count));
    CHECK(count == 2);
    textra_text_range_t range = {0, 2};
    char output[5];
    OK(textra_paragraph_copy_text(paragraph, &range, output, sizeof(output),
                                  &size));
    CHECK(size == 5 && !memcmp(output, input, 5));
    textra_paragraph_destroy(paragraph);
  }
#if defined(__APPLE__)
  CHECK(munmap(memory, page * 2) == 0);
#endif
}

static void test_font_features(void) {
  textra_font_descriptor_t *descriptor = NULL, *clone = NULL, *snapshot = NULL;
  textra_style_t* style = NULL;
  uint32_t count = 99;
  textra_font_feature_t values[] = {{{'l', 'i', 'g', 'a'}, 0},
                                    {{'c', 'v', '0', '1'}, 2},
                                    {{'l', 'i', 'g', 'a'}, 1},
                                    {{'x', 'x', 'x', 'x'}, -1}};
  textra_font_feature_t result = {{'t', 'e', 's', 't'}, 123};
  CHECK(sizeof(result) == 8 && offsetof(textra_font_feature_t, value) == 4);
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_get_font_feature_count(descriptor, &count));
  CHECK(count == 0);
  CHECK(textra_font_descriptor_set_font_features(NULL, values, 4) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_font_descriptor_set_font_features(descriptor, NULL, 1) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_font_descriptor_get_font_feature_count(descriptor, NULL) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_font_descriptor_get_font_feature(NULL, 0, &result) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_font_descriptor_get_font_feature(descriptor, 0, NULL) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_font_descriptor_get_font_feature(descriptor, 0, &result) ==
        TEXTRA_STATUS_OUT_OF_RANGE);
  CHECK(result.value == 123 && !memcmp(result.tag, "test", 4));
  OK(textra_font_descriptor_set_font_features(descriptor, values, 4));
  values[0].value = 7;
  OK(textra_font_descriptor_get_font_feature(descriptor, 0, &result));
  CHECK(result.value == 0 && !memcmp(result.tag, "liga", 4));
  OK(textra_font_descriptor_clone(descriptor, &clone));
  OK(textra_style_create(&style));
  OK(textra_style_set_font_descriptor(style, descriptor));
  textra_font_feature_t invalid[] = {{{'k', 'e', 'r', 'n'}, 0},
                                     {{'b', 'a', 'd', 0}, 1}};
  CHECK(textra_font_descriptor_set_font_features(descriptor, invalid, 2) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  invalid[1].tag[3] = (char)0xff;
  CHECK(textra_font_descriptor_set_font_features(descriptor, invalid, 2) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  OK(textra_font_descriptor_get_font_feature_count(descriptor, &count));
  CHECK(count == 4);
  OK(textra_font_descriptor_get_font_feature(descriptor, 0, &result));
  CHECK(result.value == 0);
  OK(textra_font_descriptor_set_font_features(descriptor, NULL, 0));
  OK(textra_font_descriptor_get_font_feature_count(descriptor, &count));
  CHECK(count == 0);
  OK(textra_style_get_font_descriptor(style, &snapshot));
  for (uint32_t i = 0; i < 4; ++i) {
    textra_font_feature_t a, b;
    OK(textra_font_descriptor_get_font_feature(clone, i, &a));
    OK(textra_font_descriptor_get_font_feature(snapshot, i, &b));
    CHECK(!memcmp(&a, &b, sizeof(a)));
    CHECK(!memcmp(a.tag, values[i].tag, 4));
    CHECK(a.value == (i == 0 ? 0 : values[i].value));
  }
  textra_font_descriptor_destroy(snapshot);
  textra_font_descriptor_destroy(clone);
  textra_font_descriptor_destroy(descriptor);
  textra_style_destroy(style);
  CHECK(textra_get_abi_version_minor() == TEXTRA_ABI_VERSION_MINOR);
}

static void test_self_rendering_backend(void) {
  const textra_rendering_backend_t inputs[] = {
      TEXTRA_RENDERING_BACKEND_INVALID,
      TEXTRA_RENDERING_BACKEND_SKIA,
      TEXTRA_RENDERING_BACKEND_SKITY,
      TEXTRA_RENDERING_BACKEND_ANDROID,
      TEXTRA_RENDERING_BACKEND_IOS,
      TEXTRA_RENDERING_BACKEND_MACOS,
      TEXTRA_RENDERING_BACKEND_OHOS,
      TEXTRA_RENDERING_BACKEND_LVGL,
      TEXTRA_RENDERING_BACKEND_IOS_SELF_RENDERING,
      TEXTRA_RENDERING_BACKEND_MACOS_SELF_RENDERING,
      TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT,
      UINT32_MAX};
  for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
    const textra_rendering_backend_t preferred = inputs[i];
    const textra_rendering_backend_t selected =
        textra_get_self_rendering_backend(preferred);
    if (preferred == TEXTRA_RENDERING_BACKEND_SKIA ||
        preferred == TEXTRA_RENDERING_BACKEND_SKITY) {
      CHECK(selected == preferred ||
            selected == TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT ||
            selected == TEXTRA_RENDERING_BACKEND_INVALID);
    } else {
      CHECK(selected == TEXTRA_RENDERING_BACKEND_INVALID);
    }
  }
  CHECK(textra_get_abi_version_major() == 1u);
  CHECK(textra_get_abi_version_minor() >= 1u);
}

static void test_values(void) {
  textra_style_t* style = NULL;
  textra_style_t* copy = NULL;
  textra_bool_t present = 1;
  float number = 0;
  uint32_t color = 1;
  OK(textra_style_create(&style));
  OK(textra_style_set_text_size(style, 24));
  OK(textra_style_set_foreground_color(style, 0));
  OK(textra_style_has_attribute(style, TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR,
                                &present));
  CHECK(present);
  OK(textra_style_get_foreground_color(style, &color));
  CHECK(color == 0);
  OK(textra_style_clone(style, &copy));
  OK(textra_style_unset_attribute(copy, TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE));
  OK(textra_style_has_attribute(copy, TEXTRA_STYLE_ATTRIBUTE_TEXT_SIZE,
                                &present));
  CHECK(!present);
  OK(textra_style_get_text_size(copy, &number));
  CHECK(fabsf(number - 40.0f / 3.0f) < 0.001f);
  OK(textra_style_get_text_size(style, &number));
  CHECK(number == 24);
  CHECK(textra_style_set_text_size(style, NAN) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_style_set_word_break(style, UINT8_MAX) ==
        TEXTRA_STATUS_UNSUPPORTED);
  OK(textra_style_reset(style));
  OK(textra_style_has_attribute(style, TEXTRA_STYLE_ATTRIBUTE_FOREGROUND_COLOR,
                                &present));
  CHECK(!present);
  textra_style_destroy(copy);
  textra_style_destroy(style);

  textra_font_descriptor_t* descriptor = NULL;
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_add_family(descriptor, "test", 4, TEXTRA_TRUE));
  char buffer[4] = {'x', 'x', 'x', 'x'};
  uint32_t size = 0;
  present = 0;
  CHECK(textra_font_descriptor_copy_family(descriptor, 0, buffer, 3, &size,
                                           &present) ==
        TEXTRA_STATUS_BUFFER_TOO_SMALL);
  CHECK(size == 4 && buffer[0] == 'x' && !present);
  OK(textra_font_descriptor_copy_family(descriptor, 0, buffer, 4, &size,
                                        &present));
  CHECK(!memcmp(buffer, "test", 4) && present);
  textra_font_descriptor_destroy(descriptor);

  textra_paragraph_options_t options = {0};
  options.struct_size = sizeof(options);
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&options, &paragraph));
  CHECK(textra_paragraph_add_text_run(paragraph, NULL, "\xf0\x80\x80\x80", 4) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(textra_paragraph_add_text_run(paragraph, NULL, "\xed\xa0\x80", 3) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  OK(textra_paragraph_add_text_run(paragraph, NULL,
                                   "a\xe4\xb8\xad\xf0\x9f\x98\x80", 8));
  uint32_t count = 0;
  OK(textra_paragraph_get_char_count(paragraph, &count));
  CHECK(count == 3);
  textra_text_range_t range = {1, 3};
  OK(textra_paragraph_copy_text(paragraph, &range, NULL, 0, &size));
  CHECK(size == 7);
  CHECK(textra_paragraph_restore_style(paragraph) ==
        TEXTRA_STATUS_INVALID_STATE);
  textra_style_t* appended_style = NULL;
  OK(textra_style_create(&appended_style));
  OK(textra_style_set_text_size(appended_style, 24));
  OK(textra_style_set_bold(appended_style, TEXTRA_TRUE));
  OK(textra_paragraph_save_style(paragraph));
  OK(textra_paragraph_add_text_run(paragraph, appended_style, "x", 1));
  OK(textra_paragraph_restore_style(paragraph));
  textra_style_t* queried = NULL;
  OK(textra_paragraph_query_style(paragraph, 3, &queried));
  OK(textra_style_get_text_size(queried, &number));
  CHECK(number == 24);
  OK(textra_style_get_bold(queried, &present));
  CHECK(!present);
  textra_text_range_t edit = {0, 1};
  CHECK(textra_paragraph_apply_style(paragraph, appended_style, &edit) ==
        TEXTRA_STATUS_UNSUPPORTED);
  textra_style_destroy(queried);
  textra_style_destroy(appended_style);
  textra_paragraph_destroy(paragraph);

  CHECK(textra_get_abi_version_major() == TEXTRA_ABI_VERSION_MAJOR);

  textra_paragraph_style_t* paragraph_style = NULL;
  OK(textra_paragraph_style_create(&paragraph_style));
  const char ellipsis[] = "\xe2\x80\xa6\xf0\x9f\x98\x80";
  OK(textra_paragraph_style_set_ellipsis(paragraph_style, ellipsis, 7));
  char copied_ellipsis[7];
  OK(textra_paragraph_style_copy_ellipsis(paragraph_style, copied_ellipsis,
                                          sizeof(copied_ellipsis), &size));
  CHECK(size == 7 && !memcmp(copied_ellipsis, ellipsis, 7));
  textra_paragraph_style_destroy(paragraph_style);
}

static int destroyed = 0;
static int drawn = 0;
static int reject_draw = 0;
static textra_status_t measure(void* data, textra_run_metrics_t* out) {
  (void)data;
  out->ascent = -12;
  out->descent = 3;
  out->advance = 18;
  return TEXTRA_STATUS_OK;
}
static textra_status_t draw_delegate(void* data, textra_canvas_t* canvas,
                                     float x, float y) {
  (void)data;
  CHECK(canvas && isfinite(x) && isfinite(y));
  ++drawn;
  return reject_draw ? TEXTRA_STATUS_INTERNAL_ERROR : TEXTRA_STATUS_OK;
}
static void destroy_callback(void* data) {
  (void)data;
  ++destroyed;
}
static textra_status_t reject_measure(void* data, textra_run_metrics_t* out) {
  (void)data;
  (void)out;
  return TEXTRA_STATUS_INTERNAL_ERROR;
}
static textra_status_t reject_line(void* data,
                                   const textra_layout_region_t* region,
                                   const textra_line_layout_event_t* event,
                                   textra_layout_flow_t* flow) {
  (void)data;
  (void)region;
  (void)event;
  (void)flow;
  return TEXTRA_STATUS_INTERNAL_ERROR;
}
static textra_status_t reject_range(void* data, float* top, float height,
                                    float start, float end,
                                    textra_range_writer_t writer,
                                    void* writer_context) {
  (void)data;
  (void)top;
  (void)height;
  textra_horizontal_range_t value = {end, start};
  return writer(writer_context, &value, 1);
}
static void test_callbacks(void) {
  textra_run_callbacks_t callbacks = {0};
  callbacks.struct_size = sizeof(callbacks);
  callbacks.version = 1;
  callbacks.measure = reject_measure;
  callbacks.destroy = destroy_callback;
  textra_run_delegate_t* delegate = (textra_run_delegate_t*)1;
  CHECK(textra_run_delegate_create(&callbacks, NULL, &delegate) ==
        TEXTRA_STATUS_INTERNAL_ERROR);
  CHECK(!delegate && destroyed == 0);
  callbacks.measure = measure;
  callbacks.draw = draw_delegate;
  OK(textra_run_delegate_create(&callbacks, NULL, &delegate));
  textra_paragraph_options_t options = {0};
  options.struct_size = sizeof(options);
  options.type = TEXTRA_PARAGRAPH_TYPE_GENERAL;
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&options, &paragraph));
  OK(textra_paragraph_add_shape_run(paragraph, NULL, delegate, TEXTRA_FALSE));
  textra_run_delegate_destroy(delegate);
  CHECK(destroyed ==
        0);  // The paragraph owns the remaining internal reference.
  textra_paragraph_destroy(paragraph);
  CHECK(destroyed == 1);
  struct {
    uint32_t size;
    uint32_t version;
  } short_table = {sizeof(short_table), 1};
  delegate = (textra_run_delegate_t*)1;
  CHECK(textra_run_delegate_create((const textra_run_callbacks_t*)&short_table,
                                   NULL, &delegate) ==
            TEXTRA_STATUS_INVALID_ARGUMENT &&
        !delegate);
}

#if defined(__APPLE__)
static void test_backend_selection(void) {
  textra_text_layout_options_t options = {0};
  options.struct_size = sizeof(options);
  options.backend = UINT32_MAX;
  textra_text_layout_t* layout = (textra_text_layout_t*)1;
  CHECK(textra_text_layout_create(&options, &layout) ==
            TEXTRA_STATUS_UNSUPPORTED &&
        !layout);
  CHECK(textra_text_layout_preload(TEXTRA_RENDERING_BACKEND_INVALID) ==
        TEXTRA_STATUS_UNSUPPORTED);
  OK(textra_text_layout_preload(TEST_NATIVE_BACKEND));
  OK(textra_text_layout_preload(TEST_SELF_BACKEND));
  textra_font_manager_options_t manager = {0};
  manager.struct_size = sizeof(manager);
  options.backend = TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT;
  options.default_font_manager = &manager;
  CHECK(textra_text_layout_create(&options, &layout) ==
            TEXTRA_STATUS_UNSUPPORTED &&
        !layout);
}
static textra_style_t* make_style(void) {
  textra_typeface_options_t typeface_options = {0};
  typeface_options.struct_size = sizeof(typeface_options);
  typeface_options.backend = TEST_BACKEND;
  CTFontRef native = CTFontCreateWithName(CFSTR("Helvetica"), 20, NULL);
  typeface_options.native_font = native;
  textra_font_descriptor_t* descriptor = NULL;
  OK(textra_font_descriptor_create(&descriptor));
  OK(textra_font_descriptor_set_typeface(descriptor, &typeface_options));
  CFRelease(native);
  textra_style_t* style = NULL;
  OK(textra_style_create(&style));
  OK(textra_style_set_font_descriptor(style, descriptor));
  textra_font_descriptor_destroy(descriptor);
  OK(textra_style_set_text_size(style, 20));
  return style;
}
static void test_layout_and_draw(void) {
  const int previous_destroyed = destroyed;
  textra_font_manager_options_t manager_options = {0};
  manager_options.struct_size = sizeof(manager_options);

  textra_text_layout_options_t engine_options = {0};
  engine_options.struct_size = sizeof(engine_options);
  engine_options.backend = TEST_BACKEND;
  engine_options.default_font_manager = &manager_options;

  textra_text_layout_t* engine = NULL;
  OK(textra_text_layout_create(&engine_options, &engine));
  textra_text_context_t* context = NULL;
  OK(textra_text_context_create(&context));
  textra_paragraph_options_t options = {0};
  options.struct_size = sizeof(options);
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&options, &paragraph));
  // Exercise default font resolution as well as explicit native-font import.
  OK(textra_paragraph_add_text_run(paragraph, NULL, "Default ", 8));
  textra_style_t* style = make_style();
  const char text[] = "Hello world, ABI \xe4\xb8\xad\xe6\x96\x87";
  OK(textra_paragraph_add_text_run(paragraph, style, text,
                                   (uint32_t)strlen(text)));
  textra_style_destroy(style);
  textra_run_callbacks_t delegate_callbacks = {0};
  delegate_callbacks.struct_size = sizeof(delegate_callbacks);
  delegate_callbacks.version = 1;
  delegate_callbacks.measure = measure;
  delegate_callbacks.draw = draw_delegate;
  delegate_callbacks.destroy = destroy_callback;
  textra_run_delegate_t* delegate = NULL;
  OK(textra_run_delegate_create(&delegate_callbacks, NULL, &delegate));
  OK(textra_paragraph_add_shape_run(paragraph, NULL, delegate, TEXTRA_FALSE));
  textra_run_delegate_destroy(delegate);
  // A failed object draw must not make subsequent text receive a NULL painter.
  OK(textra_paragraph_add_text_run(paragraph, NULL, "after delegate", 14));
  textra_layout_region_options_t region_options = {0};
  region_options.struct_size = sizeof(region_options);
  region_options.width = 240;
  region_options.height = 240;
  region_options.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  region_options.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  textra_layout_region_t* region = NULL;
  OK(textra_layout_region_create(&region_options, &region));
  textra_layout_flow_t flow = UINT8_MAX;
  OK(textra_text_layout_layout(engine, paragraph, region, context, &flow));
  CHECK(flow == TEXTRA_LAYOUT_FLOW_NORMAL);
  CHECK(textra_paragraph_add_text_run(paragraph, NULL, "x", 1) ==
        TEXTRA_STATUS_BUSY);
  textra_layout_region_metrics_t metrics = {0};
  metrics.struct_size = sizeof(metrics);
  OK(textra_layout_region_get_metrics(region, &metrics));
  CHECK(metrics.line_count > 0 && metrics.width > 0);
  // A v1 prefix output must not overwrite a smaller caller's allocation.
  struct {
    uint32_t size;
    float width;
    uint32_t canary;
  } prefix = {8, 0, 0xabcdef01};
  OK(textra_layout_region_get_metrics(
      region, (textra_layout_region_metrics_t*)&prefix));
  CHECK(prefix.size == 8 && prefix.width > 0 && prefix.canary == 0xabcdef01);
  textra_text_line_metrics_t line = {0};
  line.struct_size = sizeof(line);
  OK(textra_text_line_get_metrics(region, 0, &line));
  CHECK(line.end_char > line.start_char && line.bottom >= line.top);
  uint32_t hit = 0;
  OK(textra_text_line_hit_test(region, 0, -100, &hit));
  CHECK(hit >= line.start_char && hit <= line.end_char);
  textra_rect_t bounds;
  OK(textra_text_line_get_bounds(region, 0, NULL, TEXTRA_FALSE, &bounds));
  CHECK(bounds.right >= bounds.left);
  textra_text_range_t boundary;
  OK(textra_paragraph_get_grapheme_boundary(paragraph, 1, &boundary));
  CHECK(boundary.start <= 1 && boundary.end > 1);

  unsigned char* pixels = (unsigned char*)calloc(256 * 256, 4);
  CHECK(pixels);
  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
  CGContextRef native =
      CGBitmapContextCreate(pixels, 256, 256, 8, 256 * 4, color_space,
                            kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(color_space);
  CHECK(native);
  CGContextTranslateCTM(native, 0, 256);
  CGContextScaleCTM(native, 1, -1);
  textra_canvas_options_t canvas_options = {0};
  canvas_options.struct_size = sizeof(canvas_options);
  canvas_options.backend = TEST_BACKEND;
  canvas_options.native_canvas = native;
  textra_layout_drawer_options_t drawer_options = {0};
  drawer_options.struct_size = sizeof(drawer_options);
  drawer_options.canvas = &canvas_options;
  textra_layout_drawer_t* drawer = NULL;
  OK(textra_layout_drawer_create(&drawer_options, &drawer));
  textra_paragraph_destroy(paragraph);
  paragraph = NULL;
  textra_text_layout_destroy(engine);
  engine = NULL;
  textra_text_context_destroy(context);
  context = NULL;
  textra_layout_drawer_t* foreign_drawer = NULL;
  canvas_options.backend = TEST_BACKEND == TEST_NATIVE_BACKEND
                               ? TEST_SELF_BACKEND
                               : TEST_NATIVE_BACKEND;
  OK(textra_layout_drawer_create(&drawer_options, &foreign_drawer));
  CHECK(textra_layout_drawer_draw_region(foreign_drawer, region) ==
        TEXTRA_STATUS_BACKEND_MISMATCH);
  textra_layout_drawer_destroy(foreign_drawer);
  OK(textra_layout_drawer_draw_region(drawer, region));
  CHECK(drawn > 0);
  unsigned char* previous = (unsigned char*)malloc(256 * 256 * 4);
  CHECK(previous);
  memcpy(previous, pixels, 256 * 256 * 4);
  int nonzero = 0;
  for (size_t i = 0; i < 256 * 256 * 4; i++) nonzero |= pixels[i];
  CHECK(nonzero);
  memset(pixels, 0, 256 * 256 * 4);
  OK(textra_layout_drawer_draw_region(drawer, region));
  CHECK(!memcmp(previous, pixels, 256 * 256 * 4));
  free(previous);
  reject_draw = 1;
  for (int i = 0; i < 2; ++i) {
    int before = drawn;
    CHECK(textra_layout_drawer_draw_region(drawer, region) ==
          TEXTRA_STATUS_INTERNAL_ERROR);
    CHECK(drawn == before + 1);
  }
  reject_draw = 0;
  OK(textra_layout_drawer_draw_region(drawer, region));
  OK(textra_text_line_get_paragraph(region, 0, &paragraph));
  OK(textra_layout_region_clear(region));
  OK(textra_paragraph_add_text_run(paragraph, NULL, "x", 1));
  textra_paragraph_destroy(paragraph);
  textra_layout_region_destroy(region);
  textra_layout_drawer_destroy(drawer);
  CGContextRelease(native);
  free(pixels);
  CHECK(destroyed == previous_destroyed + 1);
}

static void test_layout_callback_errors(void) {
  textra_text_layout_options_t engine_options = {0};
  engine_options.struct_size = sizeof(engine_options);
  engine_options.backend = TEST_BACKEND;

  textra_text_layout_t* engine = NULL;
  OK(textra_text_layout_create(&engine_options, &engine));
  textra_text_layout_destroy(engine);
  engine = NULL;
  OK(textra_text_layout_create(&engine_options, &engine));
  textra_paragraph_options_t options = {0};
  options.struct_size = sizeof(options);
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&options, &paragraph));
  textra_style_t* style = make_style();
  OK(textra_paragraph_add_text_run(paragraph, style, "callback error", 14));
  textra_style_destroy(style);
  textra_text_context_t* context = NULL;
  OK(textra_text_context_create(&context));
  textra_layout_page_callbacks_t callbacks = {0};
  callbacks.struct_size = sizeof(callbacks);
  callbacks.version = 1;
  callbacks.on_line = reject_line;
  textra_layout_region_options_t region_options = {0};
  region_options.struct_size = sizeof(region_options);
  region_options.width = 200;
  region_options.height = 100;
  region_options.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  region_options.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  region_options.callbacks = &callbacks;
  textra_layout_region_t* region = NULL;
  OK(textra_layout_region_create(&region_options, &region));
  textra_layout_flow_t flow = UINT8_MAX;
  CHECK(textra_text_layout_layout(engine, paragraph, region, context, &flow) ==
        TEXTRA_STATUS_INTERNAL_ERROR);
  CHECK(flow == UINT8_MAX);
  CHECK(textra_text_layout_layout(engine, paragraph, region, context, &flow) ==
        TEXTRA_STATUS_INVALID_STATE);
  OK(textra_layout_region_clear(region));
  OK(textra_text_context_reset(context));
  textra_layout_region_destroy(region);
  callbacks.on_line = NULL;
  callbacks.get_ranges = reject_range;
  OK(textra_layout_region_create(&region_options, &region));
  CHECK(textra_text_layout_layout(engine, paragraph, region, context, &flow) ==
        TEXTRA_STATUS_INVALID_ARGUMENT);
  textra_layout_region_destroy(region);
  textra_text_context_destroy(context);
  textra_paragraph_destroy(paragraph);
  textra_text_layout_destroy(engine);
}
#endif

#if defined(__APPLE__)
static void test_continuation_and_single_line(void) {
  textra_text_layout_options_t engine_options = {0};
  engine_options.struct_size = sizeof(engine_options);
  engine_options.backend = TEST_BACKEND;

  textra_text_layout_t* engine = NULL;
  OK(textra_text_layout_create(&engine_options, &engine));
  textra_text_context_t* context = NULL;
  OK(textra_text_context_create(&context));
  textra_style_t* style = make_style();
  textra_paragraph_options_t options = {0};
  options.struct_size = sizeof(options);
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&options, &paragraph));
  const char* words =
      "first second third fourth fifth sixth seventh eighth ninth tenth "
      "eleventh twelfth";
  OK(textra_paragraph_add_text_run(paragraph, style, words,
                                   (uint32_t)strlen(words)));
  textra_layout_region_options_t region_options = {0};
  region_options.struct_size = sizeof(region_options);
  region_options.width = 85;
  region_options.height = 28;
  region_options.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  region_options.height_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  textra_layout_region_t* regions[32] = {0};
  uint32_t count = 0, last = 0;
  textra_layout_flow_t flow = UINT8_MAX;
  do {
    CHECK(count < 32);
    OK(textra_layout_region_create(&region_options, &regions[count]));
    OK(textra_text_layout_layout(engine, paragraph, regions[count], context,
                                 &flow));
    uint32_t position = 0;
    OK(textra_text_context_get_position(context, &position));
    CHECK(position > last || flow == TEXTRA_LAYOUT_FLOW_NORMAL);
    last = position;
    ++count;
  } while (flow != TEXTRA_LAYOUT_FLOW_NORMAL);
  CHECK(count > 1);
  CHECK(textra_paragraph_add_text_run(paragraph, NULL, "x", 1) ==
        TEXTRA_STATUS_BUSY);
  OK(textra_text_context_reset(context));
  CHECK(textra_text_context_set_feature(context,
                                        TEXTRA_FEATURE_DISABLE_SHAPE_CACHE,
                                        TEXTRA_TRUE) == TEXTRA_STATUS_BUSY);
  for (uint32_t i = 0; i < count; ++i) textra_layout_region_destroy(regions[i]);
  OK(textra_text_context_set_feature(
      context, TEXTRA_FEATURE_DISABLE_SHAPE_CACHE, TEXTRA_TRUE));
  OK(textra_paragraph_add_text_run(paragraph, NULL, "x", 1));
  textra_paragraph_destroy(paragraph);

  options.type = TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE;
  OK(textra_paragraph_create(&options, &paragraph));
  OK(textra_paragraph_add_text_run(paragraph, style, "single", 6));
  CHECK(textra_paragraph_add_text_run(paragraph, style, "second", 6) ==
        TEXTRA_STATUS_UNSUPPORTED);
  textra_paragraph_style_t* unsupported = (textra_paragraph_style_t*)1;
  CHECK(textra_paragraph_get_paragraph_style(paragraph, &unsupported) ==
            TEXTRA_STATUS_UNSUPPORTED &&
        !unsupported);
  OK(textra_text_layout_layout_single_line(engine, paragraph, context));
  textra_paragraph_metrics_t metrics = {0};
  metrics.struct_size = sizeof(metrics);
  OK(textra_paragraph_get_metrics(paragraph, &metrics));
  CHECK(metrics.width > 0 && metrics.height > 0);
  OK(textra_text_context_reset(context));
  CHECK(textra_text_context_set_feature(context,
                                        TEXTRA_FEATURE_DISABLE_SHAPE_CACHE,
                                        TEXTRA_FALSE) == TEXTRA_STATUS_BUSY);
  textra_paragraph_destroy(paragraph);
  OK(textra_text_context_set_feature(
      context, TEXTRA_FEATURE_DISABLE_SHAPE_CACHE, TEXTRA_FALSE));
  textra_style_destroy(style);
  textra_text_context_destroy(context);
  textra_text_layout_destroy(engine);
}
#endif

#if defined(__APPLE__)
#endif
static void test_enum_options(void) {
  // Nonzero padding must not be read as part of a one-byte enum field.
  textra_paragraph_options_t po;
  memset(&po, 0xa5, sizeof(po));
  po.struct_size = sizeof(po);
  po.type = TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE;
  textra_paragraph_t* paragraph = NULL;
  OK(textra_paragraph_create(&po, &paragraph));
  textra_paragraph_type_t type = UINT8_MAX;
  OK(textra_paragraph_get_type(paragraph, &type));
  CHECK(type == TEXTRA_PARAGRAPH_TYPE_SINGLE_LINE);
  textra_paragraph_destroy(paragraph);

  textra_layout_region_options_t ro = {0};
  ro.struct_size = sizeof(ro);
  ro.width = 100;
  ro.height = 200;
  ro.width_mode = TEXTRA_LAYOUT_MODE_DEFINITE;
  ro.height_mode = TEXTRA_LAYOUT_MODE_AT_MOST;
  textra_layout_region_t* region = NULL;
  OK(textra_layout_region_create(&ro, &region));
  textra_layout_region_destroy(region);

  textra_gradient_stop_t stops[] = {{0xffff0000, 0}, {0xff0000ff, 1}};
  textra_resolved_gradient_options_t go;
  memset(&go, 0xa5, sizeof(go));
  go.struct_size = sizeof(go);
  go.start = (textra_point_t){0, 0};
  go.end = (textra_point_t){100, 100};
  go.center = (textra_point_t){50, 50};
  go.radius_x = go.radius_y = 50;
  go.start_angle = 0;
  go.stops = stops;
  go.stop_count = 2;
  for (textra_gradient_type_t kind = TEXTRA_GRADIENT_TYPE_LINEAR;
       kind <= TEXTRA_GRADIENT_TYPE_CONIC; ++kind) {
    go.type = kind;
    textra_resolved_gradient_t* gradient = NULL;
    OK(textra_resolved_gradient_create(&go, &gradient));
    CHECK(gradient);
    textra_resolved_gradient_destroy(gradient);
  }
  textra_resolved_gradient_t* gradient = NULL;
  go.type = 0;
  CHECK(textra_resolved_gradient_create(&go, &gradient) ==
        TEXTRA_STATUS_UNSUPPORTED);
  CHECK(!gradient);
}

int main(void) {
  test_self_rendering_backend();
#if defined(__APPLE__)

#endif
  test_values();
  test_font_features();
  test_bounded_utf8_input();
  test_enum_options();
  test_callbacks();
#if defined(__APPLE__)
  test_backend_selection();
  for (int i = 0; i < 2; ++i) {
    test_backend = i == 0 ? TEST_NATIVE_BACKEND : TEST_SELF_BACKEND;
    test_layout_and_draw();
    test_layout_callback_errors();
    test_continuation_and_single_line();
  }
#endif
#if defined(__APPLE__)

#endif
  puts("Textra C API tests passed");
  return 0;
}
