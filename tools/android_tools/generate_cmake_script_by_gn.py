#!/usr/bin/env python3
# Copyright 2023 The Lynx Authors. All rights reserved.

"""
usage: generate_cmake_script_by_gn.py [-h] --gen-file GEN_FILE --abi ABI [--gn-args GN_ARGS] [--target TARGET] [--flavor FLAVOR] [--build-type BUILD_TYPE]
                                      [--keep-libs KEEP_LIBS [KEEP_LIBS ...]] [--cmake-version CMAKE_VERSION] [--project-name PROJECT_NAME]

This script generate CMakeLists-impl.cmake from a gn shared_library target.

"""

import sys
import argparse
import os
import shutil
import subprocess
import logging

CUR_FILE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(CUR_FILE_DIR, "..", "gn_tools"))
from gn_to_cmake import gn_to_cmake

# project root build path
ROOT_PATH = os.path.join(CUR_FILE_DIR, '..', '..')

GN_DEFAULT_PATH = os.path.join(ROOT_PATH, 'out/gn_products_for_gradle')

def generateProjectJson(abi, args_list):
  target_cpu = ''
  if abi == 'x86':
    target_cpu = 'x86'
  elif abi == 'armeabi-v7a' or abi == 'armeabi':
    target_cpu = 'arm'
  elif abi == 'mips':
    target_cpu = 'mipsel'
  elif abi == 'x86_64':
    target_cpu = 'x64'
  elif abi == 'arm64-v8a':
    target_cpu = 'arm64'
  elif abi == 'mips64':
    target_cpu = 'mips64el'
  else:
    return 1

  gn_args = 'target_os=\\\"%s\\\" target_cpu=\\\"%s\\\" ' % ('android', target_cpu)
  gn_args +=  'use_ndk_cxx=true'

  flavor_name = ''
  build_type = ''
  for arg in args_list:
    arg_kv = arg.split('=')
    if len(arg_kv) != 2:
      continue
    key = arg_kv[0]
    value = arg_kv[1]
    if key == 'flavor_name':
      flavor_name = value
      continue
    if value == 'true' or value == 'false':
      gn_args += ' %s' % arg
    else:
      gn_args += ' %s=\\\"%s\\\"' % (key, value)
    if key == 'is_debug':
      if value == 'true':
        build_type = 'debug'
      else:
        build_type = 'release'

  build_variant = ''
  if len(flavor_name) > 0 and len(build_type) > 0:
    build_variant = flavor_name + build_type

  out_path = 'android_' + abi + '_' + build_variant
  gn_out_path = os.path.join(GN_DEFAULT_PATH, out_path)
  args = ' --args="' + gn_args + '"'
  gn_path = os.path.join(ROOT_PATH, 'buildtools', 'gn', 'gn')
  gn_command = gn_path + ' gen ' + gn_out_path + args + ' --ide=json'
  return subprocess.call(gn_command, shell=True)

def copy_to_project(cmake_path, abi, build_variant):
  path_to_path = {}
  for root, ds, fs in os.walk(cmake_path):
    for f in fs:
      fullname = os.path.join(root, f)
      ab_path = fullname.replace(cmake_path+'/', '')
      ab_path = os.path.dirname(ab_path)
      des_path = os.path.join(ROOT_PATH, ab_path, 'CMakeLists_impl', 'impl', build_variant, abi)
      if not os.path.exists(des_path):
        os.makedirs(des_path, exist_ok=True)
      des_path = os.path.join(des_path, 'CMakeLists-impl.cmake')
      path_to_path[fullname] = des_path
  for key in path_to_path.keys():
    value = path_to_path[key]
    shutil.copy(key, value)

def generateCmakeLists(target, cmake_version, cmake_project_name, abi, flavor_name, build_type, keep_libs):
  target_name = target.split(':')[1]

  build_variant = ''
  gn_out_path = ''
  if len(flavor_name) > 0 and len(build_type) > 0:
    build_variant = flavor_name + build_type
    out_path = 'android_' + abi + '_' + build_variant
    gn_out_path = os.path.join(GN_DEFAULT_PATH, out_path)
  else:
    # When flavor name and build type are not set, means that caller doesn't distinguish build_variant type,
    # so directly find any project.json path in the GN_DEFAULT_PATH to generate CMakeLists.txt.
    if os.path.exists(GN_DEFAULT_PATH):
      for d in os.listdir(GN_DEFAULT_PATH):
        dir = os.path.join(GN_DEFAULT_PATH, d)
        if os.path.isdir(dir):
          gn_out_path = dir
          break

  if gn_out_path == '':
    return 0

  compiler_target = '//:gen_basic_compiler_flags'

  json_path = os.path.join(gn_out_path, 'project.json')
  r = gn_to_cmake(json_path, target, cmake_version, cmake_project_name, compiler_target, keep_libs, flavor_name)

  if r != 0:
    return r
  
  cmake_path = os.path.join(gn_out_path, 'CMakeLists')
  copy_to_project(cmake_path, abi, build_variant)

  return 0

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--gen-file', type=str, required=True, default= 'project.json', help='The name of the file to generate, supporting "project.json" and "CMakeLists.txt"')
  parser.add_argument('--abi', type=str, required=True, help='Android abi')
  parser.add_argument('--gn-args', type=str, required=False, help='GN build parameters for generating project.json')
  parser.add_argument('--target', type=str, required=False, help='The target from which you want to generate CMakeLists')
  parser.add_argument('--flavor', type=str, required=False, help="target's flavor name for compilation")
  parser.add_argument('--build-type', type=str, required=False, help="target's build type for compilation")
  parser.add_argument('--keep-libs', nargs='+', required=False, help='Basic libs config in buildroot will be removed. The libs in keep-libs list will be filtered out')
  parser.add_argument('--cmake-version', type=str, required=False, help='the min version of cmake')
  parser.add_argument('--project-name', type=str, required=False, help='Project name of CMakeLists.txt')
  args = parser.parse_args()
  abi = args.abi
  gn_args = args.gn_args
  target = args.target
  flavor_name = args.flavor if args.flavor and len(args.flavor) > 0  else ''
  build_type = args.build_type if args.build_type and len(args.build_type) > 0  else ''
  keep_libs = args.keep_libs
  args_list = []
  keep_libs_list = []
  if (gn_args and len(gn_args) > 0):
    args_list = list(set(gn_args.split(',')))
  if (keep_libs and len(keep_libs) > 0):
    keep_libs_list = keep_libs
  cmake_version = args.cmake_version if args.cmake_version else '3.4.1'
  cmake_project_name = args.project_name if args.project_name else ''

  if args.gen_file == 'project.json':
    return generateProjectJson(abi, args_list)
  elif args.gen_file == 'CMakeLists.txt':
    return generateCmakeLists(target, cmake_version, cmake_project_name, abi, flavor_name, build_type, keep_libs_list)
  else:
    logging.warning('Please enter the name of the file you want to produce with "--gen-file"!!')
    return -1

if __name__ == "__main__":
  sys.exit(main())