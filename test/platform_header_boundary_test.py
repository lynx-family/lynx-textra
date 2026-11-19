#!/usr/bin/env python3
"""Host examples stay public; renderer implementations stay outside core."""
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
            self.assertNotIn("examples/capi/platform/", path.read_text(), str(path))
        cmake = (ROOT / "CMakeLists.txt").read_text()
        self.assertNotIn("TEXTRA_ENABLE_SKIA_BACKEND", cmake)
        self.assertNotIn("find_library", cmake)
    def test_native_c_backend_boundary(self):
        self.assertFalse((ROOT / "public/textra/capi/platform").exists())
        self.assertFalse(list((ROOT / "public/textra/capi").rglob("*.hpp")))
        root = ROOT / "examples/capi/platform"
        self.assertFalse((ROOT / "public/textra/platform/capi").exists())
        for path in list(root.rglob("*.hpp")) + list(root.rglob("*.h")):
            text = path.read_text()
            self.assertNotIn("#include <textra/platform/", text, str(path))
            self.assertNotIn("#include <textra/i_", text, str(path))
            for name in ("ICanvasHelper", "IFontManager", "ITypefaceHelper", "BackendFactories"):
                self.assertNotIn(name, text, str(path))

    def test_independent_skity_platforms(self):
        root = ROOT / "examples/capi/platform"
        for platform in ("apple", "ios", "mac", "android", "ohos", "lvgl"):
            self.assertFalse((root / platform).exists())
        adapter = root / "skity_coretext/textra_skity_coretext_backend"
        for suffix in (".h", ".cc"):
            path = adapter.with_suffix(suffix)
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

    def test_compiled_backends(self):
        import subprocess
        root = ROOT / "examples/capi/platform"
        for path in root.rglob("*_backend.h"):
            self.assertNotIn("BACKEND_IMPLEMENTATION", path.read_text())
            self.assertTrue(path.with_suffix(".cc").is_file())
            header = path.relative_to(ROOT).as_posix()
            source = f"#include <{header}>\n#include <{header}>\n"
            # Both languages consume declarations without renderer SDK headers.
            for compiler, standard, language in (("clang++", "c++17", "c++"),
                                                   ("clang", "c11", "c")):
                subprocess.run([compiler, "-std="+standard, "-Werror", "-fsyntax-only",
                                "-x", language, "-I"+str(ROOT / "public"),
                                "-I"+str(ROOT), "-"], input=source, text=True,
                               check=True, capture_output=True)

    def test_pure_c_headers(self):
        import subprocess
        for path in (ROOT / "public/textra/capi").rglob("*.h"):
            subprocess.run(["clang", "-std=c11", "-Werror", "-fsyntax-only", "-x", "c", "-I"+str(ROOT / "public"), str(path)], check=True, capture_output=True)
if __name__ == "__main__":
    unittest.main()
