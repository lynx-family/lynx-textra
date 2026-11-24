#!/usr/bin/env python3
"""Summarize paired samples from independent shaping benchmark processes."""
import argparse
import json
from pathlib import Path
import statistics

REPORT_VERSION = 6


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('runs', type=Path, nargs='+')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    runs = [json.loads(path.read_text()) for path in args.runs]
    if not runs:
        raise ValueError('No benchmark runs')
    version = runs[0].get('version')
    if version not in (2, 3, 4, REPORT_VERSION):
        raise ValueError('Unsupported benchmark version')
    fields = ('rounds', 'batch', 'warmup') + (('global_warmup_ms',) if version >= 3 else ())
    key = ('case', 'codepoints', 'metrics_round', 'mode')
    row_key = lambda row: tuple(row[k] for k in key)
    indexed = []
    for run in runs:
        if run.get('version') != version or not run.get('results_equal') or run.get('cleanup_status', run.get('unregister_status', 0)) != 0 or len(run.get('rows', [])) != 40:
            raise ValueError('Incomplete/incompatible benchmark or unequal shaping output')
        if any(run.get(k) != runs[0].get(k) for k in fields):
            raise ValueError('Runs use different measurement parameters')
        rows_by_key = {row_key(row): row for row in run['rows']}
        if len(rows_by_key) != 40 or (indexed and rows_by_key.keys() != indexed[0].keys()):
            raise ValueError('Duplicate or mismatched workloads')
        for row in run['rows']:
            if not row.get('results_equal') or len(row['cpp']) != run['rounds'] or len(row['capi']) != run['rounds']:
                raise ValueError('Incomplete samples or unequal workload output')
        indexed.append(rows_by_key)
    median = statistics.median
    output = []
    for row in runs[0]['rows']:
        rows = [index[row_key(row)] for index in indexed]
        a = [value for other in rows for value in other['cpp']]
        b = [value for other in rows for value in other['capi']]
        if len(a) != len(b) or any(value <= 0 for value in a+b):
            raise ValueError('Invalid paired samples')
        record = {k: row[k] for k in key}
        record.update(
            cpp_us=median(a), capi_us=median(b),
            paired_delta_us=median([y-x for x,y in zip(a,b)]),
            paired_delta_percent=100*median([y/x-1 for x,y in zip(a,b)]),
            per_process_delta_percent=[100*median([y/x-1 for x,y in zip(other['cpp'],other['capi'])]) for other in rows],
            cpp_cleanup_us=median([value for other in rows for value in other['cpp_cleanup']]),
            capi_cleanup_us=median([value for other in rows for value in other['capi_cleanup']]))
        for name in ('cpp_setup', 'capi_setup'):
            values = [value for other in rows for value in other.get(name, [])]
            if values:
                record[name + '_us'] = median(values)
        output.append(record)
    summary = dict(version=version, runs=len(runs), rounds_per_run=runs[0]['rounds'], batch=runs[0]['batch'], warmup=runs[0]['warmup'],
                   results_equal=True, unit='microseconds_per_call', percent_method='median of paired block-mean ratios',
                   rows=output)
    if version >= 3:
        summary.update(global_warmup_ms=runs[0]['global_warmup_ms'],
                       workload_rotations=[run.get('workload_rotation') for run in runs])
    args.output.write_text(json.dumps(summary, ensure_ascii=False, indent=2)+'\n')
    for row in output:
        if row['mode']=='engine_cache_miss':
            print(f"{row['case']:14} round={row['metrics_round']}  delta={row['paired_delta_us']:+.3f} us  {row['paired_delta_percent']:+.1f}%")

if __name__ == '__main__':
    main()
