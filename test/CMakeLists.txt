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
    add_subdirectory(${PROJECT_SOURCE_DIR}/public/textra/capi/platform host_adapters)
    add_executable(SkiaHeaderOnlyTest capi/skia_header_only_test.cc capi/skia_header_peer.cc)
    target_link_libraries(SkiaHeaderOnlyTest PRIVATE textra_capi_skia)
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
target_sources(TextLayoutLiteTest PRIVATE
        ${PROJECT_SOURCE_DIR}/test/skity_coretext_capi_test.cc)

# Compile the host adapter clients as C in separate translation units. Skity's
# static binary requires a C++ linker, but no C++ adapter source is compiled.
add_executable(SkityHeaderOnlyTest
        capi/skity_header_only_test.c capi/skity_header_peer.c)
target_include_directories(SkityHeaderOnlyTest PRIVATE
        ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/public)
target_compile_features(SkityHeaderOnlyTest PRIVATE c_std_11)
set_target_properties(SkityHeaderOnlyTest PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(SkityHeaderOnlyTest PRIVATE skity)
enable_testing()
add_test(NAME SkityHeaderOnlyTest COMMAND SkityHeaderOnlyTest)
