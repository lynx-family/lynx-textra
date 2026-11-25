#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Compare exact artifact bytes; size growth is advisory, never a gate failure."""
import hashlib
import zipfile
from pathlib import Path

THRESHOLD = 10 * 1024


def artifact(path):
    data = Path(path).read_bytes()
    if not data:
        raise ValueError('empty artifact: ' + str(path))
    return dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def android_artifacts(path):
    result = {'release.aar': artifact(path)}
    with zipfile.ZipFile(path) as archive:
        seen = set()
        for entry in archive.infolist():
            if entry.filename in seen:
                raise ValueError('duplicate AAR entry: ' + entry.filename)
            seen.add(entry.filename)
            if entry.filename.startswith('jni/') and entry.filename.endswith('.so'):
                data = archive.read(entry)
                if not data:
                    raise ValueError('empty native library: ' + entry.filename)
                result[entry.filename] = dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
    for abi in ('arm64-v8a', 'armeabi-v7a', 'x86'):
        if 'jni/' + abi + '/liblynxtextra.so' not in result:
            raise ValueError('AAR missing ABI: ' + abi)
    return result


def compare(baseline, candidate):
    if not baseline or not candidate:
        raise ValueError('no artifacts to compare')
    if any(item['bytes'] <= 0 for items in (baseline, candidate) for item in items.values()):
        raise ValueError('artifact sizes must be positive')
    rows = []
    for name in sorted(baseline.keys() | candidate.keys()):
        before = baseline.get(name, dict(bytes=0, sha256=None))
        after = candidate.get(name, dict(bytes=0, sha256=None))
        delta = after['bytes'] - before['bytes']
        rows.append(dict(artifact=name, baseline=before, candidate=after,
                         delta=delta, percent=100 * delta / before['bytes'] if before['bytes'] else None,
                         warning=delta > THRESHOLD))
    return rows


def markdown(report):
    lines = ['## ' + report['platform'] + ' package size', '',
             'Baseline: `' + report['baseline'] + '`; candidate: `' + report['candidate'] + '`.', '',
             'Advisory threshold: growth greater than 10 KiB (10,240 bytes).', '']
    if report['status'] == 'error':
        lines += ['**Comparison unavailable:** ' + report['error'], '']
    else:
        lines += ['| Artifact | Base bytes | Head bytes | Delta bytes | Change | Result |',
                  '|---|---:|---:|---:|---:|---|']
        for row in report['artifacts']:
            percent = '{:+.2f}%'.format(row['percent']) if row['percent'] is not None else 'new'
            lines.append('| {} | {} | {} | {:+d} | {} | {} |'.format(
                row['artifact'], row['baseline']['bytes'], row['candidate']['bytes'],
                row['delta'], percent, 'WARNING' if row['warning'] else 'OK'))
        lines.append('')
    return '\n'.join(lines)
