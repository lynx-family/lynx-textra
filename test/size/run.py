#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Build comparable Release libraries from two pinned revisions, then report size."""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

from report import android_artifacts, artifact, compare, markdown, THRESHOLD

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / 'perf'))
from paired_build import build_pair
from platform_build import checkout, cleanup, execute


def windows_exports(tree):
    names = set()
    for header in (tree / 'public/textra/capi').glob('*.h'):
        text = re.sub(r'/\*.*?\*/|//[^\n]*', '', header.read_text(), flags=re.S)
        names.update(re.findall(r'TEXTRA_API\s+[^;{}]*?\b(textra_[a-z0-9_]+)\s*\(', text))
    if not names:
        raise ValueError('no C API exports found')
    return 'EXPORTS\n' + '\n'.join(sorted(names)) + '\n'


def native_build(source, tree, output, platform, cpu, jobs):
    arguments = dict(is_debug=False, use_flutter_cxx=False, target_cpu=cpu)
    if platform == 'windows':
        arguments.update(target_os='win', is_clang=True)
        adapter = tree / 'test/size'
        adapter.mkdir(parents=True, exist_ok=True)
        shutil.copy2(HERE / 'BUILD.gn', adapter / 'BUILD.gn')
        (adapter / 'textra.def').write_text(windows_exports(tree))
        root = tree / 'BUILD.gn'
        root.write_text(root.read_text() + '\ngroup("size_check") { deps = [ "//test/size:size_library" ] }\n')
        target, filename = 'test/size:size_library', 'lynxtextra.dll'
    else:
        arguments.update(use_xcode=True, use_flutter_cxx=True)
        target, filename = 'platform/mac:lynxtextra', 'liblynxtextra.dylib'
    flags = ' '.join(key + '=' + json.dumps(value) for key, value in arguments.items())
    suffix = '.exe' if os.name == 'nt' else ''
    subprocess.run([str(source / ('buildtools/gn/gn' + suffix)), 'gen', str(output),
                    '--root=' + str(tree), '--args=' + flags], check=True)
    # Match the paired benchmark's historical-build compatibility: retain
    # diagnostics without making size measurement a duplicate warning gate.
    diagnostics = ['-Wno-error=shorten-64-to-32']
    if platform == 'macos':
        flag = '-Wno-error=nontrivial-memcall'
        probe = subprocess.run(['xcrun', 'clang++', '-Werror', flag, '-fsyntax-only', '-x', 'c++', '-'],
                               input='', text=True, capture_output=True)
        if probe.returncode == 0:
            diagnostics.append(flag)
    for path in (output / 'obj').rglob('*.ninja'):
        path.write_text('\n'.join(line + ' ' + ' '.join(diagnostics)
                                  if line.startswith('cflags_cc = ') else line
                                  for line in path.read_text().split('\n')))
    subprocess.run([str(source / ('buildtools/ninja/ninja' + suffix)), '-C', str(output),
                    '-j', str(jobs), target], check=True)
    raw = output / filename
    if platform == 'macos':
        stripped = output / ('stripped-' + filename)
        subprocess.run(['xcrun', 'strip', '-S', '-x', '-o', str(stripped), str(raw)], check=True)
        raw = stripped
    return raw, dict(gn=arguments, diagnostics=diagnostics)


def android_build(tree, output, jobs):
    project = tree / 'platform/android'
    sdk = os.environ.get('ANDROID_HOME') or os.environ.get('ANDROID_SDK_ROOT')
    if not sdk:
        raise ValueError('Android SDK unavailable')
    (project / 'local.properties').write_text('sdk.dir=' + Path(sdk).as_posix() + '\n')
    # Bound the external Ninja compiler as well as Gradle's worker pool. Apply
    # identical scheduling-only settings to both owned checkouts.
    cmake = project / 'lynxtextra/CMakeLists.txt'
    text = cmake.read_text()
    marker = 'project(lynxtextra)'
    if text.count(marker) != 1:
        raise ValueError('unrecognized Android CMake project')
    text = text.replace(marker, marker + '\nset_property(GLOBAL PROPERTY JOB_POOLS size_compile=' +
                        str(jobs) + ' size_link=1)\nset(CMAKE_JOB_POOL_COMPILE size_compile)\n' +
                        'set(CMAKE_JOB_POOL_LINK size_link)')
    cmake.write_text(text)
    subprocess.run([str(project / 'gradlew'), ':lynxtextra:assembleRelease', '--no-daemon',
                    '--max-workers=' + str(jobs)], cwd=project, check=True)
    aar = project / 'lynxtextra/build/outputs/aar/lynxtextra-release.aar'
    destination = output / 'lynxtextra-release.aar'
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(aar, destination)
    (output / 'build.json').write_text(json.dumps(dict(
        command=[':lynxtextra:assembleRelease', '--no-daemon', '--max-workers=' + str(jobs)],
        sdk=str(Path(sdk).resolve()), jdk=os.environ.get('JAVA_HOME'),
        configuration='platform/android Release AAR; all configured ABIs'), indent=2))
    return destination


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform', choices=['android', 'macos', 'windows'], required=True)
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--source', type=Path, default=HERE.parents[1])
    parser.add_argument('--jobs', type=int)
    args = parser.parse_args()
    source, output = args.source.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report = dict(schema=1, platform=args.platform, baseline=args.baseline, candidate=args.candidate,
                  threshold_bytes=THRESHOLD, status='error', artifacts=[])
    trees = []
    try:
        if not all(re.fullmatch('[0-9a-f]{40}', sha) for sha in (args.baseline, args.candidate)):
            raise ValueError('full revision SHAs are required')
        if execute(['git', '-C', source, 'rev-parse', 'HEAD']) != args.candidate:
            raise ValueError('candidate does not match checkout')
        if execute(['git', '-C', source, 'diff', 'HEAD', '--', 'src', 'public', 'platform', 'config.gni', 'BUILD.gn']):
            raise ValueError('candidate engine has uncommitted changes')
        revisions = [('baseline', args.baseline), ('candidate', args.candidate)]
        for version, revision in revisions:
            tree = output / (version + '-source')
            trees.append(tree)
            checkout(source, revision, tree)

        def compile_revision(item, jobs):
            version, _ = item
            tree, build = output / (version + '-source'), output / (version + '-build')
            if args.platform == 'android':
                return android_artifacts(android_build(tree, build, jobs))
            result, configurations = {}, {}
            for cpu in (('arm64', 'x64') if args.platform == 'macos' else ('x64',)):
                binary, flags = native_build(source, tree, build / cpu, args.platform, cpu, jobs)
                result[cpu + '/' + binary.name] = artifact(binary)
                configurations[cpu] = flags
            (build / 'build.json').write_text(json.dumps(configurations, indent=2))
            return result

        baseline, candidate = build_pair(compile_revision, revisions, args.jobs)
        report['artifacts'] = compare(baseline, candidate)
        report['status'] = 'warning' if any(row['warning'] for row in report['artifacts']) else 'ok'
        for row in report['artifacts']:
            if row['warning']:
                print('::warning::{} package size: {} grew by {} bytes (>10 KiB)'.format(
                    args.platform, row['artifact'], row['delta']))
        return 0
    except Exception as error:
        report['error'] = str(error)
        print('::warning::Package size comparison unavailable; see size report and build logs.')
        raise
    finally:
        (output / 'report.json').write_text(json.dumps(report, indent=2))
        (output / 'summary.md').write_text(markdown(report))
        for tree in trees:
            cleanup(source, tree)


if __name__ == '__main__':
    raise SystemExit(main())
