# Skity release SDKs

Tests and demos consume **Skity v1.0.0** release binaries. `skity.json` pins
both platforms' URLs and SHA-256 digests. No Skity sources or codec dependencies
are fetched or compiled by Textra. Updating this lock is an explicit SDK upgrade.

Run from the repository root after the usual core dependency setup:

```sh
# macOS software-rendering tests and Metal/GL demos
./tools/hab sync . --target-only --target skity-macos
# Android demos (also required before configuring the Android Gradle project)
./tools/hab sync . --target-only --target skity-android
```

Habitat verifies each archive and uses its existing HTTP cache. CI/CQ uses these
same commands. Missing SDKs cause a build configuration error; there is no
source-build fallback. The third_party/skity_release directory is ignored.

The Apple XCFramework includes arm64/x86_64 macOS and iOS device/simulator
slices. Its macOS binaries require macOS 15.0. This deployment requirement is
applied to Skity consumers, not the standalone Textra core. CMake imports the
framework through cmake/SkityRelease.cmake. Test/demo PNG output uses ImageIO;
the release framework does not include Skity's optional codec module.

Android uses the release AAR's Prefab skity-native::skity target. v1.0.0 uses
c++_shared and includes armeabi-v7a, arm64-v8a, x86 and x86_64. Keep the demo's
STL setting compatible with that SDK.
