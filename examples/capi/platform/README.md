# Host rendering adapters

The Skia/Skity adapter headers contain pure C declarations. Hosts include them
normally; there are no implementation macros. Compile each adapter once, or link
its optional library. The Textra core neither compiles these adapters nor links
the renderer SDKs.

| Rendering backend | Adapter target | Source files under this directory |
| --- | --- | --- |
| Skia | `textra_capi_skia` | `skia/textra_skia_backend.cc` |
| Skity | `textra_capi_skity` | `skity/textra_skity_backend.cc` |
| iOS/macOS Skity + CoreText | `textra_capi_skity_coretext` | `skity_coretext/textra_skity_coretext_backend.cc` |

Skity and Skity+CoreText are independent platform adapters. Each target compiles
its own callbacks and links the host Skity SDK directly; neither links the other.
Other `*_impl.hpp` files are private implementation dependencies. Consumers
include the corresponding platform's `*_backend.h` only.
All adapters use C++17 and compile without C++ exceptions. Tables/ownership and
initialization behavior are unchanged.

## CMake

First provide `skia` and/or `skity` library targets, including SDK include paths
and required link dependencies (imported targets are supported). Then add this
directory to the host build:

```cmake
add_subdirectory(${TEXTRA_SOURCE_ROOT}/examples/capi/platform textra_host_adapters)
target_link_libraries(my_host PRIVATE textra_capi_skity)
```

Targets are created for SDK targets present when this directory is added. Apple
builds provide the same `textra_capi_skity_coretext` adapter target on iOS and
macOS. Link it instead of the base target when using CoreText shaping.
This directory builds adapter libraries only; link the Textra core separately.
The host's Skity build must enable CoreText for the CoreText adapter.

## GN

Use `//examples/capi/platform:textra_capi_skia` or
`//examples/capi/platform:textra_capi_skity`, or `//examples/capi/platform:textra_capi_skity_coretext`. The host links its matching renderer SDK. GN uses the repository's
`third_party/skia` and `third_party/skity/include` header roots.

## Direct compilation

Add the repository root and `public` to the include paths, add the SDK's include
paths, and compile the source files listed above into a host library. Link the
renderer SDK and, for CoreText adapter, the CoreText framework. The application
can then use these C entry points without SDK dependencies in its C headers:

```c
#include <examples/capi/platform/skity_coretext/textra_skity_coretext_backend.h>

/* Initialize options and check the returned status. */
/* textra_skity_coretext_canvas_callbacks_initialize(&canvas_options); */
```
