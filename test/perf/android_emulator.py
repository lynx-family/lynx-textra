#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Start/stop one owned AVD, following Lynx's Android E2E runner setup."""
import argparse
import json
import os
import socket
import subprocess
import time
from pathlib import Path


def choose_port():
    for port in range(5554,5680,2):
        sockets = [socket.socket(),socket.socket()]
        try:
            for sock, number in zip(sockets,[port,port+1]): sock.bind(('127.0.0.1',number))
            return port
        except OSError:
            pass
        finally:
            for sock in sockets: sock.close()
    raise RuntimeError('no unused Android emulator ports')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['start','stop'])
    parser.add_argument('--state',type=Path,required=True)
    args = parser.parse_args()
    sdk = Path(os.environ['ANDROID_HOME'])
    adb,emulator = sdk/'platform-tools/adb',sdk/'emulator/emulator'
    if args.action=='stop':
        if args.state.exists():
            state = json.loads(args.state.read_text())
            subprocess.run([str(adb),'-s',state['serial'],'emu','kill'],
                           check=False,capture_output=True,timeout=15)
        return 0
    avds = subprocess.check_output([str(emulator),'-list-avds'],text=True).splitlines()
    if not avds: raise RuntimeError('runner has no AVD; use Lynx Android E2E runner')
    args.state.parent.mkdir(parents=True,exist_ok=True)
    port = choose_port();serial = 'emulator-'+str(port)
    with args.state.with_suffix('.log').open('w') as log:
        process = subprocess.Popen([str(emulator),'-avd',avds[0],'-port',str(port),
            '-no-window','-no-snapshot','-no-audio','-no-boot-anim','-read-only'],
            stdout=log,stderr=subprocess.STDOUT,start_new_session=True,
            env={**os.environ,'QT_QPA_PLATFORM':'offscreen'})
    args.state.write_text(json.dumps(dict(serial=serial,pid=process.pid,avd=avds[0])))
    deadline = time.monotonic()+300
    while time.monotonic()<deadline:
        if process.poll() is not None: raise RuntimeError('emulator exited; see emulator log')
        result = subprocess.run([str(adb),'-s',serial,'shell','getprop','sys.boot_completed'],
                                text=True,capture_output=True,timeout=10)
        if result.returncode==0 and result.stdout.strip()=='1':
            if os.environ.get('GITHUB_ENV'):
                with open(os.environ['GITHUB_ENV'],'a') as env: env.write('ANDROID_SERIAL='+serial+'\n')
            print(serial);return 0
        time.sleep(2)
    raise TimeoutError('Android emulator did not boot within 300 seconds')


if __name__=='__main__':
    raise SystemExit(main())
