#!/usr/bin/env python3
"""Independently verify all demo reports/images and generate a comparison gallery.

Usage: python3 test/capi/verify_results.py /tmp/textra-demo-parity
Expected child directories: android, ios, skity. Requires Pillow.
"""
import argparse
import html
import json
from pathlib import Path
from PIL import Image

CASES = ('latin', 'cjk', 'wrapped', 'rtl', 'bold', 'italic', 'multiline', 'fallback')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    rows, summary = [], {}
    passed = True
    for backend in ('android', 'ios', 'skity'):
        folder = args.directory / backend
        results = json.loads((folder / 'results.json').read_text())
        if [row.get('case') for row in results] != list(CASES):
            raise RuntimeError(f'{backend}: incomplete/mismatched cases: {results}')
        backend_ok = True
        for sample, result in zip(CASES, results):
            images = []
            for mode in ('cpp', 'capi', 'repeat'):
                path = folder / f'{sample}-{mode}.png'
                if not path.exists():
                    pam = path.with_suffix('.pam').read_bytes()
                    header, data = pam.split(b'ENDHDR\n', 1)
                    expected = b'P7\nWIDTH 320\nHEIGHT 200\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\n'
                    if header != expected or len(data) != 320*200*4:
                        raise RuntimeError(f'Unexpected bitmap format: {path}')
                    Image.frombytes('RGBA', (320,200), data).save(path)
                with Image.open(path) as image:
                    if image.size != (320, 200):
                        raise RuntimeError(f'Unexpected bitmap dimensions: {path}')
                    images.append(image.convert('RGBA').tobytes())
            pixel_equal = images[0] == images[1] == images[2]
            checks = all(result.get(key) is True for key in
                         ('pixels_equal', 'metrics_equal', 'repeat_equal', 'nonempty'))
            ok = checks and pixel_equal and result.get('status', 0) == 0 and result.get('cpp_ok', True)
            passed &= ok
            backend_ok &= ok
            rows.append(f'<tr><td>{html.escape(backend+" / "+sample)}<br>{"PASS" if ok else "FAIL"}</td>'+
                        ''.join(f'<td><img src="{backend}/{sample}-{mode}.png"></td>' for mode in ('cpp','capi','repeat'))+'</tr>')
        summary[backend] = {'cases': len(results), 'passed': backend_ok}
        interface_file = folder / 'interfaces/results.json'
        if backend == 'android' and interface_file.exists():
            interface_results = json.loads(interface_file.read_text())
            interface_ok = len(interface_results) >= 81 and all(row.get('passed') is True for row in interface_results)
            operations = {int(row['name'].split('-')[1]) for row in interface_results if row.get('name', '').startswith('op-')}
            interface_ok &= operations == set(range(27))
            for row in interface_results:
                name = row.get('name', '')
                if not name.startswith('op-'):
                    continue
                with Image.open(folder / 'interfaces' / (name+'-actual.png')) as actual, Image.open(folder / 'interfaces' / (name+'-expected.png')) as expected:
                    interface_ok &= row.get('status') == row.get('expected_status', 0) and actual.size == expected.size == (64,64) and actual.convert('RGBA').tobytes() == expected.convert('RGBA').tobytes()
            summary[backend]['interface_checks'] = len(interface_results)
            summary[backend]['interface_passed'] = interface_ok
            summary[backend]['passed'] &= interface_ok
            passed &= interface_ok
        shared_file = folder / 'shared-java/results.json'
        if backend == 'android' and shared_file.exists():
            shared_results = json.loads(shared_file.read_text())
            shared_ok = len(shared_results) >= 27 and all(row.get('passed') is True for row in shared_results)
            for row in shared_results:
                name = row['name']
                if not name.startswith('legacy-') or not name.split('-')[1].isdigit():
                    continue
                with Image.open(folder / 'shared-java' / (name+'-actual.png')) as actual, Image.open(folder / 'shared-java' / (name+'-expected.png')) as expected:
                    shared_ok &= actual.size == expected.size == (64,64) and actual.convert('RGBA').tobytes() == expected.convert('RGBA').tobytes()
            summary[backend]['shared_java_checks'] = len(shared_results)
            summary[backend]['shared_java_passed'] = shared_ok
            summary[backend]['passed'] &= shared_ok
            passed &= shared_ok
    (args.directory / 'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    (args.directory / 'index.html').write_text('''<!doctype html><meta charset="utf-8">
<title>Textra C API demo rendering comparison</title>
<style>body{font:15px system-ui}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:8px}img{width:320px;height:200px;background:white}</style>
<h1>Textra demo rendering comparison</h1><p>Compare within each platform: C++, C API, repeated C API. These are not cross-platform font/pixel comparisons.</p>
<table><tr><th>Case</th><th>C++</th><th>C API</th><th>Repeated C API</th></tr>'''+''.join(rows)+'</table>')
    print(json.dumps(summary, indent=2))
    if not passed:
        raise SystemExit('Rendering comparison failed')

if __name__ == '__main__':
    main()
