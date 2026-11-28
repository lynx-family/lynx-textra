# Set the minimum version of CMAKE that is required
cmake_minimum_required(VERSION 3.10)
project(lynxtextra)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TEXTRA_ENABLE_SKIA "option for enable skia backend" OFF)
option(TEXTRA_BUILD_GL_DEMO "Build the macOS OpenGL demo" OFF)
if(TEXTRA_ENABLE_SKIA)
    include(${CMAKE_SOURCE_DIR}/cmake/SkiaRelease.cmake)
endif()

set(GN_ARG_LIST "")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND GN_ARG_LIST "is_debug=false")
else ()
    list(APPEND GN_ARG_LIST "is_debug=true")
endif ()

list(APPEND GN_ARG_LIST "use_flutter_cxx=false" "use_xcode=true")
if(TEXTRA_ENABLE_SKIA)
    # The prebuilt SDK targets Ubuntu 22.04; use the matching host C runtime.
    list(APPEND GN_ARG_LIST "use_default_linux_sysroot=false")
endif()

if (IS_LVGL)
    list(APPEND GN_ARG_LIST "is_lvgl=true")
endif ()

list(JOIN GN_ARG_LIST " " GN_ARGS)

execute_process(
        COMMAND source ${PROJECT_SOURCE_DIR}/tools/envsetup.sh
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE result
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Result: ${result}")

set(CMD python3 ${PROJECT_SOURCE_DIR}/tools/build_cmake_environment.py
        --gn-args ${GN_ARGS})
message(STATUS "executing: ${CMD}")
execute_process(
        COMMAND ${CMD}
        ERROR_VARIABLE result
        ERROR_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Result: ${result}")

function(GEN_CMAKE target)
    set(CMD python3 ${PROJECT_SOURCE_DIR}/tools/build_cmake_environment.py
            --target ${target})
    message(STATUS "executing GEN_CMAKE: ${target}")
    execute_process(
            COMMAND ${CMD}
            ERROR_VARIABLE result
            ERROR_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Result: ${result}")
endfunction()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    GEN_CMAKE("//platform/mac:lynxtextra")
    GEN_CMAKE("//demos/darwin/macos/glfw:gl_app_demo")
    GEN_CMAKE("//demos/darwin/macos/mtl:mtl_app_demo")
    GEN_CMAKE("//test:TextLayoutLiteTest")
    GEN_CMAKE("//examples/json_parser:json_parser")
    GEN_CMAKE("//examples/json_parser_exe:json_to_image_app")

    include(${CMAKE_SOURCE_DIR}/cmake/SkityRelease.cmake)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/glfw glfw.out)

    add_subdirectory("platform/mac")
    add_subdirectory("demos/darwin/macos/mtl")
    if(TEXTRA_BUILD_GL_DEMO)
        add_subdirectory("demos/darwin/macos/glfw")
    endif()
    add_subdirectory("test")
    add_subdirectory("examples/json_parser")
    add_subdirectory("examples/json_parser_exe")
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    GEN_CMAKE("//platform/linux:lynxtextra")
    add_subdirectory("platform/linux")
    if(TEXTRA_ENABLE_SKIA)
        add_subdirectory("test")
    endif()
endif ()

if (${IS_LVGL})
    add_subdirectory(demos/lvgl)
endif ()

if (${TEXTRA_ENABLE_SKIA})
    add_subdirectory("demos/darwin/macos/ttreaderdemo")
endif ()
