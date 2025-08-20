# Demos

Demo apps are provided on several platforms to demonstrate the usage of LynxTextra,
including:
- MacOS
- iOS
- Android
- OpenHarmony

The top-level build system is CMake, with some build scripts written in GN and
converted to CMake/Podspec via custom python/bash scripts. See detailed
instructions below on how to build and run the demo apps.

## Environment Setup

Run the following command to setup the environment, regardless of the platform you are building for.

    ```sh
    source tools/envsetup.sh
    tools/hab sync .
    ```

## MacOS Demo App

Supports two graphics backends: OpenGL and Metal. 

    ```sh
    # Build the demo app
    # Choose one of the following commands:
    # OpenGL Demo
    gn gen out/macos --args='is_debug=false'
    ninja -C out/macos gl_app_demo

    # Metal Demo
    gn gen out/macos --args='is_debug=false skity_mtl_backend=true'
    ninja -C out/macos mtl_app_demo

    # Run the demo app
    ./out/macos/gl_app_demo
    ```

## iOS Demo App

    ```sh
    cd demos/darwin/ios/TextlayoutDemo
    ./bundle_install.sh

    # Open TextlayoutDemo.xcworkspace in Xcode
    # Run the demo app (on simulator or real device)
    ```

## Android Demo App

    ```sh
    cd demos/android/app

    # Open the project in Android Studio
    # Run the demo app (on simulator or real device)
    ```

## OpenHarmony Demo App

    ```sh
    source tools/envsetup.sh --target harmony
    cd demos/harmony/lynxtextra_demo
    ohpm install --all
    hvigorw --no-daemon clean assembleHar -p buildMode=release -p module=lynxtextra@default
    ```

## MacOS JSON-to-Image CLI Tool

A CLI executable that converts a JSON file to an image, mostly used for creating images in documentation.

    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release -DSKITY_MTL_BACKEND=ON -G Ninja -S . -B build-examples
    cmake --build build-examples -t json_to_image_app
    ./build-examples/examples/json_parser_exe/json_to_image_app <path_to_json_file>
    ```