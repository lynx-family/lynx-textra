# Skity with CoreText

Use `TEXTRA_RENDERING_BACKEND_SKITY_CORETEXT` for CoreText shaping and Skity
rendering on iOS or macOS. Both platforms use the same C interface:

```c
#include <examples/capi/platform/skity_coretext/textra_skity_coretext_backend.h>

/* Initialize options and check each returned status:
 * textra_skity_coretext_canvas_callbacks_initialize(&canvas_options);
 * textra_skity_coretext_font_manager_callbacks_initialize(&manager_options);
 * textra_skity_coretext_typeface_callbacks_initialize(&typeface_options);
 */
```

Link `textra_capi_skity_coretext`, or compile only
`textra_skity_coretext_backend.cc` once in the host adapter target. It implements
its own Canvas, FontManager and Typeface callbacks and does not depend on
`textra_capi_skity`. No implementation or source-level platform macro is needed.
Link a host Skity SDK built with CoreText support and the CoreText framework.
The Textra core contains the shaper and has no Skity link dependency.

Canvas is a borrowed `skity::Canvas*`. The manager input is an optional borrowed
`skity::FontManager*`; native font input/output is `CTFontRef`. Typeface adapters
retain imported fonts. Initialize matching layout font-source, Canvas and
Typeface options. Ownership transfers only on successful owner initialization.
See the [host build guide](/examples/capi/platform/README.md) and
[C API guide](/public/textra/capi/README.md).
