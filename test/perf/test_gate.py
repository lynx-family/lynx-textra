#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Regression tests for the gate's pass/fail contract, with synthetic timings."""
import contextlib
import copy
import io
import json
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

from pathlib import Path

import gate

BASE = '1' * 40
HEAD = '2' * 40
CASE = dict(id='ascii', text='abc', rtl=False, primary='default')
ADAPTER = '''import json, sys
r = json.load(sys.stdin)
version, mode = sys.argv[1:]
if mode == 'exit': sys.exit(3)
identity = {k: k for k in ['device','os','arch','fonts','dependencies','benchmark']}
identity['backend'] = r['backend']
if mode == 'identity': identity['os'] = version
value = 12 if mode == 'slow' and version == 'candidate' else 10
output = dict(glyphs=3, missing=0, signature='same')
if mode == 'changed' and version == 'candidate': output['signature'] = 'changed'
if mode == 'missing': output['missing'] = 1
json.dump(dict(schema=1, request_id=r['request_id'], revision=r['revision'],
case=r['case']['id'], cache_disabled=True, identity=identity, output=output,
batches=[dict(shape=value-1, cleanup=1, total=value) for _ in range(r['batches'])]), sys.stdout)
'''


class GateTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.adapter = self.root / 'adapter.py'
        self.adapter.write_text(ADAPTER)

    def config(self, mode='pass'):
        config = dict(schema=1, platform='macos', backend='macos', rounds=8,
                      warmup=1, batches=3, iterations=1, timeout=5)
        for version, revision in [('baseline', BASE), ('candidate', HEAD)]:
            config[version] = dict(revision=revision,
                command=[sys.executable, str(self.adapter), version, mode],
                artifacts=[dict(path=str(self.adapter), sha256=gate.digest(self.adapter))])
        return config

    def measure(self, mode):
        with contextlib.redirect_stdout(io.StringIO()):
            report = gate.run(self.config(mode), [CASE], self.root / mode)
        self.assertTrue((self.root / mode / 'raw.json').exists())
        return report

    def test_equal_timings_pass(self):
        self.assertTrue(self.measure('pass')['passed'])

    def test_sustained_regression_fails(self):
        report = self.measure('slow')
        self.assertFalse(report['passed'])
        self.assertEqual(report['cases'][0]['status'], 'regression')
        self.assertEqual(report['cases'][0]['total']['slower_pairs'], 8)

    def test_output_change_and_missing_glyphs_fail(self):
        self.assertEqual(self.measure('changed')['cases'][0]['status'], 'output_changed')
        self.assertEqual(self.measure('missing')['cases'][0]['status'], 'missing_glyphs')

    def test_identity_drift_and_crashes_fail_without_repeated_attempts(self):
        self.assertFalse(self.measure('identity')['passed'])
        report = self.measure('exit')
        self.assertFalse(report['passed'])
        self.assertEqual(len(report['issues']), 2)

    def test_single_noisy_round_and_tiny_absolute_change_do_not_fail(self):
        self.assertFalse(gate.classify([10]*8, [10]*7+[30], {})['regression'])
        self.assertFalse(gate.classify([1]*8, [1.5]*8, {})['regression'])
        self.assertTrue(gate.classify([10]*8, [12]*8, {})['regression'])

    def test_bad_sampling_and_policy_are_rejected(self):
        for key, value in [('rounds', 7), ('rounds', True), ('batches', 1),
                           ('iterations', -1), ('warmup', 0)]:
            config = self.config()
            config[key] = value
            with self.assertRaises(ValueError): gate.validate_config(config)
        config = self.config()
        config['policy'] = dict(relative=float('nan'))
        with self.assertRaises(ValueError): gate.validate_config(config)

    def test_artifact_drift_is_rejected(self):
        config = self.config()
        self.adapter.write_text(ADAPTER + '\n# changed\n')
        with self.assertRaises(ValueError): gate.validate_config(config)

    def test_same_revision_requires_explicit_aa_control(self):
        config = self.config()
        config['candidate']['revision'] = BASE
        with self.assertRaises(ValueError): gate.validate_config(config)
        config['aa_control'] = True
        gate.validate_config(config)

    def test_stale_wrong_revision_and_nan_responses_are_rejected(self):
        request = dict(schema=1, request_id='fresh', revision=HEAD, backend='macos',
                       case=CASE, batches=3)
        response = dict(schema=1, request_id='fresh', revision=HEAD, case='ascii',
                        cache_disabled=True, identity={k: k for k in gate.IDENTITY},
                        output=dict(glyphs=3, missing=0, signature='abc'),
                        batches=[dict(shape=9, cleanup=1, total=10)]*3)
        response['identity']['backend'] = 'macos'
        gate.validate_response(response, request)
        for key, value in [('request_id', 'stale'), ('revision', BASE),
                           ('cache_disabled', False)]:
            bad = copy.deepcopy(response)
            bad[key] = value
            with self.assertRaises(ValueError): gate.validate_response(bad, request)
        bad = copy.deepcopy(response)
        bad['batches'][0]['total'] = float('nan')
        with self.assertRaises(ValueError): gate.validate_response(bad, request)

    def test_persistent_adapter_reuses_process_and_reports_regressions(self):
        lines = ADAPTER.splitlines()
        server = lines[0] + '\nfor line in sys.stdin:\n' + '\n'.join(
            '    ' + line.replace('r = json.load(sys.stdin)', 'r = json.loads(line)')
            for line in lines[1:]) + "\n    sys.stdout.write('\\n'); sys.stdout.flush()\n"
        self.adapter.write_text(server)
        config = self.config('slow')
        for version in ['baseline', 'candidate']:
            config[version]['protocol'] = 'jsonl'
        with contextlib.redirect_stdout(io.StringIO()):
            report = gate.run(config, [CASE, dict(CASE, id='second')], self.root/'persistent')
        self.assertFalse(report['passed'])
        self.assertTrue(all(x['status'] == 'regression' for x in report['cases']))
        self.assertEqual(report['issues'], [])

    def test_persistent_timeout_restarts_host_for_next_case(self):
        self.adapter.write_text("import json,sys,time\nfor line in sys.stdin:\n"
            " r=json.loads(line)\n if r.get('hang'): time.sleep(10)\n"
            " print(json.dumps({'ok':True}),flush=True)\n")
        adapter = gate.JsonLinesAdapter([sys.executable, str(self.adapter)], self.root/'host.log')
        try:
            with self.assertRaises(subprocess.TimeoutExpired):
                adapter.invoke(dict(hang=True), 0.05)
            self.assertIsNone(adapter.process)
            self.assertEqual(json.loads(adapter.invoke(dict(hang=False), 2).stdout), {'ok': True})
        finally:
            adapter.close()
            adapter.stderr.close()

    def test_persistent_close_allows_app_launcher_cleanup(self):
        marker = self.root/'cleaned'
        self.adapter.write_text("import json,sys\ntry:\n for line in sys.stdin:\n"
            "  print(json.dumps({'ok':True}),flush=True)\nfinally:\n"
            + " open("+repr(str(marker))+",'w').write('stopped')\n")
        adapter = gate.JsonLinesAdapter([sys.executable,str(self.adapter)],self.root/'host.log')
        try:
            adapter.invoke({},2)
        finally:
            adapter.close()
            adapter.stderr.close()
        self.assertEqual(marker.read_text(),'stopped')

    def test_all_platform_labels_use_the_same_validation_contract(self):
        for platform in gate.PLATFORMS:
            config = self.config()
            config['platform'] = platform
            config['backend'] = 'skity' if platform == 'windows' else platform
            gate.validate_config(config)

    def test_timeout_stops_adapter(self):
        with self.assertRaises(subprocess.TimeoutExpired):
            gate.invoke([sys.executable, '-c', 'import time; time.sleep(10)'], {}, 0.05)


if __name__ == '__main__':
    unittest.main()
