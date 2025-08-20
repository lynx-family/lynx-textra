include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(skia_app_demo
        "-framework AppKit"
        "-framework CoreData"
        "-framework CoreTelephony"
        "-framework GLUT"
        "-framework OpenGL"
        )