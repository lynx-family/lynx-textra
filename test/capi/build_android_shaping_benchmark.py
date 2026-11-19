#!/usr/bin/env python3
"""Relink a test-only core copy with the shaping benchmark JNI entry.

Reuses the original core's exact compiler/linker flags and objects. The GN
checkout, original objects and production library are not modified.
"""
import argparse
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[2]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    build, output = args.core_build.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    obj = output / 'shaping-benchmark.o'
    original = 'obj/src/ports/shaper/java/javashaper.java_shaper.o'
    command = subprocess.check_output([ROOT/'buildtools/ninja/ninja','-C',build,'-t','commands',original],text=True).splitlines()[-1]
    compile_args = shlex.split(command)
    compile_args[compile_args.index('-c')+1] = str(ROOT/'test/capi/android_shaping_benchmark.cc')
    compile_args[compile_args.index('-o')+1] = str(obj)
    compile_args[compile_args.index('-MF')+1] = str(obj)+'.d'
    subprocess.run(compile_args, cwd=build, check=True)
    target = (build/'obj/platform/android/lynxtextra/lynxtextra.ninja').read_text().splitlines()
    line = next(line for line in target if ': solink ' in line)
    objects = shlex.split(line.split(': solink ',1)[1].split(' || ',1)[0])
    flags = shlex.split(next(line.strip().split(' = ',1)[1] for line in target if line.strip().startswith('ldflags = ')))
    libs = shlex.split(next(line.strip().split(' = ',1)[1] for line in target if line.strip().startswith('libs = ')))
    library = output/'liblynxtextra.so'
    subprocess.run([compile_args[0],'-shared',*flags,'-Wl,-soname=liblynxtextra.so','-o',library,*objects,obj,*libs],cwd=build,check=True)
    print(library)

if __name__ == '__main__':
    main()
