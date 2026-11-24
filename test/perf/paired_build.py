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
