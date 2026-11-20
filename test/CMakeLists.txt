if(TEXTRA_ENABLE_SKIA)
    add_executable(GradientSkiaTest
            gradient_skia_test.cc
            ${PROJECT_SOURCE_DIR}/src/textlayout/style/gradient.cc
            ${PROJECT_SOURCE_DIR}/src/textlayout/font_info.cc
            ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/src/gtest-all.cc
            ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/src/gtest_main.cc
    )
    target_include_directories(GradientSkiaTest PRIVATE
            ${PROJECT_SOURCE_DIR}/public
            ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include
            ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest
    )
    target_compile_definitions(GradientSkiaTest PRIVATE
            FONT_ROOT="${PROJECT_SOURCE_DIR}/fonts/"
    )
    target_link_libraries(GradientSkiaTest PRIVATE skia)
    return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(TextLayoutLiteTest PRIVATE
        "-framework AppKit"
        "-framework CoreText"
        "-framework QuartzCore"
        "-framework MetalKit"
        "-framework Metal"
        "-framework IOKit"
        skity
)

target_sources(TextLayoutLiteTest PRIVATE ${PROJECT_SOURCE_DIR}/demos/darwin/macos/skity_png.cc)
