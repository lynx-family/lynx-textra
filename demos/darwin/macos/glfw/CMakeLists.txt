include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(gl_app_demo
        "-framework AppKit"
        "-framework CoreText"
        "-framework CoreVideo"
        "-framework IOKit"
        "-framework OpenGL"
        "-framework MetalKit"
        "-framework Metal"
        skity
)
