#!/usr/bin/env python3
"""Install a test APK and capture independent, warmed, rotated shaping runs."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apk', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--adb', default='adb')
    parser.add_argument('--serial')
    parser.add_argument('--package', default='com.lynx.textra.capidemo')
    parser.add_argument('--rounds', type=int, choices=range(3, 32), default=21)
    parser.add_argument('--rotations', type=int, choices=range(5), nargs='+', default=[0, 2, 4])
    parser.add_argument('--timeout', type=float, default=120)
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    if args.output.exists() and any(args.output.iterdir()):
        parser.error('--output must be empty to preserve previous measurements')
    args.output.mkdir(parents=True, exist_ok=True)
    adb = [args.adb] + (['-s', args.serial] if args.serial else [])

    def run(*arguments, **kwargs):
        return subprocess.run(adb + list(arguments), check=True, **kwargs)

    apk = args.apk.resolve()
    manifest = dict(apk=str(apk), apk_sha256=hashlib.sha256(apk.read_bytes()).hexdigest(),
                    rounds=args.rounds, rotations=args.rotations, runs=[])
    with zipfile.ZipFile(apk) as archive:
        manifest['binary_sha256'] = {name: hashlib.sha256(archive.read(name)).hexdigest()
                                    for name in ('classes.dex', 'lib/arm64-v8a/liblynxtextra.so',
                                                 'lib/arm64-v8a/libandroid_test.so')}
    manifest['device'] = {key: run('shell', 'getprop', key, capture_output=True, text=True).stdout.strip()
                          for key in ('ro.build.version.release', 'ro.build.version.sdk',
                                      'ro.product.cpu.abi', 'ro.product.model')}
    run('install', '-r', str(apk))
    manifest_file = args.output / 'manifest.json'
    manifest_file.write_text(json.dumps(manifest, indent=2) + "\n")
    reports = []
    for number, rotation in enumerate(args.rotations, 1):
        run('shell', 'run-as', args.package, 'rm', '-f', 'files/shaping-benchmark.json')
        run('shell', 'am', 'start', '-S', '-n', args.package + '/com.lynx.textra.demo.CapiDemoActivity',
            '--ez', 'shaping_benchmark', 'true', '--ei', 'rounds', str(args.rounds),
            '--ei', 'workload_rotation', str(rotation), stdout=subprocess.DEVNULL)
        deadline = time.monotonic() + args.timeout
        pid = None
        while time.monotonic() < deadline:
            state = subprocess.run(adb + ['shell', 'pidof', args.package], capture_output=True, text=True)
            if state.returncode == 0:
                pid = state.stdout.strip()
                break
            time.sleep(0.25)
        if not pid or any(item['pid'] == pid for item in manifest['runs']):
            raise RuntimeError('A fresh application process did not start')
        while time.monotonic() < deadline:
            result = subprocess.run(adb + ['exec-out', 'run-as', args.package, 'cat',
                                           'files/shaping-benchmark.json'], capture_output=True)
            try:
                data = json.loads(result.stdout)
            except (ValueError, UnicodeDecodeError):
                time.sleep(0.5)
                continue
            report = args.output / f'run-{number}.json'
            report.write_bytes(result.stdout)
            if (data.get('version') != 4 or data.get('global_warmup_ms') != 2000
                    or data.get('workload_rotation') != rotation or data.get('rounds') != args.rounds
                    or not data.get('results_equal') or data.get('cleanup_status', data.get('unregister_status')) != 0):
                raise RuntimeError(f'Incomplete or failed benchmark: {report}')
            reports.append(report)
            manifest['runs'].append(dict(run=number, rotation=rotation, pid=pid, report=report.name))
            manifest_file.write_text(json.dumps(manifest, indent=2) + '\n')
            print(f'Run {number}: rotation={rotation}, pid={pid}, outputs equal', flush=True)
            break
        else:
            raise RuntimeError(f'Benchmark timed out after {args.timeout}s')
    subprocess.run([sys.executable, str(Path(__file__).with_name('summarize_android_shaping.py')),
                    *map(str, reports), '--output', str(args.output / 'summary.json')], check=True)


if __name__ == '__main__':
    main()
