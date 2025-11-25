include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(lynxtextra PUBLIC
    "-framework AppKit"
    "-framework CoreText"
)