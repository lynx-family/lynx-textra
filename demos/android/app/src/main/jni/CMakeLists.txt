# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.6)
# Declares and names the project.

project("AndroidTest")
set(CMAKE_CXX_STANDARD 17)

set(textlayout_root_path ${PROJECT_SOURCE_DIR}/../../../../../..)
include_directories(${textlayout_root_path}/third_party/icu/source/common)

add_subdirectory(${textlayout_root_path}/examples/json_parser ${CMAKE_CURRENT_BINARY_DIR}/json_parser)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library(android_test SHARED
        paragraph_test.cc
        android_test.cc
        buffer_input_stream.cc
        tttext_native.cc
        tttext_run_delegate.cc
)

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
#  include_directories(${LYNXTEXTRA_INCLUDE_DIR}/textlayout)
  target_link_libraries(android_test ${LYNXTEXTRA_ARTIFACT}::lynxtextra)
else ()
  MESSAGE(WARNING "LynxTextra NOT found. CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
endif ()

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
        android_test
        json_parser
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        z)
