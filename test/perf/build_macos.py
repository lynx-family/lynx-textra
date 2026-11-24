#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Link the same external benchmark TU against either macOS GN source tree."""
import argparse
import json
import platform
import shlex
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--skity-library', type=Path)
    parser.add_argument('--skity-include', type=Path)
    args = parser.parse_args()
    source, build, output = args.source.resolve(), args.build.resolve(), args.output.resolve()
    if platform.system() != 'Darwin':
        parser.error('macOS builder requires a macOS host')
    output.parent.mkdir(parents=True, exist_ok=True)
    lines = subprocess.check_output([str(source/'buildtools/ninja/ninja'), '-C', str(build), '-t', 'commands', 'lynxtextra'], text=True).splitlines()
    command = shlex.split(next(x for x in lines if ' -c ' in x and x.endswith('textlayout.tt_shaper.o')))
    compiler = command[0]
    # Historical engine revisions trigger warnings under newer Apple Clang.
    # Keep diagnostics, but avoid making this timing job a duplicate warning
    # gate. These narrow flags do not affect code generation and are applied to
    # BOTH builds, only in owned generated Ninja files (never engine sources).
    warning_flags = []
    for warning in ['shorten-64-to-32', 'nontrivial-memcall']:
        flag = '-Wno-error=' + warning
        probe = subprocess.run([compiler, '-Werror', flag, '-fsyntax-only', '-x', 'c++', '-'],
                               input='', text=True, capture_output=True)
        if not probe.returncode:
            warning_flags.append(flag)
    for ninja_file in (build/'obj').rglob('*.ninja'):
        text = ninja_file.read_text()
        text = '\n'.join(line + ' ' + ' '.join(warning_flags)
                         if line.startswith('cflags_cc = ') and not all(f in line for f in warning_flags)
                         else line for line in text.split('\n'))
        ninja_file.write_text(text)
    subprocess.run([str(source/'buildtools/ninja/ninja'), '-C', str(build), 'lynxtextra'], check=True)
    command[command.index('-c')+1] = str(Path(__file__).with_name('shaping_benchmark.cc').resolve())
    command[command.index('-o')+1] = str(output)+'.o'
    command[command.index('-MF')+1] = str(output)+'.d'
    header = output.parent/'harness/test/perf/shaping_benchmark.h'
    header.parent.mkdir(parents=True, exist_ok=True)
    header.write_bytes(Path(__file__).with_name('shaping_benchmark.h').read_bytes())
    command += ['-I'+str(source/'third_party/rapidjson/include'),
                '-I'+str(output.parent/'harness')]
    if bool(args.skity_library) != bool(args.skity_include):
        parser.error('supply both Skity library and headers')
    if args.skity_library:
        command += ['-DTEXTRA_PERF_SKITY', '-I'+str(args.skity_include.resolve())]
    subprocess.run(command, cwd=build, check=True)
    target = build/'obj/platform/mac/lynxtextra.ninja'
    line = next(x for x in target.read_text().splitlines() if ': solink ' in x)
    objects = shlex.split(line.split(': solink ', 1)[1].split(' || ')[0].split(' | ')[0])
    sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'], text=True).strip()
    link = [compiler, '-isysroot', sdk, '-arch', platform.machine(), '-nostdlib++', '-Wl,-dead_strip', '-o', str(output), str(output)+'.o'] + objects
    if not any('obj/third_party/libcxx/' in obj for obj in objects):
        link += ['-lc++']
    if args.skity_library:
        link += [str(args.skity_library.resolve()), '-Wl,-rpath,'+str(args.skity_library.resolve().parent)]
    link += ['-framework', 'AppKit', '-framework', 'CoreText', '-framework', 'CoreGraphics']
    subprocess.run(link, cwd=build, check=True)
    (Path(str(output)+'.build.json')).write_text(json.dumps(dict(source=str(source), build=str(build), compile=command, link=link, warning_flags=warning_flags), indent=2))


if __name__ == '__main__':
    main()
