  #!/usr/bin/env python3
# Copyright 2023 The Lynx Authors. All rights reserved.
import sys
import argparse
import os
import shutil
import subprocess
import logging
CUR_FILE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(CUR_FILE_DIR, "..", "gn_tools"))
from gn_tools import gn_to_cmake
# project root build path
ROOT_PATH = os.path.join(CUR_FILE_DIR, '..')
GN_DEFAULT_PATH = os.path.join(ROOT_PATH, 'out/gn_products_for_cmake')
def copy_to_project(cmake_path):
  path_to_path = {}
  for root, ds, fs in os.walk(cmake_path):
    for f in fs:
      fullname = os.path.join(root, f)
      ab_path = fullname.replace(cmake_path+'/', '')
      ab_path = os.path.dirname(ab_path)
      des_path = os.path.join(ROOT_PATH, ab_path, 'CMakeLists_impl', 'impl')
      if not os.path.exists(des_path):
        os.makedirs(des_path, exist_ok=True)
      des_path = os.path.join(des_path, 'CMakeLists-impl.cmake')
      path_to_path[fullname] = des_path
  for key in path_to_path.keys():
    value = path_to_path[key]
    shutil.copy(key, value)
def generate_project_json(gn_args):
  gn_out_path = os.path.join(GN_DEFAULT_PATH)
  args = ' --args="' + gn_args + ' use_flutter_cxx=false"' + ' --export-compile-commands'
  gn_path = os.path.join(ROOT_PATH, 'buildtools', 'gn', 'gn')
  gn_command = gn_path + ' gen ' + gn_out_path + args + ' --ide=json'
  return subprocess.call(gn_command, shell=True)
def generate_target_cmake(target_name):
  gn_out_path = os.path.join(GN_DEFAULT_PATH)
  gn_to_cmake_path = os.path.join(ROOT_PATH, 'tools','gn_tools','gn_to_cmake.py')
  gn_to_cmake_args = '--json-path %s --start-target %s --cmake-version 3.10 --keep-libs ""' %(gn_out_path+'/project.json', target_name)
  gn_to_cmake_command = 'python3 %s %s' %(gn_to_cmake_path, gn_to_cmake_args)
  print ('gn_to_cmake command:'+gn_to_cmake_command)
  ret_code= subprocess.call(gn_to_cmake_command, shell=True)
  if (ret_code != 0):
   return -1
  cmake_path = os.path.join(gn_out_path, 'CMakeLists')
  copy_to_project(cmake_path)
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--gn-args', type=str, required=False, help='GN compile arguments.')
  args = parser.parse_args()
  generate_project_json(args.gn_args)
  generate_target_cmake("//src:lynxtextra_shared")
  generate_target_cmake("//src:lynxtextra_static")
  generate_target_cmake("//demos/darwin/macos/glfw:gl_app_demo")
  generate_target_cmake("//demos/darwin/macos/mtl:mtl_app_demo")
  generate_target_cmake("//test:TextLayoutLiteTest")
  generate_target_cmake("//examples/json_parser:json_parser")
  generate_target_cmake("//examples/json_parser_exe:json_to_image_app")
  # generate_target_cmake("//demos/darwin/macos/skia_app_demo:skia_app_demo")
  # generate_target_cmake("//demos/darwin/macos/ttreaderdemo:ttreaderdemo")
  # generate_target_cmake("//demos/darwin/macos/ttreaderdemo:ttreaderdemo")
#   start_target = args.start_target
#   cmake_version = args.cmake_version
#   project_name = args.project_name
#   compiler_target = args.compiler_target
#   keep_libs = args.keep_libs
#   flavor_name = args.flavor_name
  return 0
if __name__ == "__main__":
  sys.exit(main())
