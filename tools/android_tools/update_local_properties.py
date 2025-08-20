# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import os.path
import re


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', help="path to local.properties")
    parser.add_argument(
        '-p', '--properties', nargs='+',
        help='property list to be updated, which has a format of "name1=value1 name2=value2"'
    )

    args = parser.parse_args()
    property_list = args.properties if isinstance(args.properties, list) else [args.properties]
    properties = {item[0]: item[1] for item in [exp.split("=") for exp in property_list]}

    content = ''
    if os.path.exists(args.file):
        with open(args.file, 'r') as f:
            for line in f.readlines():
                matches = re.match(r'(\S+)=(.*)', line)
                if matches and matches.group(1) in properties:
                    key = matches.group(1)
                    if len(properties[key]) == 0:
                        content +=line;
                    else:
                        content += f'{key}={properties[key]}\n'
                    properties.pop(key)
                else:
                    content += line

    for k, v in properties.items():
        if v:
            content += f'{k}={v}\n'

    with open(args.file, 'w') as f:
        f.write(content)


if __name__ == '__main__':
    main()
