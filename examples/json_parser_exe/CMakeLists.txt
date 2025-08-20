include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(json_to_image_app PRIVATE
        "-framework AppKit"
        "-framework CoreText"
        "-framework QuartzCore"
        "-framework MetalKit"
        "-framework Metal"
        "-framework IOKit"
        skity
        skity-codec
)