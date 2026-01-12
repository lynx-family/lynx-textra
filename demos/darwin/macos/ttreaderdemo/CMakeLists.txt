cmake_minimum_required(VERSION 3.10)
project(skia_app_demo)
set(PRJROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
set(SKIAROOT ${PRJROOT}/third_party/skia)
link_directories(${CMAKE_BINARY_DIR}/skia)

set(CMAKE_CXX_STANDARD 17)

add_executable(ttreaderdemo
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/TTReaderDemo.cc
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.h
        ${PRJROOT}/demos/darwin/macos/ttreaderdemo/paragraph_test.cc
)
add_dependencies(ttreaderdemo FetchAndBuildSkia)
target_include_directories(ttreaderdemo PUBLIC
        ${PRJROOT}
        ${PRJROOT}/public
        ${SKIAROOT}
)

target_compile_definitions(ttreaderdemo PRIVATE
        -DFONT_ROOT="${PRJROOT}/fonts/")

target_compile_features(ttreaderdemo PRIVATE cxx_noexcept)
target_link_libraries(ttreaderdemo
        skia
        lynxtextra
)