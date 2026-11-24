#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Verify Android GN targets keep HarfBuzz out of the Textra shared library."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class AndroidHarfBuzzConfigTest(unittest.TestCase):
    def check_graph(self, cpu, extra_args):
        (ROOT / 'out').mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='harfbuzz-config-', dir=ROOT / 'out') as directory:
            args = ('target_os="android" target_cpu="' + cpu + '" '
                    'use_ndk_cxx=true is_debug=false use_flutter_cxx=false ' + extra_args)
            result = subprocess.run([str(ROOT / 'buildtools/gn/gn'), 'gen', directory,
                                     '--ide=json', '--args=' + args], cwd=ROOT,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            targets = json.loads((Path(directory) / 'project.json').read_text())['targets']
            shapers = [target for label, target in targets.items()
                       if label.split('(')[0] == '//src:skshaper']
            self.assertTrue(shapers)
            for target in shapers:
                sources = target['sources']
                self.assertTrue(any(path.endswith('/harfbuzz_lib_runtime.cc') for path in sources))
                self.assertFalse(any(path.endswith('/harfbuzz_lib_builtin.cc') for path in sources))
                self.assertIn('HARFBUZZ_LIB_NAME="libharfbuzz.so"', target['defines'])
            libraries = [label for label in targets
                         if label.split('(')[0] == '//platform/android/lynxtextra:lynxtextra']
            self.assertTrue(libraries)
            pending = list(libraries)
            visited = set()
            while pending:
                label = pending.pop()
                if label in visited:
                    continue
                visited.add(label)
                self.assertNotEqual(label.split('(')[0], '//third_party/harfbuzz:harfbuzz')
                pending.extend(targets[label].get('deps', []))

    def test_default_android_builds(self):
        for cpu in ('arm', 'arm64', 'x86'):
            with self.subTest(cpu=cpu):
                self.check_graph(cpu, '')

    def test_static_overrides_cannot_bundle_harfbuzz(self):
        for cpu in ('arm', 'arm64', 'x86'):
            for args in ('dynamic_load_harfbuzz=false',
                         'textlayout_use_local_config=false textlayout_dynamic_load_harfbuzz=false'):
                with self.subTest(cpu=cpu, args=args):
                    self.check_graph(cpu, args)


if __name__ == '__main__':
    unittest.main()
