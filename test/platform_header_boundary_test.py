#!/usr/bin/env python3
"""Host adapter headers stay private; renderer SDKs stay outside core."""
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
class PlatformHeaderBoundaryTest(unittest.TestCase):
    def test_host_examples(self):
        self.assertTrue((ROOT / "public/textra/platform/skity/skity_canvas_helper.h").is_file())
        self.assertFalse(list((ROOT / "src/ports/platform").rglob("*.h")))
        self.assertIn("public/textra/platform/**/*.h", (ROOT / "LynxTextra.podspec").read_text())
    def test_native_platforms_are_private(self):
        platforms = ("ios", "mac", "java", "ark_graphics", "lvgl")
        for platform in platforms:
            self.assertFalse((ROOT / "public/textra/platform" / platform).exists())
            self.assertTrue((ROOT / "src/ports/native" / platform).is_dir())
        for root in (ROOT / "public", ROOT / "demos", ROOT / "platform/harmony"):
            for path in root.rglob("*"):
                if path.suffix not in (".h", ".hpp", ".cc", ".mm") or not path.is_file():
                    continue
                text = path.read_text()
                self.assertNotIn('include "src/ports/native/', text, str(path))
                self.assertNotIn('include <src/ports/native/', text, str(path))
                for platform in platforms:
                    self.assertNotIn('include <textra/platform/' + platform + '/', text, str(path))
        spec = (ROOT / "LynxTextra.podspec").read_text()
        public = spec.split("sp.public_header_files", 1)[1].split("sp.private_header_files", 1)[0]
        self.assertNotIn("src/ports/native", public)
        self.assertIn('"src/ports/native/ios/*.h"', spec)

    def test_core_boundary(self):
        paths = list((ROOT / "src/capi").glob("*.cc")) + [ROOT / "src/ports/platform_helper.cc"]
        for path in paths:
            self.assertNotIn("#include <textra/platform/", path.read_text(), str(path))
            self.assertNotIn("#include <textra/capi/platform/", path.read_text(), str(path))
        cmake = (ROOT / "CMakeLists.txt").read_text()
        self.assertNotIn("TEXTRA_ENABLE_SKIA_BACKEND", cmake)
        self.assertNotIn("find_library", cmake)
    def test_native_c_backend_boundary(self):
        self.assertTrue((ROOT / "public/textra/capi/platform").is_dir())
        self.assertFalse((ROOT / "examples/capi/platform").exists())
        spec = (ROOT / "LynxTextra.podspec").read_text()
        public = spec.split("sp.public_header_files", 1)[1].split("sp.private_header_files", 1)[0]
        private = spec.split("sp.private_header_files", 1)[1].split("sp.source_files", 1)[0]
        self.assertNotIn("capi/platform", public)
        self.assertIn('"public/textra/capi/platform/**/*.{h,hpp}"', private)
        self.assertIn('"public/textra/capi/platform/**/*.{h,hpp}"',
                      spec.split("sp.source_files", 1)[1])
        root = ROOT / "public/textra/capi/platform"
        self.assertFalse((ROOT / "public/textra/platform/capi").exists())
        for path in list(root.rglob("*.hpp")) + list(root.rglob("*.h")):
            text = path.read_text()
            self.assertNotIn("#include <textra/platform/", text, str(path))
            self.assertNotIn("#include <textra/i_", text, str(path))
            for name in ("ICanvasHelper", "IFontManager", "ITypefaceHelper", "BackendFactories"):
                self.assertNotIn(name, text, str(path))

    def test_independent_skity_platforms(self):
        root = ROOT / "public/textra/capi/platform"
        for platform in ("apple", "ios", "mac", "android", "ohos", "lvgl"):
            self.assertFalse((root / platform).exists())
        adapter = root / "skity_coretext/textra_skity_coretext_backend"
        path = adapter.with_suffix(".h")
        self.assertTrue(path.is_file())
        self.assertNotRegex(path.read_text(),
                            r"TargetConditionals|TARGET_OS_|TTTEXT_OS_|__APPLE__")
        self.assertFalse((root / "skity/textra_skity_coretext_impl.hpp").exists())
        self.assertFalse((root / "skity/textra_skity_backend_internal.h").exists())
        for platform, other in (("skity", "skity_coretext"),
                                ("skity_coretext", "skity")):
            for path in (root / platform).glob("*"):
                if path.suffix in (".cc", ".h", ".hpp"):
                    self.assertNotIn("platform/" + other + "/", path.read_text())

    def test_header_only_backends(self):
        root = ROOT / "public/textra/capi/platform"
        self.assertFalse(list(root.rglob("*.cc")))
        for path in root.rglob("*_backend.h"):
            self.assertNotIn("BACKEND_IMPLEMENTATION", path.read_text())
            self.assertIn("static inline", path.read_text())

    def test_header_only_skia(self):
        import os
        import subprocess
        root = ROOT / "public/textra/capi/platform"
        sdk = os.environ.get("SKIA_INCLUDE_DIR")
        if not sdk:
            candidates = list((ROOT / "third_party/skia_release/linux-x64/headers").glob(
                "skia-*/include/core/SkCanvas.h"))
            if candidates:
                sdk = str(candidates[0].parents[2])
        if not sdk:
            self.skipTest("Skia SDK unavailable; set SKIA_INCLUDE_DIR")
        headers = ["textra/capi/platform/textra_capi_skia.h"]
        headers += [path.relative_to(ROOT / "public").as_posix()
                    for path in sorted((root / "skia").glob("*.hpp"))]
        for header in headers:
            source = f'#include <{header}>\n#include <{header}>\n'
            subprocess.run(["clang++", "-std=c++17", "-Werror", "-fno-rtti",
                            "-fsyntax-only", "-x", "c++", "-I"+str(ROOT / "public"),
                            "-isystem", sdk, "-"], input=source, text=True,
                           check=True, capture_output=True)

    def test_header_only_skity(self):
        import os
        import subprocess
        root = ROOT / "public/textra/capi/platform"
        common = root / "skity_common"
        self.assertFalse((common / "textra_skity_backend_impl.h").exists())
        for folder in ("skity", "skity_coretext", "skity_common"):
            self.assertFalse(list((root / folder).glob("*.cc")))
            self.assertFalse(list((root / folder).glob("*.hpp")))
        # Compile against the actual renderer SDK when available. No SDK stubs.
        sdk = os.environ.get("SKITY_CAPI_INCLUDE_DIR")
        framework = ROOT / "third_party/skity_release/apple/skity.xcframework/macos-arm64_x86_64"
        if sdk:
            sdk_flags = ["-isystem", sdk]
        elif (framework / "skity.framework/Headers/c/skity.h").is_file():
            sdk_flags = ["-F" + str(framework)]
        else:
            self.skipTest("Skity C API SDK unavailable; set SKITY_CAPI_INCLUDE_DIR")
        headers = sorted(path.relative_to(root).as_posix()
                         for path in common.glob("*.h"))
        headers += [backend + "/textra_" + backend + "_backend.h"
                    for backend in ("skity", "skity_coretext")]
        headers += ["textra_capi_skity.h", "textra_capi_skity_coretext.h"]
        for header in headers:
            header = "textra/capi/platform/" + header
            source = f'#include <{header}>\n#include <{header}>\n'
            for compiler, standard, language in (("clang", "c11", "c"),
                                                  ("clang++", "c++17", "c++")):
                subprocess.run([compiler, "-std=" + standard, "-Wall", "-Wextra",
                                "-Werror", "-fsyntax-only", "-x", language,
                                "-I" + str(ROOT / "public"), "-I" + str(root),
                                *sdk_flags, "-"], input=source, text=True,
                               check=True, capture_output=True)

    def test_pure_c_headers(self):
        import subprocess
        for path in (ROOT / "public/textra/capi").glob("*.h"):
            subprocess.run(["clang", "-std=c11", "-Werror", "-fsyntax-only", "-x", "c", "-I"+str(ROOT / "public"), str(path)], check=True, capture_output=True)
if __name__ == "__main__":
    unittest.main()
