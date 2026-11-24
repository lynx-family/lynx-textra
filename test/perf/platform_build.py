#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Shared owned-checkout/GN build support for the platform benchmark hosts."""
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

from gate import benchmark_digest, digest

HERE = Path(__file__).resolve().parent
SHARED = ('build', 'buildtools', 'third_party', 'tools_shared')


def execute(args, **kwargs):
    return subprocess.check_output([str(x) for x in args], text=True, **kwargs).strip()


def checkout(source, revision, destination):
    subprocess.run(['git', '-C', str(source), 'worktree', 'add', '--detach',
                    str(destination), revision], env={**os.environ, 'GIT_LFS_SKIP_SMUDGE': '1'}, check=True)
    for name in ('DEPS', '.gn'):
        if (destination/name).read_bytes() != (source/name).read_bytes():
            raise ValueError('paired builds require matching ' + name)
    for name in SHARED:
        link, target = destination/name, source/name
        if link.exists():
            raise ValueError('dependency directory is tracked: ' + name)
        if os.name == 'nt':
            subprocess.run(['cmd', '/c', 'mklink', '/J', str(link), str(target)], check=True)
        else:
            link.symlink_to(target, target_is_directory=True)
    return destination


def cleanup(source, tree):
    if os.name == 'nt':
        for name in SHARED:
            link = tree/name
            if link.exists():
                os.rmdir(link)  # Remove the owned junction, not its target.
    subprocess.run(['git', '-C', str(source), 'worktree', 'remove', '--force', str(tree)], check=False)


def overlay(tree):
    destination = tree/'test/perf'
    if destination.resolve() != HERE:
        destination.mkdir(parents=True, exist_ok=True)
        for name in ['BUILD.gn', 'shaping_benchmark.cc', 'shaping_benchmark.h']:
            shutil.copy2(HERE/name, destination/name)
        shutil.copytree(HERE/'hosts', destination/'hosts', dirs_exist_ok=True)


def build(source, tree, output, arguments):
    overlay(tree)
    root_build = tree/'BUILD.gn'
    root_build.write_text(root_build.read_text() +
        '\ngroup("textra_performance") { deps = [ "//test/perf:perf_host" ] }\n')
    suffix = '.exe' if os.name == 'nt' else ''
    gn = source/('buildtools/gn/gn' + suffix)
    ninja = source/('buildtools/ninja/ninja' + suffix)
    flags = ' '.join(k+'='+json.dumps(v) for k,v in arguments.items())
    subprocess.run([str(gn), 'gen', str(output), '--root='+str(tree), '--args='+flags], check=True)
    # Keep historical integer-narrowing diagnostics without turning the timing
    # job into another warnings gate. Apply identically to both owned builds.
    for path in (output/'obj').rglob('*.ninja'):
        text = path.read_text()
        text = '\n'.join(line+' -Wno-error=shorten-64-to-32'
                         if line.startswith('cflags_cc = ') else line
                         for line in text.split('\n'))
        path.write_text(text)
    subprocess.run([str(ninja), '-C', str(output), 'perf_host'], check=True)
    (output/'perf-build.json').write_text(json.dumps(dict(source=str(tree), args=arguments,
        harness=benchmark_digest(HERE), command=[str(ninja), '-C', str(output), 'perf_host']), indent=2))
    return output/('perf_host.exe' if os.name == 'nt' else 'perf_host')


def write_metadata(source, revision, arguments, destination, extra=None, fonts=()):
    dependencies = dict(deps=digest(source/'DEPS'), args=arguments,
                        builder=digest(HERE/'platform_build.py'), extra=extra or {})
    for name in ['build', 'third_party/harfbuzz', 'third_party/libcxx', 'third_party/rapidjson']:
        dependencies[name] = execute(['git', '-C', source/name, 'rev-parse', 'HEAD'])
    data = dict(revision=revision, benchmark=benchmark_digest(HERE),
        dependencies=hashlib.sha256(json.dumps(dependencies, sort_keys=True).encode()).hexdigest(),
        fonts=hashlib.sha256(json.dumps([digest(f) for f in fonts]).encode()).hexdigest())
    destination.write_text(json.dumps(data, indent=2))
    return destination


def artifacts(paths):
    return [dict(path=str(Path(f).resolve()), sha256=digest(f)) for f in paths]
