# Private host rendering adapters

These SDK adapters are shipped under `public/textra/capi/platform` as private
headers, outside the exported core C ABI. Hosts include them explicitly and
supply the matching renderer SDK.

All adapters are header-only: Skia requires C++17 and its C++ SDK; Skity uses
C11 and the Skity C API SDK. Include the corresponding umbrella and link the
Textra core and renderer SDK. No adapter source, implementation macro or adapter
archive is required. Initializers have internal linkage and the implementation
headers support inclusion from multiple translation units.

| Rendering backend | Optional build target | Header/source |
| --- | --- | --- |
| Skity | `textra_capi_skity` | `textra_capi_skity.h` |
| iOS/macOS Skity + CoreText | `textra_capi_skity_coretext` | `textra_capi_skity_coretext.h` |
| Skia | `textra_capi_skia` | `textra_capi_skia.h` (C++17 SDK) |

The two Skity headers share pure C implementations in `skity_common/`:

- `textra_skity_canvas_impl.h`: canvas callbacks, paths and drawing.
- `textra_skity_paint_impl.h`: paints, gradients and fill/stroke passes.
- `textra_skity_typeface_impl.h`: typeface lifetime, glyphs, metrics and font data.
- `textra_skity_font_manager_impl.h`: manager lifetime and font matching/loading.
- `textra_skity_common.h`: shared SDK includes and checked allocation.

Keep that directory alongside the backend headers when copying them into a host.
They depend only on the public Textra C API, the Skity C API and the C standard
library. The Textra core does not compile these adapters or link renderer SDKs.
Skia's umbrella includes its three `static inline` C-style initializers.
Private C++ headers
separate canvas callbacks, paints/gradients, typeface metrics/data and font
manager matching, with each callback table next to its implementation. C entry
points keep the C naming convention; C++ helpers follow Google C++ naming.
`skia/textra_skia_support.hpp` holds Skia-only gradient data and callback
initialization helpers in the `textra_native::skia_backend` namespace.

## C naming conventions

C functions, callback fields, parameters and internal constants use `snake_case`.
Textra symbols carry a `textra_` prefix; Skity implementation helpers use
`textra_skity_impl_`. Struct tags use `snake_case` and typedef names end in `_t`.
Macros and header guards use uppercase names with underscores. These conventions
also apply when the C headers are included from C++; C++ implementation code
follows the repository's Google C++ style.

## Direct use

Use one umbrella header: `<textra/capi/platform/textra_capi_skia.h>`,
`<textra/capi/platform/textra_capi_skity.h>` or `<textra/capi/platform/textra_capi_skity_coretext.h>`.
The Skia umbrella includes C++17 implementation headers and requires the Skia
SDK. It cannot be included from pure C because the SDK itself is C++. Skity
umbrellas work in both C11 and C++17 and require the Skity C SDK. All three
remain private headers; the exported core C ABI headers remain SDK-independent.

Add Textra's `public` and the Skity SDK to the include paths:

```c
#include <textra/capi/platform/textra_capi_skity.h>

/* Set struct_size, native inputs and userdata, then check each status:
 * textra_skity_canvas_callbacks_initialize(&canvas_options);
 * textra_skity_font_manager_callbacks_initialize(&manager_options);
 * textra_skity_typeface_callbacks_initialize(&typeface_options);
 */
```

Canvas handles are borrowed. An optional input manager is borrowed; a default
manager created by the adapter is owned. Imported typefaces are retained.
Adopt owned records once into an engine owner, or release them through their
destroy callback. Failed initialization leaves options unchanged. Keep callback
code, userdata and borrowed managers alive through dependent objects/caches.

Skity alpha.7 lacks the reverse `skity_typeface` to `CTFontRef` bridge. The
CoreText header therefore provides Canvas initialization, while manager and
typeface initialization return `TEXTRA_STATUS_UNSUPPORTED`.

## Build targets

CMake hosts may provide `skity` and/or `skia` SDK targets (including their
include/link requirements), then add this directory:

```cmake
add_subdirectory(${TEXTRA_SOURCE_ROOT}/public/textra/capi/platform textra_host_adapters)
target_link_libraries(my_host PRIVATE textra_capi_skity)
```

All CMake adapter targets are `INTERFACE` libraries carrying headers and SDK
requirements; the CoreText target is available on Apple platforms. The Skia
target also supplies C++17 and the existing no-exceptions compiler setting. Link the Textra core
separately. A static Skity binary may require the C++ linker/runtime even when
the application's adapter code is compiled as C.

GN hosts may depend on the equivalent targets in `//public/textra/capi/platform`.
The host supplies the binary SDK's include paths and libraries. No Skity source
checkout is required.
