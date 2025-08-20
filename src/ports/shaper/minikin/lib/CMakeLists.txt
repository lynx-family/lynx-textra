include_directories(.)
set(minikin_src
        #        BidiUtils.cpp
        CmapCoverage.cpp
        Emoji.cpp
        FontCollection.cpp
        FontFamily.cpp
        FontUtils.cpp
        #        GraphemeBreak.cpp
        #        GreedyLineBreaker.cpp
        #        Hyphenator.cpp
        #        HyphenatorMap.cpp
        #        Layout.cpp
        LayoutCore.cpp
        #        LayoutUtils.cpp
        #        LineBreaker.cpp
        #        LineBreakerUtil.cpp
        Locale.cpp
        LocaleListCache.cpp
        #        MeasuredText.cpp
        #        Measurement.cpp
        MinikinInternal.cpp
        #        OptimalLineBreaker.cpp
        SparseBitSet.cpp
        SystemFonts.cpp
        #        WordBreaker.cpp
)
target_sources(textlayout PUBLIC ${minikin_src})