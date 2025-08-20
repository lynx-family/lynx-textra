#!/usr/bin/env python3
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import sys
import os

SRC_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def get_out_dir(args):
    if args.target_os != None:
        target_dir = [args.target_os]
    else:
        target_dir = ['host']

    is_debug = args.is_debug

    target_dir.append("debug" if is_debug else "release")

    if args.harmony_cpu != 'arm':
      target_dir.append(args.harmony_cpu)

    return os.path.join(args.out_dir, 'out', '_'.join(target_dir))

def to_command_line(gn_args):
    def merge(key, value):
        if type(value) is bool:
            return '%s=%s' % (key, 'true' if value else 'false')
        if type(value) is int:
            return '%s=%s' % (key, value)
        return '%s="%s"' % (key, value)
    return [merge(x, y) for x, y in list(gn_args.items())]

def parse_value(v):
   if v == 'true':
      return True
   if v == 'false':
      return False
   return v

def to_gn_args(args):
    is_debug = args.is_debug

    gn_args = {}
    if args.gn_args is not None:
       gn_args = {k: parse_value(v) for k, v in (pair.split('=') for pair in args.gn_args.split())}

    # Basic GN args
    gn_args['is_official_build'] = True
    gn_args['is_debug'] = is_debug
    gn_args['target_os'] = args.target_os
    gn_args['target_cpu'] = args.harmony_cpu

    # Harmony specific GN args
    if args.target_os == "harmony":
      gn_args['enable_bitcode'] = False
      gn_args['build_lepus_compile'] = False
      gn_args['enable_lepusng_worklet'] = True
      gn_args['jsengine_type'] = 'quickjs'
      gn_args['use_primjs_napi'] = True
      gn_args['harmony_sdk_version'] = 'default'
      gn_args['v8_headers_search_path'] = '//lynx/third_party/v8/11.1.277/include'
      gn_args['disable_primjs_symbol_visibility_hidden'] = True
      gn_args['enable_primjs_prebuilt_lib'] = True

    return gn_args

def parse_args(args):
  args = args[1:]
  parser = argparse.ArgumentParser(description='A script run` gn gen`.')

  parser.add_argument('--gn-args', type=str, dest='gn_args')
  parser.add_argument('--is-debug', dest='is_debug', action='store_true', default=False)
  parser.add_argument('--harmony', dest='target_os', action='store_const', const='harmony')
  parser.add_argument('--harmony-cpu', type=str, choices=['arm', 'arm64'], default='arm')
  parser.add_argument('--ide', default='', type=str,
                      help='The IDE files to generate using GN. Use `gn gen help` and look for the --ide flag to' +
                      ' see supported IDEs. If this flag is not specified, a platform specific default is selected.')
  parser.add_argument('--out-dir', default='', type=str)

  return parser.parse_args(args)

def main(argv):
  args = parse_args(argv)

  exe = '.exe' if sys.platform.startswith(('cygwin', 'win')) else ''

  command = [
    '%s/buildtools/gn/gn%s' % (SRC_ROOT, exe),
    'gen',
  ]

  if args.ide != '':
    command.append('--ide=%s' % args.ide)

  gn_args = to_command_line(to_gn_args(args))
  out_dir = get_out_dir(args)
  command.append(out_dir)
  command.append('--args=%s' % ' '.join(gn_args))
  print("Generating GN files in: %s" % out_dir)
  try:
    gn_call_result = subprocess.call(command, cwd=SRC_ROOT)
  except subprocess.CalledProcessError as exc:
    print("Failed to generate gn files: ", exc.returncode, exc.output)
    sys.exit(1)

  if gn_call_result == 0:
    # Generate/Replace the compile commands database in out.
    compile_cmd_gen_cmd = [
      '%s/buildtools/ninja/ninja%s' % (SRC_ROOT, exe),
      '-C',
      out_dir,
      '-t',
      'compdb',
      'cc',
      'cxx',
      'objc',
      'objcxx',
      'asm',
    ]

    try:
      contents = subprocess.check_output(compile_cmd_gen_cmd, cwd=SRC_ROOT)
    except subprocess.CalledProcessError as exc:
      print("Failed to run ninja: ", exc.returncode, exc.output)
      sys.exit(1)
    compile_commands = open('%s/out/compile_commands.json' % SRC_ROOT, 'wb+')
    compile_commands.write(contents)
    compile_commands.close()

  return gn_call_result

if __name__ == '__main__':
    sys.exit(main(sys.argv))
