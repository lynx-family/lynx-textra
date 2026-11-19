# C API validation

This directory contains the C API unit tests, rendering comparison helpers,
Android font checks and performance tooling. The demonstration renderers and
application entry points remain in `demos/`; renderer adapters remain in
`examples/capi/platform/`.

## Unit tests

Build from the repository root using a configured GN output directory:

```sh
buildtools/ninja/ninja -C out/capi-validation \
  test:capi_test test:backend_test test:backend_error_test test:font_features_test
(cd out/capi-validation && ./capi_test && ./backend_test && ./backend_error_test && ./font_features_test)
```

`backend_error_test` requires macOS/CoreText. `backend_test` currently uses
SKITY_CORETEXT for manager ownership checks and requires CoreText shaping support.
Platform behavior and compiled-backend requirements are unchanged. Test targets
are defined in `test/BUILD.gn`, not the production `src/BUILD.gn`.

## Rendering comparisons

`ios_comparison.hpp`, `mac_comparison.hpp` and `skity_comparison.hpp` compare
C++/C layout metrics, pixels and repeated draws. Demo comparison modes use these
helpers. `skity_compare.cc` is the standalone Skity comparison entry point.
`verify_results.py` independently checks exported reports and images.

See the [demo integration guide](/demos/capi/README.md) for platform commands.

## Android checks and benchmarks

- `android/com/lynx/textra/demo/CapiFontArchitectureTest.java`: font assertions,
  included by the demo Gradle source set and focused APK builder.
- `android_shaping_benchmark.cc`: test-only JNI shaping/architecture checks.
- `build_android_shaping_benchmark.py`: links a separate test core copy.
- `build_android_demo.py`: packages the existing demo with its test helpers.
- `run_android_shaping_benchmark.py` and `summarize_android_shaping.py`: benchmark
  execution and result processing.

All Python entry points run from any working directory when their arguments use
absolute paths. They discover the repository from their own location. Moving the
files does not change benchmark methodology or existing report versions.

`font_features_test` compares C/C++ pixels, metrics and repeated rendering for
OpenType feature lists using both native and self-rendering CoreText backends,
for general and single-line paragraphs. It alternates settings on the same
engines to exercise cache isolation and verifies that the Baskerville `liga`
switch actually changes output. Other tags remain backend/font dependent.
The pure C test covers copying, clearing, bounds, invalid tags and style snapshots.
