cmake_minimum_required(VERSION 3.10)
project(skia_app_demo)
set(PRJROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(SKIAROOT ${PRJROOT}/third_party/skia)
link_directories(${SKIAROOT}/out/Static)

set(CMAKE_CXX_STANDARD 17)

add_executable(skia_app_demo
        ${PRJROOT}/demos/darwin/macos/skia_app_demo/SkiaAppDemo.cpp
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.h
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.cc
        ${SKIAROOT}/tools/sk_app/Window.cpp
        ${SKIAROOT}/tools/sk_app/mac/main_mac.mm
        ${SKIAROOT}/tools/sk_app/mac/Window_mac.mm
)
target_include_directories(skia_app_demo PUBLIC
        ${PRJROOT}
        ${PRJROOT}/public
        ${SKIAROOT}
)

target_compile_definitions(skia_app_demo PRIVATE
    -DSK_METAL
    -DSK_GANESH
    -DSK_RELEASE
    -DFONT_ROOT="${PRJROOT}/fonts/")

target_compile_features(skia_app_demo PRIVATE cxx_noexcept)
target_link_libraries(skia_app_demo
        "-framework AppKit"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
        skia
        window
        lynxtextra
        )