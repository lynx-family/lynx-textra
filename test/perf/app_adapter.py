#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Serial JSON-lines transport to owned iOS-simulator/Android benchmark apps."""
import argparse
import json
import os
import shlex
import subprocess
import sys
import time
import uuid
from pathlib import Path


def call(argv, **kwargs):
    kwargs.setdefault('timeout', 10)
    if 'input' not in kwargs:
        kwargs.setdefault('stdin', subprocess.DEVNULL)
    return subprocess.run(argv, check=True, capture_output=True, **kwargs).stdout


def android(adb, device, package, request, timeout):
    prefix = [adb, '-s', device]
    def shell(argv, **kwargs):
        return call(prefix+['shell', shlex.join(argv)], **kwargs)
    shell(['am', 'start', '-W', '-n', package+'/com.lynx.textra.perf.PerfActivity'])
    shell(['run-as', package, 'rm', '-f', 'files/response.json'])
    shell(['run-as', package, 'sh', '-c', 'cat > files/request.tmp'],
          input=json.dumps(request).encode())
    shell(['run-as', package, 'mv', 'files/request.tmp', 'files/request.json'])
    deadline = time.monotonic()+timeout
    while time.monotonic() < deadline:
        # shell v2 preserves the remote exit code and separates stderr. exec-out
        # reports missing-file diagnostics on stdout with a successful exit code.
        result = subprocess.run(prefix+['shell', '-T', shlex.join(
                                    ['run-as', package, 'cat', 'files/response.json'])],
                                capture_output=True, timeout=5, stdin=subprocess.DEVNULL)
        if result.returncode == 0 and result.stdout.strip():
            response = json.loads(result.stdout)
            if response.get('request_id') == request['request_id']:
                return response
            if 'error' in response:
                raise RuntimeError(str(response))
        time.sleep(0.01)
    raise TimeoutError('Android benchmark did not respond')


def ios(device, package, request, timeout):
    container = Path(call(['xcrun','simctl','get_app_container',device,package,'data'], text=True).strip())
    directory = container/'Documents'
    directory.mkdir(exist_ok=True)
    output = directory/'response.json'
    output.unlink(missing_ok=True)
    temporary = directory/('request-'+uuid.uuid4().hex+'.tmp')
    temporary.write_text(json.dumps(request))
    temporary.replace(directory/'request.json')
    # SpringBoard's first launch after simulator boot can exceed the generic
    # command limit. Startup is outside measured shaping and response polling;
    # keep it bounded without retrying a request that may already be running.
    call(['xcrun','simctl','launch',device,package], text=True, timeout=30)
    deadline = time.monotonic()+timeout
    while time.monotonic() < deadline:
        if output.exists():
            response = json.loads(output.read_text())
            if response.get('request_id') == request['request_id']:
                return response
            if 'error' in response:
                raise RuntimeError(str(response))
        time.sleep(0.01)
    raise TimeoutError('iOS benchmark did not respond')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform', choices=['android','ios'], required=True)
    parser.add_argument('--device', required=True)
    parser.add_argument('--package', required=True)
    parser.add_argument('--adb', default='adb')
    parser.add_argument('--timeout', type=float, default=20)
    args = parser.parse_args()
    if args.platform == 'android':
        fingerprint = call([args.adb,'-s',args.device,'shell','getprop','ro.build.fingerprint'], text=True).strip()
    else:
        devices = json.loads(call(['xcrun','simctl','list','devices','--json'], text=True))['devices']
        fingerprint = next(runtime for runtime, entries in devices.items()
                           if any(d['udid']==args.device for d in entries))
    try:
        for line in sys.stdin:
            request = json.loads(line)
            response = (android(args.adb,args.device,args.package,request,args.timeout)
                        if args.platform=='android' else
                        ios(args.device,args.package,request,args.timeout))
            response['identity']['device'] = args.platform+'-simulator:'+args.device
            response['identity']['os'] = fingerprint
            print(json.dumps(response), flush=True)
    finally:
        if args.platform == 'android':
            subprocess.run([args.adb,'-s',args.device,'shell','am','force-stop',args.package],
                           capture_output=True, timeout=10, check=False, stdin=subprocess.DEVNULL)
        else:
            subprocess.run(['xcrun','simctl','terminate',args.device,args.package],
                           capture_output=True, timeout=10, check=False, stdin=subprocess.DEVNULL)


if __name__ == '__main__':
    raise SystemExit(main())
