# Skity host adapter

Use `TEXTRA_RENDERING_BACKEND_SKITY` for Skity rendering with HarfBuzz shaping.
Include `examples/capi/platform/skity/textra_skity_backend.h` and link
`textra_capi_skity`, or compile `textra_skity_backend.cc` once in the host.
No implementation macro is needed. The host links its Skity SDK and the Textra
core separately.

Canvas is a borrowed `skity::Canvas*`. The optional manager input is a borrowed
`skity::FontManager*`; native font input is `skity::Typeface*`. Initialize the
options and adopt owned records once into an engine owner, or destroy them if
initialization of that owner fails. Keep callback code and userdata alive through
all dependent results/caches.

This adapter has its own callbacks and resource wrappers. Skity with CoreText
shaping is a separate platform under
[`skity_coretext/`](/examples/capi/platform/skity_coretext/README.md).
