#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Verify backend selection independently of host SDKs and platform macros."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
BACKENDS = ["Android", "IOS", "MacOS", "OHOS", "LVGL", "Skia", "Skity",
            "IOSSelfRendering", "MacOSSelfRendering", "SkityCoreText", "Invalid"]


class RenderingBackendConfigTest(unittest.TestCase):
    def test_combinations(self):
        cases = [
            ([], set()),
            (["TEXTRA_ENABLE_ANDROID_BACKEND"], {"Android"}),
            (["TEXTRA_ENABLE_OHOS_BACKEND"], {"OHOS"}),
            (["TEXTRA_ENABLE_LVGL_BACKEND"], {"LVGL"}),
            (["ENABLE_SKSHAPER"], {"Skia", "Skity"}),
            (["ENABLE_CTSHAPER"], {"SkityCoreText"}),
            (["TEXTRA_ENABLE_IOS_BACKEND"], set()),
            (["TEXTRA_ENABLE_MACOS_BACKEND"], set()),
            (["TEXTRA_ENABLE_IOS_BACKEND", "ENABLE_CTSHAPER"],
             {"IOS", "IOSSelfRendering", "SkityCoreText"}),
            (["TEXTRA_ENABLE_MACOS_BACKEND", "ENABLE_CTSHAPER"],
             {"MacOS", "MacOSSelfRendering", "SkityCoreText"}),
            (["ENABLE_SKSHAPER", "ENABLE_CTSHAPER"],
             {"Skia", "Skity", "SkityCoreText"}),
        ]
        with tempfile.TemporaryDirectory(prefix="textra-backend-config-") as tmp:
            source = Path(tmp) / "test.cc"
            binary = Path(tmp) / "test"
            for definitions, expected in cases:
                with self.subTest(definitions=definitions):
                    checks = [
                        f"if (IsBackendCompiled(RenderingBackend::k{backend}) != "
                        f"{str(backend in expected).lower()}) return {i + 1};"
                        for i, backend in enumerate(BACKENDS)
                    ]
                    for backend in BACKENDS:
                        selected = "Invalid"
                        if backend in {"Skia", "Skity"}:
                            if backend in expected:
                                selected = backend
                            elif "SkityCoreText" in expected:
                                selected = "SkityCoreText"
                        checks.append(
                            f"if (GetSelfRenderingBackend(RenderingBackend::k{backend}) != "
                            f"RenderingBackend::k{selected}) return 100;")
                    source.write_text(
                        '#include "src/ports/rendering_backend_internal.h"\n'
                        'using namespace ttoffice::tttext;\n'
                        'using namespace ttoffice::tttext::internal;\n'
                        'int main() {\n' + '\n'.join(checks) + '\n}\n')
                    subprocess.run([os.environ.get("CXX", "clang++"), "-std=c++17",
                                    "-I" + str(ROOT), "-I" + str(ROOT / "public"),
                                    *["-D" + d for d in definitions], str(source),
                                    "-o", str(binary)], check=True, capture_output=True)
                    subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
