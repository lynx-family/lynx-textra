#!/usr/bin/env python3
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import os
import sys
from subprocess import check_call


CUR_DIR = os.path.dirname(os.path.abspath(__file__))
HARMONY_DIR = CUR_DIR

def patch_oh_package(module_path, version):
    cmd = 'ohpm version {}'.format(version)
    print(f'run command {cmd} in {module_path}')
    check_call(cmd, shell=True, cwd=module_path)
    
    package_file = os.path.join(module_path, "oh-package.json5")
    # print replaced file
    with open(package_file, "r") as f:
        print(f.read())

def collect_module_config_list(args):
    import json5
    with open(os.path.join(HARMONY_DIR,'build-profile.json5'), 'r') as f:
        build_profile = json5.load(f)

    module_config_list = build_profile['modules']
    if args.verbose:
        print('module_config_list is'+ str(module_config_list))
    return module_config_list

def run_package_har(module_name, module_path, verbose):
    if verbose:
        print(f'===== start run package {module_name} =====')

    local_properties_path = os.path.join(HARMONY_DIR, 'local.properties')
    if not os.path.exists(local_properties_path):
        print('harmony/local.properties not found')
        if 'HARMONY_HOME' in os.environ:
            # write hwsdk.dir to local.properties
            harmony_sdk_path = os.environ['HARMONY_HOME']
            print('harmony/local.properties not found, write hwsdk.dir with {} to it'.format(harmony_sdk_path))
            with open(local_properties_path, 'w') as f:
                f.write('hwsdk.dir={}'.format(harmony_sdk_path))
        else:
            print('harmony/local.properties not found, and HARMONY_HOME is not set.')

    cmd = f'hvigorw assembleHar --mode module -p module={module_name}@default -p product=default -p buildMode=debug --no-daemon'
    if verbose:
        print(f'run command {cmd}')
    check_call(cmd, shell=True, cwd=HARMONY_DIR)
    # as even hvigor build failed, it still return value 0, so we need to check har file exist or not
    har_path = os.path.join(HARMONY_DIR, module_path, 'build', 'default', 'outputs', 'default', f'{module_name}.har')
    print(f'har_path is {har_path}')
    if not os.path.isfile(har_path):
        raise Exception('har file not found, please check your build')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true", default=False, help="verbose print")
    parser.add_argument("--version", type=str, default=None, help="override version")
    parser.add_argument("--publish_har", action="store_true", default=False, help="publish har")
    parser.add_argument("--modules", nargs="*", help="list of modules name")

    args = parser.parse_args()

    modules = args.modules if args.modules else []
    if len(modules) == 0:
        print('no module specified to build')
        return 0

    for module in modules:
        module_config_list = collect_module_config_list(args)
        for module_config in module_config_list:
            if module_config['name'] == module:
                module_path = module_config['srcPath']
                break
        else:
            raise Exception(f"module {module} not found in build-profile.json5")
        module_full_path = os.path.join(HARMONY_DIR, module_path)
        if args.publish_har:
            if args.version:
                publish_version = args.version
                patch_oh_package(module_full_path, publish_version)
            else:
                print(f"version is required when publish har")
                return 1
        run_package_har(module, module_full_path, args.verbose)


if __name__ == "__main__":
    sys.exit(main())