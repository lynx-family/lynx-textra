#!/usr/bin/env python3
# Copyright 2023 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.


"""
This script help us quickly auto generate an gn script template containing sources

usage: print_gn_sources.py [-h] --path PATH [--prefix PREFIX] [--header-only] [--target-name TARGET_NAME] [--mode {override,add}]

The source list of gn do not support wildcards. All sources files need to be listed when 
writing the source target. This script will help us quickly print out sources for a given 
path on the command line. The output will be in the format required by gn sources.
"""

import argparse
import os
import sys

def is_target_exist(path, target_name):
  if not os.path.exists(path):
    return False
  f = open(path)
  line = f.readline()
  find = False
  name = '("%s")' % (target_name)
  while line:
    if name in line:
      find = True
      break
    line = f.readline()
  f.close()
  return find

def write_sources_to_target(path, target_name, sources, mode):
  if target_name == None:
    target_name = os.path.split(path)[-1]
  
  file_path = os.path.dirname(os.path.abspath(__file__))
  root_path = os.path.join(file_path, '..', '..')
  r_path = os.path.relpath(path, root_path)
  build_file_path = os.path.join(path, 'BUILD.gn')
  
  f = None
  if mode == 'override' or mode == None:
    f = open(build_file_path, 'w+')
    # header
    f.write('# Copyright 2023 The Lynx Authors. All rights reserved.\n')
    f.write('#\n\n')
  elif mode == 'add':
    if is_target_exist(build_file_path, target_name):
      print('\nTarget %s already exists in %s. You can copy the above sources to the corresponding target as required\n' % (target_name, build_file_path))
      return -1
    need_header = not os.path.exists(build_file_path)
    f = open(build_file_path, 'a+')
    if need_header:
      # header
      f.write('# Copyright 2023 The Lynx Authors. All rights reserved.\n')
      f.write('#\n\n')
    else:
      f.write('\n\n')
  # sources
  share_sources_name = target_name + '_shared_sources'
  config_name = '%s_private_config' % (target_name)
  f.write(share_sources_name)
  f.write(' = [\n')
  for s in sources:
    f.write('  ' + s + '\n')
  f.write(']\n\n')
  # target
  source_set_name = 'source_set'
  f.write('%s("%s") {\n' % (source_set_name, target_name))
  f.write('  sources = %s\n' % (share_sources_name))
  f.write('  public = []\n\n')
  f.write('  configs += [ ":%s" ]\n\n' % (config_name))
  f.write('  deps = []\n')
  f.write('}\n\n')
  # config
  f.write('config("%s") {\n' % (config_name))
  f.write('  include_dirs = []\n\n')
  f.write('  cflags = []\n\n')
  f.write('  defines = []\n')
  f.write('}\n\n')

  f.close()

  print('\nTarget full name is: //%s:%s\n' % (r_path, target_name))

  return 0

def print_source_file(path, prefix, header_only, target_name, mode):
  file_list = []
  path_len = len(path)
  for root, ds, fs in os.walk(path):
    for f in fs:
      if f == 'LICENSE' or f == 'BUILD.gn' or f == '.DS_Store':
        continue
      fullname = os.path.join(root, f)
      fullname = fullname[path_len+1:]
      if header_only:
        if fullname.endswith('.h') or fullname.endswith('.hpp'):
          fullname = "\""+prefix+fullname+"\","
          file_list.append(fullname)
      else:
        fullname = "\""+prefix+fullname+"\","
        file_list.append(fullname)
  
  file_list.sort()
  for source in file_list:
    print(source)
  if mode == None:
    return
  if (not os.path.exists(os.path.join(path, 'BUILD.gn'))):
    return write_sources_to_target(path, target_name, file_list, mode)
  else:
    print('\n!!!The BUILD.gn file already exists. You can copy the above sources to the corresponding target as required!!!\n')
  return 0    

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--path', type=str, required=True)
  parser.add_argument('--prefix', type=str, default='', required=False)
  parser.add_argument('--header-only', default=False, action='store_true')
  parser.add_argument('--target-name', type=str, required=False)
  parser.add_argument('--mode', type=str, choices=['override', 'add'], required=False)
  args = parser.parse_args()
  path = args.path
  prefix = args.prefix
  header_only = args.header_only
  target_name = args.target_name
  mode = args.mode
  return print_source_file(path, prefix, header_only, target_name, mode)

if __name__ == '__main__':
  sys.exit(main())