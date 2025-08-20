if (ANDROID)

    set(FLAVOR "flavor")
    set(GN_ARGS "flavor_name=${FLAVOR}")
    set(GN_TARGET "//examples/json_parser:json_parser")
    
    set(textlayout_root_path ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(BUILD_TYPE "debug")
        string(APPEND GN_ARGS ",is_debug=true")
    else ()
        set(BUILD_TYPE "release")
        string(APPEND GN_ARGS ",is_debug=false")
    endif ()
    
    # Generate project.json
    execute_process(
        COMMAND python3 
            ${textlayout_root_path}/tools/android_tools/generate_cmake_script_by_gn.py 
            --gen-file project.json 
            --abi ${CMAKE_ANDROID_ARCH_ABI} 
            --flavor ${FLAVOR} 
            --build-type ${BUILD_TYPE} 
            --gn-args ${GN_ARGS}
        WORKING_DIRECTORY ${textlayout_root_path}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
    )
    message(STATUS "json_parser: Generate project.json result: ${result}")
    
    # Generate json_parser's CMakeLists-impl.cmake
    execute_process(
        COMMAND python3 
            ${textlayout_root_path}/tools/android_tools/generate_cmake_script_by_gn.py 
            --gen-file CMakeLists.txt 
            --abi ${CMAKE_ANDROID_ARCH_ABI} 
            --flavor ${FLAVOR} 
            --build-type ${BUILD_TYPE} 
            --target ${GN_TARGET} 
            --cmake-version 3.6
        WORKING_DIRECTORY ${textlayout_root_path}
        OUTPUT_VARIABLE result
        ERROR_VARIABLE result
    )
    message(STATUS "json_parser: Generate CMakeLists.txt result: ${result}")

    # Include the generated CMakeLists-impl.cmake
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/${FLAVOR}${BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}/CMakeLists-impl.cmake)
    
    # Expose include directory to the linked library
    target_include_directories(json_parser PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR} 
        ${textlayout_root_path})

elseif (APPLE)

    # For macOS platform, use the existing CMakeLists-impl.cmake
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_impl/impl/CMakeLists-impl.cmake)
    
    # Link frameworks and libraries
    target_link_libraries(json_parser PUBLIC
              "-framework AppKit"
              "-framework CoreText"
              "-framework QuartzCore"
              "-framework MetalKit"
              "-framework Metal"
              "-framework IOKit"
              skity
              skity-codec
    )

endif()
