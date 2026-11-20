cmake_minimum_required(VERSION 3.10)
project(skia_app_demo)
set(PRJROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)

set(CMAKE_CXX_STANDARD 17)

add_executable(ttreaderdemo
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/TTReaderDemo.cc
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.h
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.cc
)
target_include_directories(ttreaderdemo PUBLIC
        ${PRJROOT}
        ${PRJROOT}/public
)

target_compile_definitions(ttreaderdemo PRIVATE
        -DFONT_ROOT="${PRJROOT}/fonts/")

target_compile_features(ttreaderdemo PRIVATE cxx_noexcept)
target_link_libraries(ttreaderdemo
        skia
        lynxtextra
)
add_executable(SkiaPrebuiltSmoke ${PRJROOT}/test/skia_prebuilt_smoke.cc)
target_include_directories(SkiaPrebuiltSmoke PRIVATE ${PRJROOT}/public)
target_link_libraries(SkiaPrebuiltSmoke PRIVATE skia lynxtextra)
