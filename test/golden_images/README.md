# Skity release baseline

These 54 PNGs were regenerated with the immutable **Skity v1.0.0 Apple
XCFramework** pinned in `hab.ci/skity.json`. Tests use its software
canvas; PNG encoding uses system ImageIO because the release has no codec module.
The artifact is built with the CoreText font backend, whereas the previous source
build used FreeType. This changes glyph metrics, raster edges, and some wrapping
and alignment positions. These baselines intentionally capture the release SDK.

All 54 old/new/difference pairs were visually reviewed before replacement. Image
dimensions remain unchanged. Backgrounds, inline boxes, text colors, decoration,
shadows, bidi text and clipping remain represented. Changes include glyph-edge
coverage and font extents, with changed line breaks in `layout_mode2`,
`line_spacing`, and `word_break_test1/2`; these are not pixel-identical to the
previous source SDK. The long repeated-text `one_string` case has the largest
changed-pixel fraction (about 32.25%). Comparison thresholds were not relaxed.

Baseline generation environment: macOS 27.2.1 arm64, Apple SDK 26.5, Release build.
All 310 unit tests passed. The font-collection case explicitly uses the
repository's NotoSansCJK and Inter fixtures and checks that installing an asset
manager selects a different, non-null face. It no longer uses the Menlo variable
font alias or the unresolved, misspelled Helvetica lookup.
Runtime baselines on the CI runner remain subject to CI validation.

Reproduce from the repository root:

```sh
./tools/hab sync . --target-only --target skity-macos
buildtools/cmake/bin/cmake -S . -B cmake-build-test -G Ninja -DCMAKE_BUILD_TYPE=Release
buildtools/cmake/bin/cmake --build cmake-build-test --target TextLayoutLiteTest
mkdir -p golden_check
(cd golden_check && ../cmake-build-test/test/TextLayoutLiteTest)
python3 test/compare_images.py --actual-dir=golden_check --golden-dir=test/golden_images
```
