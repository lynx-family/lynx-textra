# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Compile independent revisions with a shared, bounded Ninja job budget."""
import os
from concurrent.futures import ThreadPoolExecutor


def job_budget(value=None):
    if value is None:
        cpus = len(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else os.cpu_count()
        return max(1, min(8, cpus or 2))
    if value < 1:
        raise ValueError('build job budget must be positive')
    return value


def build_pair(build_one, revisions, jobs=None):
    """Join every compiler, including on failure, before returning to the caller."""
    budget = job_budget(jobs)
    workers = min(2, budget)
    with ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(build_one, revision, max(1, budget // workers))
                   for revision in revisions]
        return [future.result() for future in futures]


def shared_engine_dependencies(source):
    """Compare core/toolchain pins while the host supplies one renderer SDK."""
    import ast
    tree = ast.parse((source / 'DEPS').read_text())
    for statement in tree.body:
        if (isinstance(statement, ast.Assign) and
                any(isinstance(target, ast.Name) and target.id == 'deps'
                    for target in statement.targets) and
                isinstance(statement.value, ast.Dict)):
            values = statement.value
            entries = [(key, value) for key, value in zip(values.keys, values.values)
                       if not (isinstance(key, ast.Constant) and isinstance(key.value, str)
                               and (key.value == 'third_party/skity' or
                                    key.value.startswith('third_party/skity/')))]
            values.keys = [key for key, _ in entries]
            values.values = [value for _, value in entries]
            return ast.dump(tree, include_attributes=False)
    raise ValueError('DEPS must contain a literal deps dictionary')


def verify_shared_dependencies(baseline, candidate):
    if shared_engine_dependencies(baseline) != shared_engine_dependencies(candidate):
        raise ValueError('paired builds require matching engine DEPS')
    if (baseline / '.gn').read_bytes() != (candidate / '.gn').read_bytes():
        raise ValueError('paired builds require matching .gn')
