# the minimum version of CMake.
cmake_minimum_required(VERSION 3.10)

#include($ENV{HARMONY_HOME}/sdk/default/openharmony/native/build/cmake/ohos.toolchain.cmake)


project(lynxtextra)

set(CMAKE_CXX_STANDARD 17)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

set(PROJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
message(STATUS ${PROJ_ROOT})
message(STATUS "BuildType: ${CMAKE_BUILD_TYPE}")

if (${CMAKE_OHOS_ARCH_ABI} STREQUAL "arm64-v8a")
   set(ABI "arm64")
else()
   set(ABI "arm")
endif()
set(FLAVOR "flavor")
set(GN_ARGS "lynxtextra=true enable_lto=true use_flutter_cxx=false")
set(GN_TARGET "//platform/harmony/lynxtextra:lynxtextra")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TYPE "debug")
    set(IS_DEBUG "--is-debug")
else ()
    set(BUILD_TYPE "release")
    set(IS_DEBUG "")
endif ()

message(STATUS "=================GENERATE CMAKE================")
message(STATUS "${GN_ARGS}")
message(STATUS ${result})
execute_process(
        COMMAND python3 tools/harmony_tools/generate_cmake_script_by_gn.py --harmony --harmony-cpu ${ABI} --ide=json ${IS_DEBUG} --gn-args=${GN_ARGS}
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
execute_process(
        COMMAND python3 tools/gn_tools/gn_to_cmake.py --json-path out/harmony_${BUILD_TYPE}_${ABI}/project.json --start-target ${GN_TARGET} --cmake-version 3.10 --keep-libs ''
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
execute_process(
        COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/${BUILD_TYPE}/${ABI}
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
execute_process(
        COMMAND cp out/harmony_${BUILD_TYPE}_${ABI}/CMakeLists/platform/harmony/lynxtextra/CMakeLists.txt ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/${BUILD_TYPE}/${ABI}/CMakeLists-impl.cmake
        WORKING_DIRECTORY ${PROJ_ROOT}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
)
message(STATUS ${result})
message(STATUS "=================GENERATE CMAKE END================")

# According to the build variant,
# import the CMakeLists-impl.cmake file to compile the native methods.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=shorten-64-to-32 -Wshorten-64-to-32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=shorten-64-to-32 -Wshorten-64-to-32")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto=full -Wl,--gc-sections -Wl,--hash-style=sysv")
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/${BUILD_TYPE}/${ABI}/CMakeLists-impl.cmake)
#target_link_libraries(lynxtextra PUBLIC libace_napi.z.so libace_ndk.z.so libnative_drawing.so libnative_display_manager.so)