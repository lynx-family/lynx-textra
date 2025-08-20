# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import os

run_dir = os.path.dirname(os.path.abspath(__file__))
gn_dir = str(os.path.join(run_dir, "../../buildtools/gn/"))
output_dir = str(os.path.join(run_dir, "out"))
data_dir = str(os.path.join(run_dir, "data"))
exe_name = "linebreak_test"


def build_exe(output: str, var: str):
    if not os.path.exists(output):
        os.makedirs(output)
    os.system("cd %s && %s/gn gen -C . %s && ninja -C . linebreak_test" % (output, gn_dir, var))


def input_files() -> list:
    return [str(os.path.join(data_dir, f)) for f in os.listdir(data_dir) if os.path.isfile(os.path.join(data_dir, f))]


def output_filename(input_filename: str, output: str) -> str:
    basename = os.path.basename(input_filename)
    return str(os.path.join(output, basename))


def exec_linebreak(exe_file: str, files: list):
    exe_dir = str(os.path.dirname(exe_file))
    exe_file = str(os.path.basename(exe_file))
    for file in files:
        output = output_filename(file, exe_dir)
        os.system("cd %s && ./%s %s %s" % (exe_dir, exe_file, file, output))


def run_linebreak(name: str, var: str):
    output = str(os.path.join(output_dir, name))
    build_exe(output, var)
    exec_linebreak(str(os.path.join(output, exe_name)), input_files())


def read_result(filename: str) -> list:
    with open(filename, "r+") as file:
        lines = file.readlines()
        lines = [[var.strip(" \r\n") for var in str(line).split(" ")] for line in lines]
        return lines


def merge_result(base: list, compare: list) -> list:
    assert len(base) == len(compare)
    result = []
    for i in range(len(base)):
        assert base[i][0] == compare[i][0]
        result.append([base[i][0], base[i][1], compare[i][1]])
    return result


def init_diff_map() -> dict:
    return {
        "none": [0, 0],
        "graphme": [0, 0],
        "word": [0, 0],
        "linebreakable": [0, 0],
        "mustlinebreak": [0, 0],
        "paragraph": [0, 0],
        "total": [0, 0],
    }


def boundary_type_to_index(type: str) -> int:
    if type == "none":
        return 0
    elif type == "graphme":
        return 1
    elif type == "word":
        return 2
    elif type == "word":
        return 3
    elif type == "linebreakable":
        return 4
    elif type == "mustlinebreak":
        return 5
    elif type == "paragraph":
        return 6


def count_diff(result: list, diff: dict):
    for line in result:
        assert diff.get(line[1]) is not None
        diff[line[1]][0] += 1
        if line[1] != line[2]:
            diff[line[1]][1] += 1
            diff["total"][1] += 1
        diff["total"][0] += 1


def log_diff(diff: dict, name: str):
    print(name + ":")
    print("type: wrong / total")
    for key, value in diff.items():
        print("%s: %d / %d | %.2f%%" % (
            str(key), int(value[1]), int(value[0]),
            float(value[1]) / float(value[0]) * 100 if float(value[0]) != 0 else 0))


def write_merged_result(result: list, path: str):
    with open(path, "w+") as file:
        file.writelines(
            ["%s %s %s %s\n" % (line[0], line[1], line[2], ("" if line[1] == line[2] else "wrong")) for line in
             result])


def compare_result(base: str, compare: str):
    output_path_base = str(os.path.join(output_dir, base))
    output_path_compare = str(os.path.join(output_dir, compare))
    inputs = input_files()
    for input_file in inputs:
        output_file_base = output_filename(input_file, output_path_base)
        output_file_compare = output_filename(input_file, output_path_compare)
        base_results = read_result(output_file_base)
        compare_results = read_result(output_file_compare)
        results = merge_result(base_results, compare_results)
        write_merged_result(results, output_filename(input_file, output_dir))
        diff = init_diff_map()
        count_diff(results, diff)
        log_diff(diff, str(os.path.basename(input_file)))


def run():
    linebreak = "linebreak"
    simple = "simple"
    run_linebreak(linebreak, "--args='boundary_analyst=\"icu\"'")
    run_linebreak(simple, "--args='boundary_analyst=\"simple\"'")
    compare_result(linebreak, simple)


if __name__ == "__main__":
    run()
