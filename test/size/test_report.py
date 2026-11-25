# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
import json
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

from report import android_artifacts, compare, markdown, THRESHOLD
import run


class SizeReportTest(unittest.TestCase):
    def compare_size(self, delta):
        return compare({'library': dict(bytes=100000, sha256='base')},
                       {'library': dict(bytes=100000 + delta, sha256='head')})[0]

    def test_exact_threshold_does_not_warn(self):
        self.assertFalse(self.compare_size(THRESHOLD)['warning'])

    def test_one_byte_over_threshold_warns(self):
        self.assertTrue(self.compare_size(THRESHOLD + 1)['warning'])

    def test_equal_or_smaller_does_not_warn(self):
        for delta in (0, -20000):
            self.assertFalse(self.compare_size(delta)['warning'])

    def test_missing_or_empty_artifacts_are_not_success(self):
        for before, after in (({}, {}), ({'library': dict(bytes=10)}, {}),
                              ({'library': dict(bytes=10)}, {'library': dict(bytes=0)})):
            with self.assertRaises(ValueError):
                compare(before, after)

    def test_aar_checks_native_bytes_per_abi(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'library.aar'
            with zipfile.ZipFile(path, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
                for abi in ('arm64-v8a', 'armeabi-v7a', 'x86'):
                    archive.writestr('jni/' + abi + '/liblynxtextra.so', b'X' * 20000)
            result = android_artifacts(path)
            self.assertEqual(20000, result['jni/arm64-v8a/liblynxtextra.so']['bytes'])
            self.assertEqual(path.stat().st_size, result['release.aar']['bytes'])

    def test_missing_abi_is_not_success(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'library.aar'
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr('jni/x86/liblynxtextra.so', b'library')
            with self.assertRaisesRegex(ValueError, 'missing ABI'):
                android_artifacts(path)

    def test_added_native_dependency_is_reported_as_growth(self):
        before = {'release.aar': dict(bytes=100, sha256='base')}
        after = {'release.aar': dict(bytes=120, sha256='head'),
                 'jni/arm64-v8a/libnew.so': dict(bytes=THRESHOLD + 1, sha256='new')}
        row = next(row for row in compare(before, after) if row['artifact'].endswith('libnew.so'))
        self.assertEqual(THRESHOLD + 1, row['delta'])
        self.assertTrue(row['warning'])
        self.assertIsNone(row['percent'])

    def test_build_failure_retains_error_report(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'results'
            argv = ['run.py', '--platform', 'windows', '--baseline', 'a' * 40,
                    '--candidate', 'b' * 40, '--output', str(output)]
            with patch.object(sys, 'argv', argv), patch.object(run, 'execute', side_effect=['b' * 40, '']), \
                 patch.object(run, 'checkout', side_effect=ValueError('dependency versions differ')), \
                 patch.object(run, 'cleanup'):
                with self.assertRaisesRegex(ValueError, 'dependency versions differ'):
                    run.main()
            report = json.loads((output / 'report.json').read_text())
            self.assertEqual('error', report['status'])
            self.assertEqual([], report['artifacts'])
            self.assertIn('Comparison unavailable', (output / 'summary.md').read_text())

    def test_windows_exports_follow_each_revisions_headers(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            headers = root / 'public/textra/capi'
            headers.mkdir(parents=True)
            (headers / 'api.h').write_text('''
/* TEXTRA_API void textra_not_exported(void); */
TEXTRA_API uint32_t TEXTRA_CALL textra_get_version(void);
TEXTRA_API textra_status_t TEXTRA_CALL
textra_layout(textra_layout_t* layout);
''')
            self.assertEqual('EXPORTS\ntextra_get_version\ntextra_layout\n', run.windows_exports(root))

    def test_growth_warning_returns_success(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'results'
            argv = ['run.py', '--platform', 'windows', '--baseline', 'a' * 40,
                    '--candidate', 'b' * 40, '--output', str(output)]
            artifacts = ({'library': dict(bytes=100, sha256='base')},
                         {'library': dict(bytes=100 + THRESHOLD + 1, sha256='head')})
            with patch.object(sys, 'argv', argv), patch.object(run, 'execute', side_effect=['b' * 40, '']), \
                 patch.object(run, 'checkout'), patch.object(run, 'cleanup'), \
                 patch.object(run, 'build_pair', return_value=artifacts):
                self.assertEqual(0, run.main())
            self.assertEqual('warning', json.loads((output / 'report.json').read_text())['status'])

    def test_warning_summary_preserves_byte_delta(self):
        rows = [self.compare_size(THRESHOLD + 1)]
        text = markdown(dict(platform='macos', baseline='base', candidate='head',
                             status='warning', artifacts=rows))
        self.assertIn('+10241', text)
        self.assertIn('WARNING', text)


if __name__ == '__main__':
    unittest.main()
