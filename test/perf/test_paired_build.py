# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
import threading
import unittest

from paired_build import build_pair


class PairedBuildTest(unittest.TestCase):
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
