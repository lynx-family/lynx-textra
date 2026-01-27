include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=shorten-64-to-32 -Wshorten-64-to-32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=sign-compare -Wsign-compare")

target_link_libraries(lynxtextra PUBLIC
    "-framework AppKit"
    "-framework CoreText"
)