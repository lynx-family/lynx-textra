#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Run paired, cache-disabled shaping cases through local or device adapters."""
import argparse
import hashlib
import json
import math
import os
import queue
import re
import signal
import statistics
import subprocess
import threading
import uuid
from pathlib import Path

PLATFORMS = {'android', 'ios', 'macos', 'harmony', 'windows'}
IDENTITY = ('device', 'os', 'arch', 'backend', 'fonts', 'dependencies', 'benchmark')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def benchmark_digest(directory):
    return hashlib.sha256(''.join(digest(Path(directory) / name) for name in
                          ['shaping_benchmark.cc', 'shaping_benchmark.h']).encode()).hexdigest()


def validate_config(config):
    if config.get('schema') != 1 or config.get('platform') not in PLATFORMS:
        raise ValueError('unsupported configuration schema/platform')
    for version in ('baseline', 'candidate'):
        item = config[version]
        if not re.fullmatch(r'[0-9a-f]{40}', item['revision']):
            raise ValueError('pin full commit SHAs')
        if not isinstance(item.get('command'), list) or not item['command'] or not all(isinstance(x, str) and x for x in item['command']):
            raise ValueError('command must be an argv array, never a shell string')
        if item.get('protocol', 'oneshot') not in {'oneshot', 'jsonl'}:
            raise ValueError('unknown adapter protocol')
        if not item.get('artifacts'):
            raise ValueError('declare measured executable/app artifacts')
        for artifact in item['artifacts']:
            if digest(artifact['path']) != artifact['sha256']:
                raise ValueError('artifact checksum mismatch: ' + artifact['path'])
    if config['baseline']['revision'] == config['candidate']['revision'] and not config.get('aa_control'):
        raise ValueError('same revision requires explicit aa_control')
    if config.get('backend') not in {'skity', 'macos', 'ios', 'android', 'harmony'}:
        raise ValueError('unsupported shaping backend')
    for key, default, minimum in [('rounds', 8, 8), ('batches', 6, 3),
                                   ('iterations', 100, 1), ('warmup', 20, 1)]:
        value = config.get(key, default)
        if type(value) is not int or value < minimum:
            raise ValueError('invalid sampling option: ' + key)
    for key, default in [('relative', 0.05), ('absolute_us', 1.0)]:
        value = config.get('policy', {}).get(key, default)
        if type(value) not in (float, int) or not math.isfinite(value) or value < 0:
            raise ValueError('invalid regression threshold')
    timeout = config.get('timeout', 30)
    if type(timeout) not in (float, int) or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError('invalid timeout')
    if 'aa_control' in config and type(config['aa_control']) is not bool:
        raise ValueError('aa_control must be a boolean')


def validate_response(response, request):
    if not isinstance(response, dict):
        raise ValueError('benchmark response must be an object')
    if response.get('revision') != request['revision']:
        raise ValueError('wrong engine revision')
    if response.get('schema') != 1 or response.get('request_id') != request['request_id']:
        raise ValueError('stale or incompatible benchmark response')
    if response.get('case') != request['case']['id'] or response.get('cache_disabled') is not True:
        raise ValueError('case/cache policy mismatch')
    identity = response['identity']
    if request.get('benchmark') and identity.get('benchmark') != request['benchmark']:
        raise ValueError('wrong benchmark source')
    if not all(isinstance(identity.get(k), str) and identity[k] for k in IDENTITY):
        raise ValueError('incomplete device/backend/font/dependency identity')
    if identity['backend'] != request['backend']:
        raise ValueError('unexpected shaping backend')
    for key in ('glyphs', 'missing'):
        value = response['output'][key]
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise ValueError('invalid output counts')
    if not isinstance(response['output'].get('signature'), str) or not response['output']['signature']:
        raise ValueError('missing output signature')
    batches = response['batches']
    if len(batches) != request['batches']:
        raise ValueError('incomplete timing batches')
    for batch in batches:
        for phase in ('shape', 'cleanup', 'total'):
            value = batch[phase]
            if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
                raise ValueError('non-positive/non-finite timing')
        if not math.isclose(batch['shape'] + batch['cleanup'], batch['total'], rel_tol=0.001, abs_tol=0.001):
            raise ValueError('timing phases do not span directly measured total')
    return response


def invoke(command, request, timeout):
    # A device adapter can spawn children. Kill the whole local process group on
    # timeout so an old benchmark cannot contaminate later paired measurements.
    with subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, text=True,
                          start_new_session=os.name != 'nt') as process:
        try:
            stdout, stderr = process.communicate(json.dumps(request), timeout=timeout)
        except BaseException:
            if os.name == 'nt':
                subprocess.run(['taskkill', '/F', '/T', '/PID', str(process.pid)],
                               capture_output=True, check=False)
            else:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
            process.communicate()
            raise
        return subprocess.CompletedProcess(command, process.returncode, stdout, stderr)


class JsonLinesAdapter:
    """One serial engine process per round; restart after a failed request."""
    def __init__(self, command, stderr_path):
        self.command = command
        self.stderr = stderr_path.open('a')
        self.process = None

    def close(self, force=False):
        if self.process is not None:
            if not force and self.process.poll() is None:
                # EOF lets app launchers stop their native process in finally.
                try:
                    self.process.stdin.close()
                    self.process.wait(timeout=10)
                except (OSError, subprocess.TimeoutExpired):
                    pass
            if self.process.poll() is None:
                if os.name == 'nt':
                    subprocess.run(['taskkill', '/F', '/T', '/PID', str(self.process.pid)],
                                   capture_output=True, check=False)
                else:
                    try:
                        os.killpg(self.process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
            self.process.wait()
            self.reader.join(timeout=1)
            if not self.process.stdin.closed:
                self.process.stdin.close()
            self.process.stdout.close()
            self.process = None

    def invoke(self, request, timeout):
        if self.process is None:
            self.responses = queue.Queue()
            self.process = subprocess.Popen(self.command, stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, stderr=self.stderr, text=True, bufsize=1,
                start_new_session=os.name != 'nt')
            responses, stdout = self.responses, self.process.stdout
            def read():
                for line in stdout:
                    responses.put(line)
                responses.put(None)
            self.reader = threading.Thread(target=read, daemon=True)
            self.reader.start()
        try:
            self.process.stdin.write(json.dumps(request) + '\n')
            self.process.stdin.flush()
            line = self.responses.get(timeout=timeout)
            if line is None:
                code = self.process.wait()
                raise ValueError('persistent adapter exited ' + str(code))
            return subprocess.CompletedProcess(self.command, 0, line, '')
        except queue.Empty:
            self.close(force=True)
            raise subprocess.TimeoutExpired(self.command, timeout)
        except BaseException:
            self.close(force=True)
            raise


def classify(before, after, policy):
    ratios = [(b / a - 1) for a, b in zip(before, after)]
    ratio = statistics.median(ratios)
    absolute = statistics.median(b - a for a, b in zip(before, after))
    slower = sum(x > 0 for x in ratios)
    regression = (ratio > policy.get('relative', 0.05)
                  and absolute > policy.get('absolute_us', 1.0)
                  and slower >= math.ceil(len(ratios) * 7 / 8))
    return dict(baseline_us=statistics.median(before), candidate_us=statistics.median(after),
                paired_percent=ratio * 100, paired_absolute_us=absolute,
                slower_pairs=slower, regression=regression)


def run(config, cases, output):
    validate_config(config)
    output.mkdir(parents=True, exist_ok=False)
    (output / 'config.json').write_text(json.dumps(config, indent=2))
    records, issues, reference = [], [], {}
    unavailable = set()
    rounds = config.get('rounds', 8)
    expected_identity = None
    for trial in range(rounds):
        adapters = {v: JsonLinesAdapter(config[v]['command'], output / f'{trial}-{v}.host.stderr')
                    for v in ('baseline', 'candidate') if config[v].get('protocol') == 'jsonl'}
        try:
            rotated = cases[trial % len(cases):] + cases[:trial % len(cases)]
            for case in rotated:
                for version in (['baseline', 'candidate'] if trial % 2 == 0 else ['candidate', 'baseline']):
                    if (version, case['id']) in unavailable:
                        continue
                    request = dict(schema=1, request_id=uuid.uuid4().hex, case=case,
                                   codepoints=list(map(ord, case['text'])), backend=config['backend'],
                                   warmup=config.get('warmup', 20), batches=config.get('batches', 6),
                                   iterations=config.get('iterations', 100), cache_disabled=True,
                                   fonts=config.get('fonts', []), revision=config[version]['revision'],
                                   benchmark=config.get('benchmark_sha256'))
                    label = f'{trial}-{version}-{case["id"]}'
                    try:
                        result = (adapters[version].invoke(request, config.get('timeout', 30))
                                  if version in adapters else
                                  invoke(config[version]['command'], request, config.get('timeout', 30)))
                        (output / (label + '.stdout')).write_text(result.stdout)
                        (output / (label + '.stderr')).write_text(result.stderr)
                        if result.returncode:
                            raise ValueError('adapter exited ' + str(result.returncode))
                        response = validate_response(json.loads(result.stdout), request)
                        identity = {k: response['identity'][k] for k in IDENTITY}
                        if expected_identity is None:
                            expected_identity = identity
                        if identity != expected_identity:
                            raise ValueError('device/OS/backend/fonts/dependencies changed during paired run')
                        key = (version, case['id'])
                        if key in reference and response['output'] != reference[key]:
                            raise ValueError('unstable glyph/cluster/advance output')
                        reference[key] = response['output']
                        records.append(dict(round=trial, version=version, case=case['id'], response=response))
                    except (subprocess.TimeoutExpired, ValueError, KeyError, OSError, TypeError, AttributeError) as error:
                        unavailable.add((version, case['id']))
                        issues.append(dict(round=trial, version=version, case=case['id'], reason=str(error)))
                    (output / 'raw.json').write_text(json.dumps(records, indent=2))
                    (output / 'issues.json').write_text(json.dumps(issues, indent=2))
        finally:
            for adapter in adapters.values():
                adapter.close()
                adapter.stderr.close()
        print(f'paired round {trial + 1}/{rounds}', flush=True)
    summary = []
    for case in cases:
        name = case['id']
        row = dict(case=name, status='invalid')
        selected = [r for r in records if r['case'] == name]
        if len(selected) == rounds * 2:
            before = reference['baseline', name]
            after = reference['candidate', name]
            row['output_equal'] = before == after
            row['missing'] = after['missing']
            for phase in ('shape', 'cleanup', 'total'):
                values = {v: [statistics.median(b[phase] for b in next(
                    r['response']['batches'] for r in selected if r['round'] == t and r['version'] == v))
                    for t in range(rounds)] for v in ('baseline', 'candidate')}
                row[phase] = classify(values['baseline'], values['candidate'], config.get('policy', {}))
            row['status'] = ('output_changed' if before != after else 'missing_glyphs' if after['missing']
                             else 'regression' if row['total']['regression'] else 'pass')
        summary.append(row)
    try:
        validate_config(config)
    except ValueError as error:
        issues.append(dict(reason='inputs changed during run: ' + str(error)))
    report = dict(schema=1, platform=config['platform'], identity=expected_identity,
                  passed=not issues and all(x['status'] == 'pass' for x in summary), cases=summary, issues=issues)
    (output / 'report.json').write_text(json.dumps(report, indent=2))
    lines = [f"Performance gate: {'PASS' if report['passed'] else 'FAIL'} ({config['platform']}, {config['backend']})",
             '', 'Baseline: ' + config['baseline']['revision'],
             'Candidate: ' + config['candidate']['revision'], '',
             'ShapeCache disabled; total = directly measured ShapeText plus result release.',
             'Shape and cleanup phase results are retained separately in report.json.', '',
             '| Case | Baseline us | Candidate us | Paired change | Slower pairs | Status |',
             '|---|---:|---:|---:|---:|---|']
    for row in summary:
        total = row.get('total', {})
        number = lambda key: f"{total[key]:.3f}" if key in total else '-'
        lines.append('| {} | {} | {} | {} | {} | {} |'.format(
            row['case'], number('baseline_us'), number('candidate_us'),
            number('paired_percent') + '%', total.get('slower_pairs', '-'), row['status']))
    if issues:
        lines += ['', 'Execution/identity/input errors (see issues.json):']
        lines += ['- ' + json.dumps(issue, ensure_ascii=False) for issue in issues]
    (output / 'summary.md').write_text('\n'.join(lines) + '\n')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--cases', type=Path, default=Path(__file__).with_name('cases.json'))
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    config = json.loads(args.config.read_text())
    manifest = json.loads(args.cases.read_text())
    cases = manifest['cases']
    ids = [x['id'] for x in cases]
    if manifest.get('schema') != 1 or not ids or len(set(ids)) != len(ids) or not all(re.fullmatch(r'[A-Za-z0-9_-]+', x) for x in ids):
        parser.error('invalid workload manifest')
    config['cases_sha256'] = digest(args.cases)
    config['benchmark_sha256'] = benchmark_digest(Path(__file__).parent)
    report = run(config, cases, args.output)
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
