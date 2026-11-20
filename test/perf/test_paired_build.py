# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
import tempfile
from pathlib import Path
import threading
import unittest

from paired_build import build_pair, verify_shared_dependencies


class PairedBuildTest(unittest.TestCase):
    def test_only_renderer_pins_may_differ(self):
        with tempfile.TemporaryDirectory() as temp:
            baseline, candidate = Path(temp)/'base', Path(temp)/'head'
            baseline.mkdir(); candidate.mkdir()
            for root in (baseline, candidate):
                (root/'.gn').write_text('buildconfig = "//build/config/BUILDCONFIG.gn"')
            (baseline/'DEPS').write_text("deps = {'third_party/harfbuzz': {'commit': 'core'}, "
                                        "'third_party/skity': {'commit': 'old-renderer'}}")
            (candidate/'DEPS').write_text("deps = {'third_party/harfbuzz': {'commit': 'core'}}")
            verify_shared_dependencies(baseline, candidate)
            (candidate/'DEPS').write_text("deps = {'third_party/harfbuzz': {'commit': 'changed'}}")
            with self.assertRaisesRegex(ValueError, 'engine DEPS'):
                verify_shared_dependencies(baseline, candidate)
            (candidate/'DEPS').write_bytes((baseline/'DEPS').read_bytes())
            (candidate/'.gn').write_text('different build configuration')
            with self.assertRaisesRegex(ValueError, '.gn'):
                verify_shared_dependencies(baseline, candidate)

    def test_builds_overlap_and_split_the_job_budget(self):
        ready = threading.Barrier(2)
        def compile_revision(version, jobs):
            ready.wait(timeout=3)
            return version,jobs
        self.assertEqual(build_pair(compile_revision,['base','head'],6),
                         [('base',3),('head',3)])

    def test_failed_build_still_joins_other_compiler(self):
        ready,finished = threading.Barrier(2),threading.Event()
        def compile_revision(version, jobs):
            ready.wait(timeout=3)
            if version=='base': raise RuntimeError('compile failed')
            finished.wait(timeout=0.05)
            finished.set()
        with self.assertRaisesRegex(RuntimeError,'compile failed'):
            build_pair(compile_revision,['base','head'],2)
        self.assertTrue(finished.is_set())

    def test_one_job_builds_serially_and_invalid_budget_does_not_start(self):
        calls = []
        def compile_revision(version, jobs):
            calls.append((version,jobs))
        build_pair(compile_revision,['base','head'],1)
        self.assertEqual(calls,[('base',1),('head',1)])
        with self.assertRaises(ValueError):build_pair(compile_revision,['base','head'],0)
        self.assertEqual(len(calls),2)


if __name__=='__main__':
    unittest.main()
