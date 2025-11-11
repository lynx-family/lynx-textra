# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.6)
# Declares and names the project.

project("SelfRender")
set(CMAKE_CXX_STANDARD 17)
add_library(skity_demo SHARED
        android_gl_example.cc
        android_skity_jni.cc
        android_skity_render.cc
        android_skity_render.hpp
)

set(textlayout_root_path ${PROJECT_SOURCE_DIR}/../../../../../..)
target_include_directories(skity_demo PRIVATE
        ${textlayout_root_path}/third_party/icu/source/common
        ${textlayout_root_path}/third_party/skity/include
        ${textlayout_root_path}/public
)

add_subdirectory(${textlayout_root_path}/third_party/skity skity.out)
target_compile_definitions(skity_demo PRIVATE -DSKITY_JNI_PKG="com/lynx/textra/skitydemo")
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
    target_link_libraries(skity_demo ${LYNXTEXTRA_ARTIFACT}::lynxtextra)
else ()
    MESSAGE(WARNING "LynxTextra NOT found. CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
endif ()

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
        skity_demo
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        skity
        EGL
        GLESv2
        z)
