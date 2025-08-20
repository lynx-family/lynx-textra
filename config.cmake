IF (CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build.")
    set(ASAN_COMPILE_OPTION "")
    add_definitions(-DDEBUG=1)

ELSEIF (CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build.")
    set(ASAN_COMPILE_OPTION "")
    add_definitions(-DNDEBUG)
    set(ENABLE_GTEST OFF)
ELSEIF (CMAKE_BUILD_TYPE MATCHES Asan)
    message("Asan build.")
    add_definitions(-DDEBUG -DNDEBUG)
    add_compile_options(-g -O0 -mllvm -asan-use-private-alias=1)
    set(ASAN_COMPILE_OPTION -fsanitize=address)
    set(ENABLE_GTEST OFF)
else ()
    message("Some other build type.")
    set(ENABLE_GTEST OFF)
ENDIF ()

if (STATIC_BUILD)
    set(BUILD_TYPE STATIC CACHE STRING "Build Static Library")
else ()
    set(BUILD_TYPE SHARED CACHE STRING "Build Shared Library")
endif ()

if (USE_ICU)
    message("use icu")
    add_definitions(-DUSE_ICU)
endif ()

if (USE_SKITY)
    add_definitions(-DUSE_SKITY)
endif ()

if (USE_MINIKIN_SHAPER)
    add_definitions(-DSELF_RENDERING)
endif ()


if (ENABLE_GTEST)
    add_definitions(-DENABLE_GTEST)
    include(FetchContent)
    cmake_policy(SET CMP0135 NEW)
    set(DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
    )

    FetchContent_MakeAvailable(googletest)
    enable_testing()
    message("gtest src:" ${googletest_SOURCE_DIR})
    include_directories(${googletest_SOURCE_DIR}/googletest/include)
endif ()