cmake_minimum_required(VERSION 3.12.4)
project(lvgl LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LV_BUILD_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE PATH "Config path" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "Build demos" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)

target_compile_definitions(lynxtextra PUBLIC LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/lvgl lvgl.dir)

# Find and include SDL2 library
find_package(SDL2 REQUIRED)

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

set(MAIN_SOURCES mouse_cursor_icon.c hal/hal.c)
set(MAIN_LIBS lvgl ${SDL2_LIBRARIES} lynxtextra)

# Create the main executable, depending on the FreeRTOS option
list(APPEND MAIN_SOURCES main.cc)

if (NOT MSVC)
    list(APPEND MAIN_LIBS m pthread)
endif ()

add_executable(lvgl_demo ${MAIN_SOURCES})
target_include_directories(lvgl_demo PRIVATE ${CMAKE_SOURCE_DIR}/public ${CMAKE_SOURCE_DIR}/third_party)
target_compile_definitions(lvgl_demo PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(lvgl_demo ${MAIN_LIBS})

# On Windows, GUI applications do not show a console by default,
# which hides log output. This ensures a console is available for logging.
if (WIN32)
    if (MSVC)
        target_link_options(mlvgl_demon PRIVATE "/SUBSYSTEM:CONSOLE")
    else ()
        target_link_options(lvgl_demo PRIVATE "-mconsole")
    endif ()
endif ()

# Apply additional compile options if the build type is Debug
if (ASAN)
    message(STATUS "AddressSanitizer enabled")

    # Add AddressSanitizer flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
else ()
    message(STATUS "AddressSanitizer disabled")
endif ()
