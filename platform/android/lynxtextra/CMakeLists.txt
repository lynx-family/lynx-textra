# Set the minimum version of CMAKE that is required
cmake_minimum_required(VERSION 3.10)
project(lynxtextra)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if (DEFINED ENV{ANDROID_NDK})
    message(STATUS "ANDROID_NDK: $ENV{ANDROID_NDK}")
    include($ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake)
    include($ENV{ANDROID_NDK}/build/cmake/abis.cmake)
    include($ENV{ANDROID_NDK}/build/cmake/platforms.cmake)
    include($ENV{ANDROID_NDK}/build/cmake/system_libs.cmake)
else ()
    message(STATUS "ANDROID_NDK not found")
endif ()

set(PROJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
message(STATUS ${PROJ_ROOT})
message(STATUS "BuildType: ${CMAKE_BUILD_TYPE}")

set(FLAVOR "flavor")
set(GN_ARGS "enable_lto=true,use_flutter_cxx=false,flavor_name=${FLAVOR}")
set(GN_TARGET "//platform/android/lynxtextra:lynxtextra")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TYPE "debug")
    string(APPEND GN_ARGS ",is_debug=true")
else ()
    set(BUILD_TYPE "release")
    string(APPEND GN_ARGS ",is_debug=false")
endif ()

message(STATUS "=================GENERATE CMAKE================")
execute_process(
        COMMAND python3 tools/android_tools/generate_cmake_script_by_gn.py --gen-file project.json --abi ${CMAKE_ANDROID_ARCH_ABI} --flavor ${FLAVOR} --build-type ${BUILD_TYPE} --gn-args ${GN_ARGS}
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
execute_process(
        COMMAND python3 tools/android_tools/generate_cmake_script_by_gn.py --gen-file CMakeLists.txt --abi ${CMAKE_ANDROID_ARCH_ABI} --flavor ${FLAVOR} --build-type ${BUILD_TYPE} --target ${GN_TARGET} --cmake-version 3.10
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
message(STATUS "=================GENERATE CMAKE END================")

# According to the build variant,
# import the CMakeLists-impl.cmake file to compile the native methods.
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=full -Wl,--gc-sections -Wl,--hash-style=sysv -fno-rtti -fno-exceptions")
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/${FLAVOR}${BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}/CMakeLists-impl.cmake)

find_library(log-lib log)
target_link_libraries(lynxtextra
        ${log-lib}
)
