include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

if (IS_LVGL)
    # Match the plain link signature emitted by the GN-to-CMake generator.
    target_link_libraries(lynxtextra lvgl)
endif ()

if(TEXTRA_ENABLE_SKIA)
    target_link_libraries(lynxtextra textra_skia_runtime)
endif()
