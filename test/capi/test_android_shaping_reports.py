#!/usr/bin/env python3
"""Check report handling without an Android device or timed measurements."""
import copy
import io
import json
from pathlib import Path
import re
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

from summarize_android_shaping import REPORT_VERSION

SCRIPTS = Path(__file__).resolve().parent


def report(rotation=0):
    rows = []
    for case in range(5):
        for rounded in (False, True):
            for mode in ('platform_shape', 'engine_cache_miss', 'engine_cache_hit',
                         'first_shape_fresh_manager'):
                rows.append(dict(case=str(case), codepoints=32, metrics_round=rounded,
                                 mode=mode, results_equal=True, cpp=[10, 10, 10],
                                 capi=[11, 11, 11], cpp_cleanup=[1, 1, 1],
                                 capi_cleanup=[2, 2, 2], cpp_setup=[3, 3, 3],
                                 capi_setup=[4, 4, 4]))
    return dict(version=REPORT_VERSION, global_warmup_ms=2000,
                workload_rotation=rotation, rounds=3, batch=2, warmup=1,
                results_equal=True, cleanup_status=0, rows=rows)


class AndroidShapingReportsTest(unittest.TestCase):
    def test_report_version_matches_native_producer(self):
        source = (SCRIPTS / 'android_shaping_benchmark.cc').read_text()
        match = re.search(r'version\\":(\d+)', source)
        self.assertIsNotNone(match)
        self.assertEqual(int(match[1]), REPORT_VERSION)

    def collect(self, directory, payload):
        apk = directory / 'fixture.apk'
        with zipfile.ZipFile(apk, 'w') as archive:
            for name in ('classes.dex', 'lib/arm64-v8a/liblynxtextra.so',
                         'lib/arm64-v8a/libandroid_test.so'):
                archive.writestr(name, b'fixture')
        original_run = subprocess.run
        rotation = 0
        process = 0

        def fake_adb(command, **kwargs):
            nonlocal rotation, process
            if command[0] == sys.executable:
                return original_run(command, capture_output=True, **kwargs)
            stdout = b''
            if 'start' in command:
                process += 1
                rotation = int(command[-1])
            elif 'pidof' in command:
                stdout = str(process).encode()
            elif 'exec-out' in command:
                data = copy.deepcopy(payload)
                data['workload_rotation'] = rotation
                stdout = json.dumps(data).encode()
            elif 'getprop' in command:
                stdout = b'fixture-device'
            if kwargs.get('text'):
                stdout = stdout.decode()
            return subprocess.CompletedProcess(command, 0, stdout)

        output = directory / 'runs'
        arguments = [str(SCRIPTS / 'run_android_shaping_benchmark.py'), '--apk', str(apk),
                     '--output', str(output), '--rotations', '0', '2', '--rounds', '3']
        with patch.object(sys, 'argv', arguments), patch('subprocess.run', side_effect=fake_adb), \
                patch('sys.stdout', new_callable=io.StringIO):
            runpy.run_path(arguments[0], run_name='__main__')
        return json.loads((output / 'summary.json').read_text())

    def test_current_report_completes_collection_and_summary(self):
        with tempfile.TemporaryDirectory() as directory:
            summary = self.collect(Path(directory), report())
        self.assertEqual(summary['version'], REPORT_VERSION)
        self.assertEqual(summary['runs'], 2)
        self.assertEqual(summary['global_warmup_ms'], 2000)
        self.assertEqual(summary['workload_rotations'], [0, 2])
        self.assertEqual(len(summary['rows']), 40)
        self.assertAlmostEqual(summary['rows'][0]['paired_delta_percent'], 10)
        self.assertEqual(summary['rows'][0]['capi_setup_us'], 4)

    def test_collection_rejects_stale_or_failed_reports(self):
        for field, value in (('version', 4), ('rounds', 5), ('global_warmup_ms', 0),
                             ('results_equal', False), ('cleanup_status', 1)):
            with self.subTest(field=field), tempfile.TemporaryDirectory() as directory:
                payload = report()
                payload[field] = value
                with self.assertRaisesRegex(RuntimeError, 'Incomplete or failed benchmark'):
                    self.collect(Path(directory), payload)

    def test_summary_version_compatibility(self):
        for version in (2, 3, 4, REPORT_VERSION, REPORT_VERSION + 1):
            with self.subTest(version=version), tempfile.TemporaryDirectory() as directory:
                path = Path(directory)
                data = report()
                data['version'] = version
                (path / 'run.json').write_text(json.dumps(data))
                result = subprocess.run([sys.executable, str(SCRIPTS / 'summarize_android_shaping.py'),
                                         str(path / 'run.json'), '--output', str(path / 'summary.json')],
                                        capture_output=True, text=True)
                if version == REPORT_VERSION + 1:
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn('Unsupported benchmark version', result.stderr)
                else:
                    self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()
