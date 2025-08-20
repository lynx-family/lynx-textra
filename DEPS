import os
import platform

system = platform.system().lower()
machine = platform.machine().lower()
machine = "x86_64" if machine == "amd64" else machine

root_dir = os.getcwd()

deps = {
    "tools_shared": {
        "type": "solution",
        "url": "https://github.com/lynx-family/tools-shared.git",
        "commit": "18c1fbff9e74d90567dad87ae422be27d31be278",
        'deps_file': 'dependencies/DEPS',
        "ignore_in_git": True,
    },
    'third_party/harfbuzz': {
        'type': 'git',
        'url': 'https://github.com/harfbuzz/harfbuzz',
        'commit': 'a070f9ebbe88dc71b248af9731dd49ec93f4e6e6',
        'patches': [
            os.path.join(root_dir, 'patches', 'harfbuzz', '*.patch')
        ],
        "ignore_in_git": True,
    },
    'third_party/icu': {
        "type": "git",
        "url": "https://chromium.googlesource.com/chromium/deps/icu",
        "commit": "f90543d272e2e2edc5b3cdf8ead0b5b3eebceef5",
        "ignore_in_git": True,
    },
    'third_party/skity': {
        "type": "solution",
        "url": "https://github.com/lynx-family/skity",
        "commit": "fed200b18481503c797a8f712fa315fe54df045d",
        "deps_file": "hab/DEPS",
        "ignore_in_git": True,
    },
    "third_party/skity/third_party/libpng": {
        "type": "git",
        "url": "https://github.com/pnggroup/libpng.git",
        "ignore_in_git": True,
        "commit": "f5e92d76973a7a53f517579bc95d61483bf108c0",
    },
    "third_party/skity/third_party/libjpeg-turbo": {
        "type": "git",
        "url": "https://github.com/libjpeg-turbo/libjpeg-turbo.git",
        "ignore_in_git": True,
        "commit": "f29eda648547b36aa594c4116c7764a6c8a079b9",
    },
    'third_party/rapidjson': {
        'type': 'git',
        'url': 'https://fuchsia.googlesource.com/third_party/rapidjson',
        'commit': 'ef3564c5c8824989393b87df25355baf35ff544b',
        'ignore_in_git': True,
    },
    "third_party/glfw": {
        "type": "git",
        "url": "https://github.com/glfw/glfw",
        "commit": "b42da9f09d6732784223ba5f9a0bda6433366ba8",
        "ignore_in_git": True,
        "condition": system in ["linux", "darwin", "windows"],
    },
    'buildtools/gn': {
        "type": "http",
        "url": f"https://github.com/lynx-family/buildtools/releases/download/gn-cc28efe6/buildtools-gn-{system}-{machine}.tar.gz",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'buildtools/llvm': {
        "type": "http",
        'url': f"https://github.com/lynx-family/buildtools/releases/download/llvm-020d2fb7/buildtools-llvm-{system}-{machine}.tar.gz",
        "ignore_in_git": True,
        "decompress": True,
        "condition": system in ['linux', 'darwin'],
    },
    'buildtools/ninja': {
        "type": "http",
        "url": {
            "linux": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip",
            "darwin": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-mac.zip",
            "windows": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip"
        }.get(system, None),
        "sha256": {
            "linux": "b901ba96e486dce377f9a070ed4ef3f79deb45f4ffe2938f8e7ddc69cfb3df77",
            "darwin": "482ecb23c59ae3d4f158029112de172dd96bb0e97549c4b1ca32d8fad11f873e",
            "windows": "524b344a1a9a55005eaf868d991e090ab8ce07fa109f1820d40e74642e289abc"
        }.get(system, None),
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'buildtools/cmake': {
        "type": "http",
        "url": {
            "linux": f"https://cmake.org/files/v3.18/cmake-3.18.1-Linux-x86_64.tar.gz",
            "darwin": f"https://dl.google.com/android/repository/ba34c321f92f6e6fd696c8354c262c122f56abf8.cmake-3.18.1-darwin.zip",
            "windows": f"https://cmake.org/files/v3.18/cmake-3.18.1-win64-x64.zip"
        }.get(system, None),
        "sha256": {
            "linux": "537de8ad3a7fb4ec9b8517870db255802ad211aec00002c651e178848f7a769e",
            "darwin": "b15d6d7ab5615a48bb14962f5a931be6cd9a0c187f4bd6be404bdd46a7bef60b",
            "windows": "2c6c06da43c1088fc3a673e4440c8ebb1531bb6511134892c0589aa0b94f11ad"
        }.get(system, None),
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'build': {
        "type": "git",
        "url": "https://github.com/lynx-family/buildroot.git",
        "commit": "79446975604356f28b44c4e67851b3c9aafa372f",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'third_party/libcxx': {
        "type": "git",
        "url": "https://chromium.googlesource.com/external/github.com/llvm/llvm-project/libcxx",
        "commit": "64d36e572d3f9719c5d75011a718f33f11126851",
        "ignore_in_git": True,
    },
    'third_party/libcxxabi': {
        "type": "git",
        "url": "https://chromium.googlesource.com/external/github.com/llvm/llvm-project/libcxxabi",
        "commit": "9572e56a12c88c011d504a707ca94952be4664f9",
        "ignore_in_git": True,
    },
    'third_party/googletest': {
        'type': 'git',
        'url': 'https://github.com/google/googletest',
        'commit': '4a00a24fff3cf82254de382437bf840cab1d3993',
        'ignore_in_git': True,
    },
    'third_party/gyp': {
        "type": "git",
        "url": "https://chromium.googlesource.com/external/gyp",
        "commit": "9d09418933ea2f75cc416e5ce38d15f62acd5c9a",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows'],
    },
}