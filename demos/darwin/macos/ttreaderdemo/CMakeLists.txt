include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)

target_link_libraries(ttreaderdemo
        "-framework AppKit"
        "-framework CoreData"
        "-framework CoreTelephony"
        "-framework GLUT"
        "-framework OpenGL"
)