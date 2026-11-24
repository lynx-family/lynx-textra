#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Build two pinned engines with one benchmark and run a paired macOS gate."""
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

from gate import benchmark_digest, digest, run

HERE = Path(__file__).resolve().parent
GN_ARGS = 'is_debug=false use_xcode=true use_flutter_cxx=true dynamic_load_harfbuzz=false'


def command(args, **kwargs):
    return subprocess.check_output([str(x) for x in args], text=True, **kwargs).strip()


def prepare(source, baseline, output):
    if not re.fullmatch('[0-9a-f]{40}', baseline):
        raise ValueError('baseline must be a full commit SHA')
    before = output / 'baseline-source'
    subprocess.run(['git', '-C', str(source), 'worktree', 'add', '--detach', str(before), baseline],
                   env={**os.environ, 'GIT_LFS_SKIP_SMUDGE': '1'}, check=True)
    # Both engines intentionally use the same fetched dependencies/toolchain.
    # Reject dependency-changing PRs rather than silently using the wrong pins.
    for filename in ('DEPS', '.gn'):
        if (before / filename).read_bytes() != (source / filename).read_bytes():
            raise ValueError('paired build requires equal ' + filename)
    for directory in ('build', 'buildtools', 'third_party', 'tools_shared'):
        if (before / directory).exists():
            raise ValueError('dependency path is tracked: ' + directory)
        (before / directory).symlink_to(source / directory, target_is_directory=True)
    return before


def metadata(source, revision, gn_args, skity, harfbuzz, fonts, destination):
    dependencies = {'gn_args': gn_args, 'builder': digest(HERE / 'build_macos.py'), 'deps': digest(source / 'DEPS'),
                    'compiler': command(['xcrun', 'clang++', '--version']),
                    'sdk': command(['xcrun', '--sdk', 'macosx', '--show-sdk-version'])}
    for directory in ('harfbuzz', 'libcxx', 'libcxxabi', 'rapidjson'):
        dependencies[directory] = command(['git', '-C', source / 'third_party' / directory,
                                           'rev-parse', 'HEAD'])
    if harfbuzz:
        dependencies['harfbuzz_library'] = digest(harfbuzz)
    if skity:
        dependencies['skity_library'] = digest(skity)
    identity = hashlib.sha256(json.dumps(dependencies, sort_keys=True).encode()).hexdigest()
    data = dict(revision=revision, dependencies=identity,
                benchmark=benchmark_digest(HERE))
    data['fonts'] = hashlib.sha256(json.dumps([digest(f) for f in fonts]).encode()).hexdigest()
    destination.write_text(json.dumps(data, indent=2))
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=HERE.parents[1])
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--backend', choices=['macos', 'skity'], default='macos')
    parser.add_argument('--skity-library', type=Path)
    parser.add_argument('--harfbuzz-library', type=Path,
                        help='optional pinned dynamic HarfBuzz; default builds bundled HarfBuzz')
    parser.add_argument('--fonts', type=Path, help='JSON array of fixed font file paths for Skity')
    args = parser.parse_args()
    source, output = args.source.resolve(), args.output.resolve()
    if command(['git', '-C', source, 'rev-parse', 'HEAD']) != args.candidate:
        parser.error('checkout does not match candidate SHA')
    if command(['git', '-C', source, 'diff', 'HEAD', '--', 'src', 'public', 'platform',
                'BUILD.gn', 'config.gni']):
        parser.error('candidate engine has uncommitted changes')
    if args.backend == 'skity' and (not args.skity_library or not args.fonts):
        parser.error('Skity requires a shared library and a fixed font manifest')
    # The same source tree supplies headers to the external benchmark only via
    # each engine's own compile command. No candidate engine headers leak into
    # the baseline build.
    output.mkdir(parents=True, exist_ok=False)
    before = output / 'baseline-source'
    gn_args = GN_ARGS
    runtime = []
    if args.harfbuzz_library:
        library = args.harfbuzz_library.resolve()
        if not library.is_file():
            parser.error('dynamic HarfBuzz library missing')
        runtime_dir = output / 'runtime'
        runtime_dir.mkdir()
        (runtime_dir / 'libharfbuzz.so').symlink_to(library)
        runtime = ['/usr/bin/env', 'DYLD_LIBRARY_PATH=' + str(runtime_dir)]
        gn_args = gn_args.replace('dynamic_load_harfbuzz=false', 'dynamic_load_harfbuzz=true')
    config = dict(schema=1, platform='macos', backend=args.backend,
                  benchmark_sha256=benchmark_digest(HERE),
                  fonts=json.loads(args.fonts.read_text()) if args.fonts else [])
    config['fonts'] = [str((source / f).resolve()) for f in config['fonts']]
    for font in config['fonts']:
        if not Path(font).is_file():
            parser.error('fixed font unavailable: ' + font)
    try:
        prepare(source, args.baseline, output)
        for version, tree, revision in [('baseline', before, args.baseline),
                                         ('candidate', source, args.candidate)]:
            build = output / (version + '-build')
            subprocess.run([str(source / 'buildtools/gn/gn'), 'gen', str(build),
                            '--root=' + str(tree), '--args=' + gn_args], check=True)
            executable = output / version
            build_command = [sys.executable, str(HERE / 'build_macos.py'), '--source', str(tree),
                             '--build', str(build), '--output', str(executable)]
            if args.skity_library:
                build_command += ['--skity-library', str(args.skity_library.resolve()),
                                  '--skity-include', str(source / 'third_party/skity/include')]
            subprocess.run(build_command, check=True)
            meta = output / (version + '.metadata.json')
            metadata(source, revision, gn_args, args.skity_library, args.harfbuzz_library, config['fonts'], meta)
            artifacts = [dict(path=str(path), sha256=digest(path))
                         for path in [executable, meta] +
                         ([args.skity_library.resolve()] if args.skity_library else []) +
                         ([args.harfbuzz_library.resolve()] if args.harfbuzz_library else []) +
                         [Path(f) for f in config['fonts']]]
            config[version] = dict(revision=revision, protocol='jsonl',
                                   command=runtime + [str(executable), str(meta), '--server'], artifacts=artifacts)
        (output / 'config.json').write_text(json.dumps(config, indent=2))
        manifest = json.loads((HERE / 'cases.json').read_text())
        config['cases_sha256'] = digest(HERE / 'cases.json')
        result = run(config, manifest['cases'], output / 'results')
        return 0 if result['passed'] else 1
    finally:
        # Leave binaries/reports as artifacts, remove only this owned checkout.
        subprocess.run(['git', '-C', str(source), 'worktree', 'remove', '--force', str(before)], check=False)


if __name__ == '__main__':
    raise SystemExit(main())
