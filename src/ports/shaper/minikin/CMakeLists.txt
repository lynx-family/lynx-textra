target_include_directories(textlayout PRIVATE . lib)
target_include_directories(textlayout PRIVATE .)
target_sources(textlayout PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/CmapCoverage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/Emoji.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FontCollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FontFamily.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FontUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/LayoutCore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/Locale.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/LocaleListCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/MinikinInternal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/SparseBitSet.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/SystemFonts.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shaper_minikin.cc
)
if (USE_SKITY)
    target_sources(textlayout PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/font/skity_minikin_font.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/font/skity_font_manager.cc
    )
else ()
    target_sources(textlayout PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/font/sk_minikin_font.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/font/sk_font_manager.cc
    )
endif ()