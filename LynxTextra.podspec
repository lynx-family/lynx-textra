# Copyright 2019 The Lynx Authors. All rights reserved.
# coding: utf-8

Pod::Spec.new do |s|
  s.name                     = "LynxTextra"
  s.version                  = "1.0.0"
  s.summary                  = "LynxTextra is a high-performance text layout engine."
  s.description              = <<-DESC
    LynxTextra is a high-performance, cross-platform text layout engine.
    This podspec allows you to use LynxTextra in your iOS project.
  DESC
  s.homepage                 = "https://github.com/lynx-family/lynx-textra"
  s.license                  = { :type => "Apache-2.0", :file => "LICENSE" }
  s.author                   = "Lynx"
  s.source                   = { :git => "https://github.com/lynx-family/lynx-textra.git", :tag => s.version.to_s }
  s.ios.deployment_target    = "12.0"
  s.requires_arc             = true


  s.subspec "LynxTextra" do |sp|
    sp.header_mappings_dir    = "public"
    sp.public_header_files    = "public/textra/font_info.h",
                                "public/textra/fontmgr_collection.h",
                                "public/textra/i_canvas_helper.h",
                                "public/textra/i_font_manager.h",
                                "public/textra/i_typeface_helper.h",
                                "public/textra/icu_wrapper.h",
                                "public/textra/layout_definition.h",
                                "public/textra/layout_drawer.h",
                                "public/textra/layout_drawer_listener.h",
                                "public/textra/layout_page_listener.h",
                                "public/textra/layout_region.h",
                                "public/textra/macro.h",
                                "public/textra/painter.h",
                                "public/textra/paragraph.h",
                                "public/textra/paragraph_style.h",
                                "public/textra/platform/ios/ios_canvas_base.h",
                                "public/textra/platform/ios/ios_font_manager.h",
                                "public/textra/platform/ios/typeface_coretext.h",
                                "public/textra/platform/java/buffer_output_stream.h",
                                "public/textra/platform/java/java_canvas_helper.h",
                                "public/textra/platform/java/java_font_manager.h",
                                "public/textra/platform/java/java_typeface.h",
                                "public/textra/platform/java/tttext_jni_proxy.h",
                                "public/textra/platform/skity/skity_canvas_helper.h",
                                "public/textra/platform/skity/skity_font_manager.h",
                                "public/textra/platform/skity/skity_font_manager_coretext.h",
                                "public/textra/platform/skity/skity_typeface_helper.h",
                                "public/textra/platform_helper.h",
                                "public/textra/run_delegate.h",
                                "public/textra/style.h",
                                "public/textra/text_layout.h",
                                "public/textra/text_line.h",
                                "public/textra/tt_color.h",
                                "public/textra/tt_path.h",
                                "public/textra/tttext_context.h"

    sp.private_header_files   = "src/ports/shaper/coretext/*.{h,hpp}",
                                "src/textlayout/*.{h,hpp}",
                                "src/textlayout/internal/*.{h,hpp}",
                                "src/textlayout/run/*.{h,hpp}",
                                "src/textlayout/style/*.{h,hpp}",
                                "src/textlayout/utils/*.{h,hpp}"

    sp.source_files           = "public/textra/font_info.h",
                                "public/textra/fontmgr_collection.h",
                                "public/textra/i_canvas_helper.h",
                                "public/textra/i_font_manager.h",
                                "public/textra/i_typeface_helper.h",
                                "public/textra/icu_wrapper.h",
                                "public/textra/layout_definition.h",
                                "public/textra/layout_drawer.h",
                                "public/textra/layout_drawer_listener.h",
                                "public/textra/layout_page_listener.h",
                                "public/textra/layout_region.h",
                                "public/textra/macro.h",
                                "public/textra/painter.h",
                                "public/textra/paragraph.h",
                                "public/textra/paragraph_style.h",
                                "public/textra/platform/ios/ios_canvas_base.h",
                                "public/textra/platform/ios/ios_font_manager.h",
                                "public/textra/platform/ios/typeface_coretext.h",
                                "public/textra/platform/java/buffer_output_stream.h",
                                "public/textra/platform/java/java_canvas_helper.h",
                                "public/textra/platform/java/java_font_manager.h",
                                "public/textra/platform/java/java_typeface.h",
                                "public/textra/platform/java/tttext_jni_proxy.h",
                                "public/textra/platform/skity/skity_canvas_helper.h",
                                "public/textra/platform/skity/skity_font_manager.h",
                                "public/textra/platform/skity/skity_font_manager_coretext.h",
                                "public/textra/platform/skity/skity_typeface_helper.h",
                                "public/textra/platform_helper.h",
                                "public/textra/run_delegate.h",
                                "public/textra/style.h",
                                "public/textra/text_layout.h",
                                "public/textra/text_line.h",
                                "public/textra/tt_color.h",
                                "public/textra/tt_path.h",
                                "public/textra/tttext_context.h",
                                "src/ports/platform_helper.cc",
                                "src/ports/shaper/coretext/shaper_core_text.h",
                                "src/ports/shaper/coretext/shaper_core_text.mm",
                                "src/textlayout/font_info.cc",
                                "src/textlayout/fontmgr_collection.cc",
                                "src/textlayout/internal/boundary_analyst.cc",
                                "src/textlayout/internal/boundary_analyst.h",
                                "src/textlayout/internal/line_range.h",
                                "src/textlayout/internal/run_range.h",
                                "src/textlayout/layout_drawer.cc",
                                "src/textlayout/layout_measurer.cc",
                                "src/textlayout/layout_measurer.h",
                                "src/textlayout/layout_position.h",
                                "src/textlayout/layout_region.cc",
                                "src/textlayout/paragraph_impl.cc",
                                "src/textlayout/paragraph_impl.h",
                                "src/textlayout/run/base_run.cc",
                                "src/textlayout/run/base_run.h",
                                "src/textlayout/run/ghost_run.h",
                                "src/textlayout/run/layout_metrics.h",
                                "src/textlayout/run/object_run.h",
                                "src/textlayout/shape_cache.h",
                                "src/textlayout/style/paragraph_style.cc",
                                "src/textlayout/style/style.cc",
                                "src/textlayout/style/style_manager.cc",
                                "src/textlayout/style/style_manager.h",
                                "src/textlayout/style/tt_color.cc",
                                "src/textlayout/style_attributes.h",
                                "src/textlayout/text_layout.cc",
                                "src/textlayout/text_layout_impl.cc",
                                "src/textlayout/text_layout_impl.h",
                                "src/textlayout/text_line_impl.cc",
                                "src/textlayout/text_line_impl.h",
                                "src/textlayout/tt_shaper.cc",
                                "src/textlayout/tt_shaper.h",
                                "src/textlayout/tttext_context.cc",
                                "src/textlayout/utils/float_comparison.h",
                                "src/textlayout/utils/log_util.h",
                                "src/textlayout/utils/tt_point.cc",
                                "src/textlayout/utils/tt_point.h",
                                "src/textlayout/utils/tt_range.cc",
                                "src/textlayout/utils/tt_range.h",
                                "src/textlayout/utils/tt_rangef.cc",
                                "src/textlayout/utils/tt_rangef.h",
                                "src/textlayout/utils/tt_rect.h",
                                "src/textlayout/utils/tt_rectf.cc",
                                "src/textlayout/utils/tt_rectf.h",
                                "src/textlayout/utils/tt_size.h",
                                "src/textlayout/utils/tt_string.cc",
                                "src/textlayout/utils/tt_string.h",
                                "src/textlayout/utils/tt_string_piece.cc",
                                "src/textlayout/utils/tt_string_piece.h",
                                "src/textlayout/utils/u_8_string.cc",
                                "src/textlayout/utils/u_8_string.h",
                                "src/textlayout/utils/value_utils.h"

    sp.pod_target_xcconfig    = {
                                  "GCC_PREPROCESSOR_DEFINITIONS" => "TTTEXT_OS_IOS",
                                  "HEADER_SEARCH_PATHS" => "\"${PODS_TARGET_SRCROOT}/\" \
                                                        \"${PODS_TARGET_SRCROOT}/public/\" \
                                                        \"${PODS_TARGET_SRCROOT}/src/\"",
                                  "CLANG_CXX_LANGUAGE_STANDARD" => "c++17"
                                }

  end
end

