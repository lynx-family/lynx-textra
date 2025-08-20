link_directories(${TEXTLAYOUT_DIRECTORY}/libs)
add_definitions(-DSK_RELEASE -DSK_GL=1 -DSKP_TEST)

set(CMAKE_CXX_STANDARD 17)

set(skia_include
        ${TEXTLAYOUT_DIRECTORY}/
        ${TEXTLAYOUT_DIRECTORY}/public
        ${TEXTLAYOUT_DIRECTORY}/include
        ${TEXTLAYOUT_DIRECTORY}/include/skia
        ${TEXTLAYOUT_DIRECTORY}/include/skia/include
        ${TEXTLAYOUT_DIRECTORY}/include/skia/include/core
        ${TEXTLAYOUT_DIRECTORY}/include/skia/include/config
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools
        ${TEXTLAYOUT_DIRECTORY}/include/skia/modules/skshaper/include
        ${TEXTLAYOUT_DIRECTORY}/include/skia/modules/skparagraph/include
        ${TEXTLAYOUT_DIRECTORY}/include/skia/modules/skunicode/include
        )

add_library(lynxtextra SHARED
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_typeface_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_font_manager.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_painter.cc
        )
target_link_libraries(lynxtextra
        textlayout
        skia
        skunicode
        skshaper
        skparagraph)
target_include_directories(lynxtextra PUBLIC ${skia_include})

add_executable(sk_app_demo
        ttreaderdemo/SkiaAppDemo.cpp
        ttreaderdemo/paragraph_test.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_typeface_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_font_manager.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_painter.cc
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/Window.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/GLWindowContext.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/WindowContext.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/main_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/Window_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/GLWindowContext_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/RasterWindowContext_mac.mm)
target_compile_features(sk_app_demo PRIVATE cxx_noexcept)

target_include_directories(sk_app_demo PUBLIC ${skia_include})
target_link_libraries(sk_app_demo
        textlayout
        skia
        skunicode
        skshaper
        skparagraph
        harfbuzz
        z
        "-framework AppKit"
        "-framework CoreData"
        "-framework CoreTelephony"
        "-framework GLUT"
        "-framework OpenGL"
        )
target_link_options(sk_app_demo PRIVATE ${ASAN_COMPILE_OPTION})

add_executable(skparagraph_demo
        skparagraph_demo/SkParagraphDemo.cpp
        skparagraph_demo/skparagraph_test.cc
        skparagraph_demo/test_font_mgr.cc
        skparagraph_demo/test_font_mgr.h
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_typeface_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_painter.cc
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/Window.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/GLWindowContext.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/WindowContext.cpp
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/main_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/Window_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/GLWindowContext_mac.mm
        ${TEXTLAYOUT_DIRECTORY}/include/skia/tools/sk_app/mac/RasterWindowContext_mac.mm)
target_compile_features(skparagraph_demo PRIVATE cxx_noexcept)
target_include_directories(skparagraph_demo PUBLIC ${skia_include})
target_link_libraries(skparagraph_demo
        skparagraph
        textlayout
        skia
        skunicode
        skshaper
        skparagraph
        harfbuzz
        z
        "-framework AppKit"
        "-framework CoreData"
        "-framework CoreTelephony"
        "-framework GLUT"
        "-framework OpenGL"
        )
target_link_options(skparagraph_demo PRIVATE ${ASAN_COMPILE_OPTION})

add_executable(ttreaderdemo
        ttreaderdemo/TTReaderDemo.cc
        ttreaderdemo/paragraph_test.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_typeface_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_font_manager.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_painter.cc
        )
target_compile_features(ttreaderdemo PRIVATE cxx_noexcept)
target_link_libraries(ttreaderdemo
        textlayout
        z
        skia
        skunicode
        skshaper
        skparagraph
        "-framework AppKit"
        "-framework CoreData"
        "-framework CoreTelephony"
        "-framework GLUT"
        "-framework OpenGL"
        )
target_link_options(ttreaderdemo PRIVATE ${ASAN_COMPILE_OPTION})
target_compile_options(ttreaderdemo PRIVATE ${ASAN_COMPILE_OPTION})
target_include_directories(ttreaderdemo PRIVATE ${skia_include})

remove_definitions(-DSKP_TEST)
add_subdirectory(${TEXTLAYOUT_DIRECTORY}/thirdparty/glfw-3.3.7 glfw.out)
function(init_target target)
    target_sources(${target} PRIVATE
            )
    target_include_directories(${target} PUBLIC
            ttreaderdemo
            ${TEXTLAYOUT_DIRECTORY}/include/skia
            ${TEXTLAYOUT_DIRECTORY}/include/skia/include
            ${TEXTLAYOUT_DIRECTORY}/include/freetype/include
            ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skity/
            ${TEXTLAYOUT_DIRECTORY}/thirdparty/skity/include
            ${TEXTLAYOUT_DIRECTORY}/thirdparty/skity/third_party/glm
            ${TEXTLAYOUT_DIRECTORY}/thirdparty/glfw-3.3.7/include
            )
    target_link_libraries(${target} PUBLIC
            z
            textlayout
            harfbuzz
            skia
            glfw
            "-framework OpenGL")
    target_compile_definitions(${target} PRIVATE
            MAIN_FONT_PATH=\"${TEXTLAYOUT_DIRECTORY}/resource/font/Noto_Sans_CJK/NotoSansCJK-Regular.ttc\"
            EMOJI_FONT_PATH=\"${TEXTLAYOUT_DIRECTORY}/resource/font/Noto_Color_Emoji/NotoColorEmoji-Regular.ttf\"
            )
    #    # Copy files to the runtime output directory
    #    add_custom_command(
    #            TARGET ${target}
    #            POST_BUILD
    #            COMMAND ${CMAKE_COMMAND} -E copy
    #            ${TEXTLAYOUT_DIRECTORY}/libs/${CMAKE_SYSTEM_PROCESSOR}/icu/*.dylib
    #            $<TARGET_FILE_DIR:${target}>
    #    )
endfunction()

add_executable(glappdemo_skity
        glfw/gl_app/gl_app.cc
        glfw/gl_app_demo_skity.cc
        ttreaderdemo/paragraph_test.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skity/skity_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skity/skity_font_manager.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skity/skity_typeface_helper.cc
        )
target_link_libraries(glappdemo_skity PRIVATE skity-static)
init_target(glappdemo_skity)

add_executable(glappdemo_skia
        glfw/gl_app/gl_app.cc
        glfw/gl_app_demo_skia.cc
        ttreaderdemo/paragraph_test.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_canvas_helper.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_font_manager.cc
        ${TEXTLAYOUT_DIRECTORY}/src/ports/platform/skia/skia_typeface_helper.cc
        )
init_target(glappdemo_skia)

