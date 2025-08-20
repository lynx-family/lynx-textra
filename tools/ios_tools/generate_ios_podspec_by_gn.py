#!/usr/bin/env python3
# Copyright 2023 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.


"""
This script help us quickly generate podspec file by gn script.

usage: generate_ios_podspec_by_gn.py [-h] --target {All,LynxTextra,json_parser}
"""

import argparse
import os
import shutil
import subprocess
import sys

CUR_FILE_PATH = os.path.dirname(os.path.abspath(__file__))
SEARCH_PATH = os.path.join(CUR_FILE_PATH, '..', 'gn_tools')
sys.path.append(SEARCH_PATH)
from gn_to_podspec import gn_to_podspec

class PodspecItem:
  def __init__(self, podspec_path, start_target, sub_target, keep_subspecs):
    self.podspec_path = podspec_path
    self.start_target = start_target
    self.sub_target = sub_target
    self.keep_subspecs = keep_subspecs

def generate_project_json(root_path, is_debug):
  print('Generate project json')
  gn_args = 'target_os=\\\"%s\\\" ' % ('ios')
  is_debug_value = 'false'
  if is_debug:
    is_debug_value = 'true'
  gn_args += f'use_xcode=true \
              is_debug={is_debug_value}'
  
  args = ' --args="' + gn_args + '"'
  gn_out_path = os.path.join(root_path, 'out', 'ios_config')
  gn_path = os.path.join(root_path, 'buildtools', 'gn', 'gn')
  gn_command = gn_path + ' gen ' + gn_out_path + args + ' --ide=json'
  r = subprocess.call(gn_command, shell=True)
  return r

def generate(targets, output_path, is_debug):
  file_path = os.path.dirname(os.path.abspath(__file__))
  root_path = os.path.join(file_path, '..', '..')

  r = generate_project_json(root_path, is_debug)
  if r != 0:
    return r
  
  podspec_dict = {}
  if 'LynxTextra' in targets:  
    lynxtextra_podspec = PodspecItem(os.path.join(root_path, 'LynxTextra.podspec'), 
      "//platform/iOS/LynxTextra:LynxTextra_podspecs_group",
      "//platform/iOS/LynxTextra:LynxTextra_subspecs_group", [])
    podspec_dict['LynxTextra'] = lynxtextra_podspec
  if 'json_parser' in targets:
    json_parser_podspec = PodspecItem(os.path.join(root_path, 'json_parser.podspec'), 
      "//examples/json_parser:json_parser_podspecs_group",
      "//examples/json_parser:json_parser_subspecs_group", [])
    podspec_dict['json_parser'] = json_parser_podspec

  json_path = os.path.join(root_path, 'out', 'ios_config', 'project.json')
  compiler_target = '//platform/iOS/LynxTextra:get_ios_base_configs'

  if "All" in targets:
    targets = list(podspec_dict.keys())
  
  r = 0
  for target in targets:
    print('Generate podspec of %s' % (target))
    podspec = podspec_dict[target]
    if podspec == None:
      print('Unknown target: %s' % (target))
      return -1
    r |= gn_to_podspec(json_path, podspec.start_target, podspec.sub_target, compiler_target, podspec.podspec_path, podspec.keep_subspecs)
    if r == 0:
      print('Copy podspec of %s' % (target))
      name = target + '.podspec'
      src_path = os.path.join(root_path, 'out', 'ios_config', 'podspecs', name)
      if output_path:
        dst_path = os.path.join(root_path, output_path, name)
      else:
        dst_path = os.path.join(root_path, name)
      shutil.copy(src_path, dst_path)
  return r

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target', type=str, choices=["All", "LynxTextra", "json_parser"], 
                      nargs='+', required=True, help='The name(s) of the podspec(s) you want to generate automatically. You can specify multiple targets like: --target LynxTextra json_parser')
  parser.add_argument('--is_debug', default=False, action='store_true', help='Whether to set the is_debug flag to true, which will be used by the gn script.')
  parser.add_argument('--output-path', default='', help='podspec file located path')
  args = parser.parse_args()
  targets = args.target
  output_path = args.output_path
  is_debug = args.is_debug
  
  return generate(targets, output_path, is_debug)

if __name__ == "__main__":
  sys.exit(main())