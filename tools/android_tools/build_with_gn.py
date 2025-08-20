#!/usr/bin/env python3
# Copyright 2023 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.


"""
Gradle supports cmake by default. It also provides syntax for cmake access.
It is very easy to use cmake in gradle with this syntax.
Gradle doesn't provide syntax to support for using GN directly in gradle script.
This script connect the gradle and GN and so that we can trigger native build with gn in gradle.
"""

import argparse
import os
import shutil
import subprocess
import sys


def get_target_cpu_from_abi(abi):
  target_cpu = ''
  if abi == 'x86':
    target_cpu = 'x86'
  elif abi == 'armeabi-v7a' or abi == 'armeabi':
    target_cpu = 'arm'
  elif abi == 'arm64-v8a':
    target_cpu = 'arm64'
  elif abi == 'mips':
    target_cpu = 'mipsel'
  elif abi == 'x86_64':
    target_cpu = 'x64'
  elif abi == 'mips64':
    target_cpu = 'mips64el'
  return target_cpu

def clean_build(root_path, abi, build_type, flavor_name):
  """
  Execute 'gn clean OUTPUT_PATH' command
  """
  command_path = os.path.join(root_path, 'buildtools', 'gn', 'gn')
  gn_out_path = os.path.join(root_path, 'out', 'android', flavor_name+build_type, abi)
  if not os.path.exists(gn_out_path):
    return 0
  gn_command = command_path + ' clean ' + gn_out_path
  print('Execute:%s\n' % (gn_command))
  r = subprocess.call(gn_command, shell=True)
  if r != 0:
    print('Execute gn clean command failed')
    return -1
  return 0

def generate_ninja_file(root_path, output_path, abi, target_cpu, build_type, flavor_name, args):
  """
  Execute 'gn gen OUTPUT_PATH --args=xxx' command
  """
  gn_args = 'target_os=\\\"%s\\\" target_cpu=\\\"%s\\\" flavor_name=\\\"%s\\\"' % ('android', target_cpu, flavor_name)
  gn_args += ' use_ndk_cxx=true is_gradle_build=true skity_example=false skity_test=false'
  for arg in args:
    gn_args += ' ' + arg

  gn_out_path = os.path.join(root_path, 'out', output_path, flavor_name+build_type, abi)
  args_str = ' --args="' + gn_args + '"'
  command_path = os.path.join(root_path, 'buildtools', 'gn', 'gn')
  gn_command = command_path + ' gen ' + gn_out_path + args_str
  print('Execute:%s\n' % (gn_command))
  r = subprocess.call(gn_command, shell=True)
  if r != 0:
    print('Execute gn gen command failed')
    return None
  return gn_out_path

def build_target(root_path, target, output_path):
  """
  Execute 'ninja -C OUTPUT_PATH' command
  """
  command_path = os.path.join(root_path, 'buildtools', 'ninja', 'ninja')
  command = command_path + ' -C ' + output_path + ' ' + target
  print('Execute:%s\n' % (command))
  r = subprocess.call(command, shell=True)
  if r != 0:
    print('Execute ninja command failed\n')
    return -1
  return 0

def copy_product(output_name, output_path, gn_output_path):
  if not os.path.exists(gn_output_path):
    print('Path not exist: %s' % (gn_output_path))
    return -1
  if not os.path.exists(output_path):
    os.makedirs(output_path)
  src = os.path.join(gn_output_path, output_name)
  print('Copy %s to %s' % (output_name, output_path))
  shutil.copy(src, output_path)
  return 0

def build_with_gn(target, abi, build_args, output_name, output_path):
  file_path = os.path.dirname(os.path.abspath(__file__))
  root_path = os.path.join(file_path, '..', '..')

  # setup env
  r = subprocess.call('source tools/envsetup.sh', shell=True)
  if r != 0:
    print('Setup environment failed!\n')

  target_cpu = get_target_cpu_from_abi(abi)
  if target_cpu == '':
    print('Can not find corresponding cpu type for %s\n' % (abi))
    return -1
  
  args_list = build_args.split(',')
  gn_args = []
  flavor_name = ''
  build_type = ''
  is_clean_task = False
  for arg_str in args_list:
    kv = arg_str.split('=')
    if len(kv) != 2:
      continue
    key = kv[0]
    value = kv[1]
    if key == 'clean_build':
      if value == 'true':
        is_clean_task = True
    elif key == 'flavor_name':
      flavor_name = value
      if flavor_name == 'asan':
        gn_args.append('is_asan=true')
    elif key == 'is_debug':
      if value == 'true':
        build_type = 'Debug'
      elif value == 'false':
        build_type = 'Release'
      gn_args.append(arg_str)
    else:
      gn_args.append(arg_str)

  if is_clean_task:
    return clean_build(root_path, abi, build_type, flavor_name)
  
  gn_output_path = generate_ninja_file(root_path, output_path, abi, target_cpu, build_type, flavor_name, gn_args)
  if gn_output_path == None:
    return -1
  
  r = build_target(root_path, target, gn_output_path)
  if r != 0:
    return r

  dst_path = os.path.join(root_path, output_path, 'build', 'intermediates', 'cmake', flavor_name+build_type, 'obj', abi)
  r = copy_product(output_name, dst_path, gn_output_path)

  return r

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target', type=str, required=True, help='The gn target name to be compiled')
  parser.add_argument('--abi', type=str, required=True, help='The abi of the target product')
  parser.add_argument('--build-args', type=str, required=True, help='Build arguments')
  parser.add_argument('--output-name', type=str, required=True, help='The product name')
  parser.add_argument('--output-path', type=str, required=False, help='The product output path')
  args = parser.parse_args()
  target = args.target
  abi = args.abi
  build_args = args.build_args
  output_name = args.output_name
  output_path = args.output_path
  if output_path == None:
    output_path = target.split(':')[0]

  return build_with_gn(target, abi, build_args, output_name, output_path)

if __name__ == "__main__":
  sys.exit(main())

