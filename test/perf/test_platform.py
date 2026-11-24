#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
import json
import hashlib
import plistlib
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import android_emulator
import app_adapter
import fetch_fonts
import run_platform


class PlatformTest(unittest.TestCase):
    def test_ios_uses_available_device_type_for_owned_simulator(self):
        devices = {'runtime-ios': [dict(udid='existing',name='iPhone 17',
                                       deviceTypeIdentifier='type-iphone')]}
        runtimes = dict(runtimes=[dict(isAvailable=True,name='iOS 26',
                                      identifier='runtime-ios',version='26.0')])
        with mock.patch('run_platform.execute',side_effect=[json.dumps(dict(devices=devices)),
                          json.dumps(runtimes),'created-udid']) as execute:
            self.assertEqual(run_platform.ios_simulator(None),('created-udid',True))
            self.assertEqual(execute.call_args.args[0][-2:],['type-iphone','runtime-ios'])

    def test_ios_rejects_unavailable_explicit_simulator(self):
        with mock.patch('run_platform.execute',return_value=json.dumps(dict(devices={}))):
            with self.assertRaises(ValueError): run_platform.ios_simulator('missing')

    def test_ios_app_adopts_scenes_and_retains_metadata(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            executable,metadata = root/'perf_host',root/'metadata.json'
            executable.write_bytes(b'fake executable');metadata.write_text('{"revision":"pinned"}')
            with mock.patch('run_platform.subprocess.run'):
                archive = run_platform.ios_app(executable,metadata,'test.perf',root/'Perf.app')
            self.assertTrue(archive.is_file())
            info = plistlib.loads((root/'Perf.app/Info.plist').read_bytes())
            self.assertEqual(info['CFBundleIdentifier'],'test.perf')
            self.assertIn('UIApplicationSceneManifest',info)
            self.assertEqual((root/'Perf.app/metadata.json').read_bytes(),metadata.read_bytes())

    def test_adb_commands_cannot_consume_protocol_stdin(self):
        with mock.patch('app_adapter.subprocess.run') as run:
            app_adapter.call(['adb','shell','getprop'])
            self.assertEqual(run.call_args.kwargs['stdin'],app_adapter.subprocess.DEVNULL)
            app_adapter.call(['adb','shell','cat'],input=b'request')
            self.assertNotIn('stdin',run.call_args.kwargs)

    def test_font_lock_validates_download_and_cache(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);manifest=root/'lock.json';data=b'font data';pin='a'*40
            lock=dict(schema=1,source_commit=pin,fonts=[dict(name='font.ttf',
                url='https://example.test/'+pin+'/font.ttf',
                sha256=hashlib.sha256(data).hexdigest())])
            manifest.write_text(json.dumps(lock))
            with mock.patch('fetch_fonts.urllib.request.urlopen') as request:
                request.return_value.read.return_value=data
                paths=fetch_fonts.fetch(root/'fonts',manifest)
                self.assertEqual(paths[0].read_bytes(),data)
                request.reset_mock();fetch_fonts.fetch(root/'fonts',manifest)
                request.assert_not_called()
                paths[0].write_bytes(b'changed')
                request.return_value.read.return_value=b'wrong download'
                with self.assertRaises(ValueError):fetch_fonts.fetch(root/'fonts',manifest)

    def test_android_waits_for_response_and_preserves_remote_exit_code(self):
        response = dict(request_id='request')
        results = [mock.Mock(returncode=1,stdout=b''),
                   mock.Mock(returncode=0,stdout=b''),
                   mock.Mock(returncode=0,stdout=json.dumps(response).encode())]
        with mock.patch('app_adapter.call'), mock.patch('app_adapter.time.sleep'), \
             mock.patch('app_adapter.subprocess.run',side_effect=results) as run:
            self.assertEqual(app_adapter.android('adb','device','package',response,5),response)
            self.assertEqual(run.call_count,3)
            self.assertEqual(run.call_args.args[0][3:5],['shell','-T'])

    def test_android_selects_available_even_port(self):
        port = android_emulator.choose_port()
        self.assertEqual(port%2,0)
        self.assertGreaterEqual(port,5554)
        self.assertLess(port,5680)


if __name__=='__main__':
    unittest.main()
