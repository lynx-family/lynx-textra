#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Regress transitive implementation sources in generated CocoaPods specs."""
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools/gn_tools'))
from gn_to_podspec import Project, write_podspec


def target(kind='source_set', sources=(), deps=()):
    return {'type': kind, 'sources': list(sources), 'deps': list(deps),
            'metadata': {}, 'public': []}


class PodspecSourcesTest(unittest.TestCase):
    def generate(self, targets, directory):
        project = Project(
            {'targets': targets,
             'build_settings': {'root_path': str(directory),
                                'build_dir': '//out'}},
            '//:pods', '//:subspecs', '//:compiler', 'Test.podspec')
        self.assertEqual(write_podspec(project, [], {}, {}), 0)
        return (directory / 'out/podspecs/Test.podspec').read_text()

    def test_transitive_native_and_callback_sources(self):
        targets = {
            '//:pods': target('group', deps=['//:sdk']),
            '//:subspecs': target('group'),
            '//:compiler': target(),
            '//:sdk': target(deps=['//:capi', '//:core']),
            '//:capi': target(sources=['//src/capi.cc'], deps=['//:core']),
            '//:core': target(sources=['//src/core.cc'],
                              deps=['//:native', '//:callbacks']),
            '//:native': target(sources=['//src/native.mm']),
            '//:callbacks': target(sources=['//src/callbacks.cc']),
        }
        with tempfile.TemporaryDirectory(prefix='textra-podspec-test-') as tmp:
            spec = self.generate(targets, Path(tmp))
        for source in ('capi.cc', 'core.cc', 'native.mm', 'callbacks.cc'):
            self.assertEqual(spec.count('"src/' + source + '"'), 1, spec)

    def test_named_subspec_is_not_flattened_into_parent(self):
        targets = {
            '//:pods': target('group', deps=['//:sdk']),
            '//:subspecs': target('group', deps=['//:shared']),
            '//:compiler': target(),
            '//:sdk': target(sources=['//src/sdk.cc'], deps=['//:shared']),
            '//:shared': target(sources=['//src/shared.cc'], deps=['//:leaf']),
            '//:leaf': target(sources=['//src/leaf.cc']),
        }
        with tempfile.TemporaryDirectory(prefix='textra-podspec-test-') as tmp:
            spec = self.generate(targets, Path(tmp))
        before_shared, after_shared = spec.split('sp.subspec "shared"', 1)
        self.assertNotIn('"src/shared.cc"', before_shared)
        self.assertNotIn('"src/leaf.cc"', before_shared)
        self.assertIn('"src/shared.cc"', after_shared)
        self.assertIn('"src/leaf.cc"', after_shared)


if __name__ == '__main__':
    unittest.main()
