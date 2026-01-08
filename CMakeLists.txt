# Set the minimum version of CMAKE that is required
cmake_minimum_required(VERSION 3.10)
project(lynxtextra)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TEXTRA_ENABLE_SKIA "option for enable skia backend" OFF)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(BUILD_TYPE "is_debug=false")
else ()
    set(BUILD_TYPE "is_debug=true")
endif ()

execute_process(
        COMMAND source ${PROJECT_SOURCE_DIR}/tools/envsetup.sh
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE result
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Result: ${result}")
execute_process(
        COMMAND python3 ${PROJECT_SOURCE_DIR}/tools/build_cmake_environment.py
        --gn-args "${BUILD_TYPE} use_flutter_cxx=false"
        OUTPUT_VARIABLE result
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Result: ${result}")

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/skity skity.out)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/glfw glfw.out)

# According to the build variant, 
# import the CMakeLists-impl.cmake file to compile the native methods.
#add_subdirectory("src")
add_subdirectory("platform/mac")
add_subdirectory("demos/darwin/macos/mtl")
add_subdirectory("test")
add_subdirectory("examples/json_parser")
add_subdirectory("examples/json_parser_exe")


if (${TEXTRA_ENABLE_SKIA})
    execute_process(
            COMMAND source ${PROJECT_SOURCE_DIR}/tools/fetch_and_build_skia.sh
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            OUTPUT_VARIABLE result
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Result: ${result}")
    add_subdirectory("demos/darwin/macos/skia_app_demo")
    add_subdirectory("demos/darwin/macos/ttreaderdemo")
endif ()
