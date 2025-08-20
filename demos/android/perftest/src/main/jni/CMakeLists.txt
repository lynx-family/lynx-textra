# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.6)
# Declares and names the project.

project("PerfTest")
set(CMAKE_CXX_STANDARD 17)
add_library(perf_test SHARED
        perf_test.cc
        tt_text_manager.cc
        tt_text_manager.h
)

set(textlayout_root_path ${PROJECT_SOURCE_DIR}/../../../../../..)
target_include_directories(perf_test PRIVATE ${textlayout_root_path}/third_party/icu/source/common)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

set(LYNXTEXTRA_ARTIFACT lynxtextra)
find_package(${LYNXTEXTRA_ARTIFACT} CONFIG)
if (${${LYNXTEXTRA_ARTIFACT}_FOUND})
    MESSAGE(WARNING "LynxTextra found. CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
    get_target_property(LYNXTEXTRA_INCLUDE_DIR ${LYNXTEXTRA_ARTIFACT}::lynxtextra INTERFACE_INCLUDE_DIRECTORIES)
    target_link_libraries(perf_test ${LYNXTEXTRA_ARTIFACT}::lynxtextra)
else ()
    MESSAGE(WARNING "LynxTextra NOT found. CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
endif ()

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
        perf_test
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        z)
