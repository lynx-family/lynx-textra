#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Fetch fixed Noto test fonts without depending on Windows language packs."""
import hashlib
import json
import urllib.request
from pathlib import Path


def fetch(destination, manifest=None):
    manifest = manifest or Path(__file__).with_name('fonts.lock.json')
    lock = json.loads(manifest.read_text())
    if lock['schema'] != 1:
        raise ValueError('unsupported font lock schema')
    destination.mkdir(parents=True, exist_ok=True)
    fonts = []
    for item in lock['fonts']:
        name = item['name']
        if Path(name).name != name or '/' in name or '\\' in name:
            raise ValueError('font filename must be a basename')
        if '/'+lock['source_commit']+'/' not in item['url']:
            raise ValueError('font URL is not pinned to the declared source revision')
        path = destination/name
        if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest()==item['sha256']:
            fonts.append(path);continue
        data = urllib.request.urlopen(item['url'],timeout=120).read()
        if hashlib.sha256(data).hexdigest()!=item['sha256']:
            raise ValueError('downloaded font checksum mismatch: '+name)
        temporary = destination/(name+'.tmp')
        temporary.write_bytes(data);temporary.replace(path)
        fonts.append(path)
    return fonts
