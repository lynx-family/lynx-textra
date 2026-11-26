# Rendering backend contract

C and C++ share the numeric identifiers in `textra/rendering_backend.h`.
`RenderingBackend` selects the shaping pipeline and identifies the expected
font/canvas compatibility contract. PlatformHelper constructs built-in native instances or per-instance callback
adapters. Zero in construction options selects the compiled native default;
explicit backend enum values remain concrete platform identifiers.

| C constant suffix / C++ value | Canvas / font adapters | Shaping implementation |
| --- | --- | --- |
| `ANDROID` / `kAndroid` | Java | JavaShaper |
| `IOS` / `kIOS` | CoreGraphics / CoreText | ShaperCoreText |
| `MACOS` / `kMacOS` | CoreGraphics / CoreText | ShaperCoreText |
| `OHOS` / `kOHOS` | ArkGraphics | ShaperArkGraphics |
| `LVGL` / `kLVGL` | LVGL | LVGLShaper |
| `SKIA` / `kSkia` | Skia | ShaperSkShaper / HarfBuzz |
| `SKITY` / `kSkity` | Skity | ShaperSkShaper / HarfBuzz |
| `IOS_SELF_RENDERING` / `kIOSSelfRendering` | CoreGraphics / CoreText | ShaperCoreTextSelfRendering |
| `MACOS_SELF_RENDERING` / `kMacOSSelfRendering` | CoreGraphics / CoreText | ShaperCoreTextSelfRendering |
| `SKITY_CORETEXT` / `kSkityCoreText` | Skity / SkityFontManagerCoreText | ShaperCoreTextSelfRendering |

C constants have the prefix `TEXTRA_RENDERING_BACKEND_`. All values are explicit
32-bit integers. C++ enum values reference those same definitions. Other C enum-like
types use the corresponding C++ enum's width and numeric values (typically
`uint8_t`). They remain C11-compatible integer typedefs with explicit constants;
the implementation checks every shared value and width at compile time.
Published widths and numbers must not be changed or reused.

Set `backend` explicitly for self-rendering. Native construction and layout
options may use zero to select the compiled native platform. The four font sources and the drawing canvas must
match the layout backend exactly, including the native/self-rendering variants.
Mismatches return `TEXTRA_STATUS_BACKEND_MISMATCH`; an unbuilt combination returns
`TEXTRA_STATUS_UNSUPPORTED`. For C++, keep the existing I* interfaces and smart
pointer ownership, and construct matching objects through PlatformHelper.
C++ callers supplying custom I* implementations are responsible for compatibility.

`PlatformHelper::IsBackendAvailable` checks compiled capabilities;
`TextLayout::Preload` additionally checks runtime shaping dependencies. Skia and
Skity do not silently switch to CoreText or platform shaping if HarfBuzz is
unavailable. Shaping cache entries are separated by rendering backend.

## Self-rendering selection (ABI 1.1)

`textra_get_self_rendering_backend(preferred)` is the C counterpart of
`PlatformHelper::GetSelfRenderingBackend`. Pass `SKIA` or `SKITY` to prefer
SkShaper when compiled, with CoreText self-rendering as the fallback. Other
inputs and builds with neither implementation return `INVALID`. This query
checks compiled capabilities, not runtime HarfBuzz availability. Hosts supply
compatible drawing/font adapters, including CoreText font access for the
fallback, and use the returned backend consistently across layout and adapters.
Explicit backend factories do not apply this selection automatically.

## PlatformHelper factories (ABI 1.0)

The original C++ `PlatformHelper` provides `CreateFontManager`,
`CreateCanvasHelper` and `CreateTypefaceHelper`. Their default overloads select
Android, iOS, macOS, OHOS or LVGL according to the compiled backend. They return the
existing I* smart pointers and reuse the original platform implementations.
Native platform implementation headers now live privately under
`src/ports/native/{ios,mac,java,ark_graphics,lvgl}` and are not installed. C++
consumers include `textra/platform_helper.h` and use I* smart pointers.
Explicit unavailable platforms return null. Options overloads accept the same
pure C callback records used by the C API.

C API initialization uses a single set of owner operations:

- `textra_text_layout_create` takes up to four `textra_font_manager_options_t` records.
- `textra_font_descriptor_set_typeface` takes `textra_typeface_options_t`.
- `textra_layout_drawer_create` takes `textra_canvas_options_t` through `canvas`.

In those records, null callback tables select the built-in native implementation.
Set backend to zero for the compiled platform or a concrete native backend.
For SKIA, SKITY or SKITY_CORETEXT, set the explicit backend and supply the
matching callback tables and opaque objects. Unsupported combinations return
`TEXTRA_STATUS_UNSUPPORTED`. There is no provider registration or type registry.
Generic I* create/destroy operations remain private.

Native layouts can omit all font sources; the engine installs a default native
manager. Callback rendering backends require an explicit default manager.
Optional sources are searched dynamic > asset > test > default. All sources use
the layout backend; manager records have no backend or flags. Default family and
fallback policy are configured on the layout, which also exposes font-cache
clearing. No FontCollection handle is public.
Skia/Skity SDK headers and libraries remain host dependencies.

Android initialization calls `textra_android_runtime_initialize(env)` from an
application JNI context. This delegates to the original TTText initialization.
Its canvas adapter binds the existing JavaCanvasHelper drawing buffer to the
supplied target and replays through the original Java decoder. No second Java
font manager or C-only drawing protocol is needed.

Native canvas input is Android Canvas jobject (retained), CGContextRef (borrowed)
or OH_Drawing_Canvas* / LVGL lv_layer_t* (borrowed). CoreText retains imported CTFontRef; Android
registers imported android.graphics.Typeface jobject with the original manager.
OHOS preserves the original borrowed OH_Drawing_Font contract: keep that font
alive until all dependent styles, layouts and caches have been released.

ABI 1.0 uses layout font-source options rather than public collection handles.
Backend numbers and callback table layouts are unchanged.
All public C headers compile as C11 without platform SDK dependencies.

```c
textra_font_manager_options_t manager = {0};
manager.struct_size = sizeof(manager);
textra_text_layout_options_t layout_options = {0};
layout_options.struct_size = sizeof(layout_options);
layout_options.default_font_manager = &manager;
textra_text_layout_t* layout = NULL;
textra_status_t status = textra_text_layout_create(&layout_options, &layout);
/* Check status before use. */
textra_canvas_options_t canvas = {0};
canvas.struct_size = sizeof(canvas);
canvas.native_canvas = cg_context;
textra_layout_drawer_options_t options = {0};
options.struct_size = sizeof(options);
options.canvas = &canvas;
textra_layout_drawer_t* drawer = NULL;
status = textra_layout_drawer_create(&options, &drawer);
/* Check status before use. CGContext remains borrowed until drawer destruction. */
textra_layout_drawer_destroy(drawer);
textra_text_layout_destroy(layout);
```

## OpenType font features (ABI 1.0)

`textra_font_descriptor_set_font_features` replaces the descriptor's feature
list with a deep copy. A feature contains exactly four printable ASCII tag
bytes and an `int32_t` value; tags are not NUL-terminated. Order, duplicate tags
and signed values follow C++ `FontDescriptor::font_feature_settings_` unchanged.
The frozen eight-byte element layout will not be extended in place.

```c
textra_font_feature_t features[] = {
    {{'l', 'i', 'g', 'a'}, 0},
    {{'c', 'v', '0', '1'}, 2},
};
textra_font_descriptor_set_font_features(descriptor, features, 2);
textra_style_set_font_descriptor(style, descriptor);

uint32_t count = 0;
textra_font_descriptor_get_font_feature_count(descriptor, &count);
textra_font_feature_t feature;
textra_font_descriptor_get_font_feature(descriptor, 0, &feature);

/* Restore defaults; apply the descriptor again to update an existing style. */
textra_font_descriptor_set_font_features(descriptor, NULL, 0);
textra_style_set_font_descriptor(style, descriptor);
```

Check each returned status in application code. Invalid tags or NULL with a
nonzero count leave the descriptor unchanged; an invalid read index returns
`OUT_OF_RANGE` without writing the output. Clones and styles retain independent
copies. Setting a style snapshots the descriptor, so later descriptor edits do
not mutate existing paragraphs or layout results.

Settings use the same font matching and shaping cache keys as C++. Actual tag
support depends on the selected backend and font. CoreText, Android Java and
OHOS forward settings through their native paths; the current HarfBuzz and LVGL
paths do not consume this list, matching their C++ behavior. Unsupported tags
are not emulated by the C wrapper.

## ABI and ownership

All three callback tables use their named `TEXTRA_*_API_VERSION` constants (2).
Initializers copy complete known fields within `struct_size` and ignore unknown
trailing fields. Callers supply the current table version and all required
callbacks; the object adapters do not validate those inputs. Tables can be
stack-local. Canvas callbacks may be NULL (no-op). FontManager and Typeface entries are required,
except `em_height`, which defaults to font metrics.
Append optional callback fields without changing published field signatures.
Drawing views, array elements and construction options are frozen records; use
new record types/functions for incompatible extensions.

On successful layout creation, one reference per configured manager is adopted;
on failure none are adopted, even if another slot was already initialized. Reusing
an owned manager across layouts or slots requires separately acquired references.
Configuration records and callback tables may be released after creation, but
userdata and callback code must outlive every dependent result/cache.
On successful construction, the adapter owns the opaque object and calls its
destroy callback exactly once after its final reference is released.
Reference increments are internal: layouts, descriptors, paragraphs and
layout results retain their dependencies automatically. No public retain
functions are declared or exported. Each returned owned handle is released
once by its caller; internal dependencies may extend the object lifetime. Failure
leaves the object with the caller. A matching callback returns one owned native
face, or NULL for no match. The face is released through the copied Typeface
destroy callback. Font bytes and native font outputs are borrowed until the
face is destroyed; Canvas paint/path/text/array inputs are call-borrowed.
Inline-object drawing uses `textra_run_callbacks_t.draw`; Canvas does not expose
separate run-delegate or background-delegate callbacks.

Userdata is borrowed. The host must keep it and the callback module alive for
all dependent objects, including fonts retained by layout results and shape
caches. Clear caches when releasing the final host resources. There is no
implicit global userdata ownership or unregister-time cleanup. Mutable native
instances require external synchronization. Callbacks are synchronous and must
not throw C++ exceptions.

Canvas, FontManager and Typeface have no standalone C drawing/query/matching
operations. The host supplies their callbacks and uses LayoutDrawer to render.
The engine calls the I* adapters, which invoke the supplied callbacks. Font
metrics/em-height and data caches, font style and engine-assigned IDs retain
the existing I* behavior. Callback failures are not translated into a separate
C error channel. Platform factory initialization failures return their status.
Existing unsupported platform methods remain unsupported.

Callback rendering backends require a compatible `default_font_manager` in the
layout options; optional sources are never silently promoted to the default.
Native layouts construct their default manager through PlatformHelper. Callers
provide valid construction options and compatible callback objects/SDK builds.

## Host self-render callback examples

`examples/capi/platform/skia` and `skity` contain SDK callback examples for host
compilation. The separate Skity+CoreText directory supports both iOS and macOS;
LVGL uses its original C++ implementations inside the engine. There are no duplicate native-platform C
canvas/font/typeface implementations. The core does not link Skia or Skity.

Host `*_callbacks_initialize` adapters fill callback records without creating
engine I* handles. Headers contain only C declarations; compile/link the matching
adapter target, with no implementation macro. CMake/GN targets and direct-source
integration are described in the [host build guide](/examples/capi/platform/README.md).
Returned tables have static lifetime. Before successful owner
initialization, the host owns the native wrapper and releases it on failure.
The standalone self-render demo borrows its records and clears shaped-font
caches before releasing the native objects.

SKITY_CORETEXT supplies matching font and CTFont conversion callbacks and uses
the built-in CoreText self-render shaper. Ordinary SKIA/SKITY use HarfBuzz and
require the font data, glyph and metrics callbacks needed by that shaper.
SDK dependencies and required graphics libraries belong to the host.

Skity Canvas initializers (`textra_skity_canvas_callbacks_initialize` and the
`textra_skity_coretext_` variant) update one options record in
place. Set `struct_size` and `native_canvas` to the borrowed `skity::Canvas*`;
the initializer fills its fixed backend and callback table, preserves userdata
and the native pointer, and allocates no wrapper. Canvas destruction is a no-op;
the host keeps the native Canvas alive until all drawers using it are destroyed.
All platform initializers now take one mutable options record. Set struct_size,
the native input and optional userdata; the function fills its fixed backend
and callback tables. Failure leaves the input unchanged. Initializers that own
state or references reject a non-null callbacks field to prevent initializing
an owned record twice. FontManager inputs also require typeface_callbacks=NULL.

Skia Canvas also borrows the original pointer without a wrapper. Skia FontManager
uses a direct SkFontMgr reference (ref at initialization, unref at destruction).
Skity managers and typefaces retain their state objects. Successful initialization writes the owned reference into native_*;
adopt that record once into an engine owner, or invoke its destroy callback
if adoption fails. Do not release an adopted record manually. The core C create
functions and their ABI are unchanged. Recompile host adapters for the new
single-argument initializer signatures.

```cpp
textra_canvas_options_t canvas{};
canvas.struct_size = sizeof(canvas);
canvas.native_canvas = native_canvas;
auto status = textra_skity_canvas_callbacks_initialize(&canvas);
```

## Built-in LVGL backend

Compile LVGL shaping/native adaptation with `enable_lvglshaper=true` (enabled
by default on the LVGL target). Its default backend is LVGL; C++ uses the three
PlatformHelper factories and C uses the existing owner initialization calls
with backend=0 and callbacks=NULL. Pass a borrowed lv_layer_t* as native_canvas
and a borrowed lv_font_t* as native_font. Fonts must outlive all dependent
styles/layouts and shaped-font caches. Native font descriptors keep an internal
Typeface token; do not reinterpret lv_font_t* as an ITypefaceHelper*.

The original LVGL C++ drawing/font behavior is retained, including existing
unsupported methods. Default font selection follows the host LVGL build's
Montserrat/CJK/default-font configuration. Native font import wraps the given
font without copying or taking ownership. The repository's duplicate LVGL C
callback example is removed; custom complete callback pipelines remain supported. A custom LVGL pipeline
must supply matching Canvas/FontManager/Typeface callbacks together; do not mix
its callback resources with the built-in native instances.

The host supplies LVGL, its font resources and the same lv_conf.h to both the
engine and application. The native C++ implementation requires LV_USE_MATRIX=1,
LV_USE_FLOAT=1 and LV_USE_VECTOR_GRAPHIC=1. The host initializes LVGL, begins and
finishes the canvas layer and dispatches its draw tasks. Engine factory/adaptor
destruction does not destroy the LVGL layer or font, or shut down LVGL.

LVGL follows the existing backend build organization: `lvgl_public` supplies
its backend macro and conventional third_party include root; `native_platform`
compiles the adapter only when LVGL is enabled. The host build supplies the
matching LVGL configuration and SDK linkage through its normal target settings.
`demos/lvgl` uses `target_link_libraries(lynxtextra PUBLIC lvgl)`. No additional
LVGL SDK/configuration arguments are introduced in declare_args. The LVGL vector
feature requires one of its supported renderers; the repository LVGL demo
enables ThorVG. These settings do not compile another LVGL runtime.

iOS and macOS share `skity_coretext/textra_skity_coretext_backend.h/.cc` and the
`textra_skity_coretext_*_callbacks_initialize` entry points. The optional host
target is `textra_capi_skity_coretext`, with no source-level platform macros.
This adapter has its own callbacks and resource wrappers; it does not link the
ordinary Skity adapter. SDK conversion/initialization remains host-compiled and
is not linked into Textra. Owner initialization uses the public options APIs; legacy internal
handle-binding/getter APIs and platform-specific test construction shims have
been removed. Internal references adopt newly created resources directly.
