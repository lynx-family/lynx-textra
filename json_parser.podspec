# Copyright 2019 The Lynx Authors. All rights reserved.
# coding: utf-8

Pod::Spec.new do |s|
  s.name = "json_parser"
  s.version = "1.0.0"
  s.summary = "Parser JSON files into LynxTextra C++ objects"
  s.description = <<-DESC
  A JSON parser that converts JSON files into LynxTextra C++ objects for text layout.
  This enables easy creation of examples and test cases using JSON files, without writing C++ code or recompiling.
  DESC
  s.homepage = "https://github.com/lynx-family/lynx-textra"
  s.license = { :type => "Apache-2.0", :file => "LICENSE" }
  s.author = { :name => "Lynx" }
  s.source = { :git => "https://github.com/lynx-family/lynx-textra.git", :tag => s.version.to_s }
  s.ios.deployment_target = "12.0"
  s.requires_arc = true


  s.subspec "json_parser" do |sp|
    sp.public_header_files    = "examples/json_parser/json_parser.h"

    sp.private_header_files   = "third_party/rapidjson/include/rapidjson/*.{h,hpp}",
                                "third_party/rapidjson/include/rapidjson/error/*.{h,hpp}",
                                "third_party/rapidjson/include/rapidjson/internal/*.{h,hpp}",
                                "third_party/rapidjson/include/rapidjson/msinttypes/*.{h,hpp}"

    sp.source_files           = "examples/json_parser/json_parser.cc",
                                "third_party/rapidjson/include/rapidjson/allocators.h",
                                "third_party/rapidjson/include/rapidjson/cursorstreamwrapper.h",
                                "third_party/rapidjson/include/rapidjson/document.h",
                                "third_party/rapidjson/include/rapidjson/encodedstream.h",
                                "third_party/rapidjson/include/rapidjson/encodings.h",
                                "third_party/rapidjson/include/rapidjson/error/en.h",
                                "third_party/rapidjson/include/rapidjson/error/error.h",
                                "third_party/rapidjson/include/rapidjson/filereadstream.h",
                                "third_party/rapidjson/include/rapidjson/filewritestream.h",
                                "third_party/rapidjson/include/rapidjson/fwd.h",
                                "third_party/rapidjson/include/rapidjson/internal/biginteger.h",
                                "third_party/rapidjson/include/rapidjson/internal/diyfp.h",
                                "third_party/rapidjson/include/rapidjson/internal/dtoa.h",
                                "third_party/rapidjson/include/rapidjson/internal/ieee754.h",
                                "third_party/rapidjson/include/rapidjson/internal/itoa.h",
                                "third_party/rapidjson/include/rapidjson/internal/meta.h",
                                "third_party/rapidjson/include/rapidjson/internal/pow10.h",
                                "third_party/rapidjson/include/rapidjson/internal/regex.h",
                                "third_party/rapidjson/include/rapidjson/internal/stack.h",
                                "third_party/rapidjson/include/rapidjson/internal/strfunc.h",
                                "third_party/rapidjson/include/rapidjson/internal/strtod.h",
                                "third_party/rapidjson/include/rapidjson/internal/swap.h",
                                "third_party/rapidjson/include/rapidjson/istreamwrapper.h",
                                "third_party/rapidjson/include/rapidjson/memorybuffer.h",
                                "third_party/rapidjson/include/rapidjson/memorystream.h",
                                "third_party/rapidjson/include/rapidjson/msinttypes/inttypes.h",
                                "third_party/rapidjson/include/rapidjson/msinttypes/stdint.h",
                                "third_party/rapidjson/include/rapidjson/ostreamwrapper.h",
                                "third_party/rapidjson/include/rapidjson/pointer.h",
                                "third_party/rapidjson/include/rapidjson/prettywriter.h",
                                "third_party/rapidjson/include/rapidjson/rapidjson.h",
                                "third_party/rapidjson/include/rapidjson/reader.h",
                                "third_party/rapidjson/include/rapidjson/schema.h",
                                "third_party/rapidjson/include/rapidjson/stream.h",
                                "third_party/rapidjson/include/rapidjson/stringbuffer.h",
                                "third_party/rapidjson/include/rapidjson/writer.h"

    sp.exclude_files          = "third_party/rapidjson/include/rapidjson/msinttypes/*"

    sp.pod_target_xcconfig    = {
                                  "GCC_PREPROCESSOR_DEFINITIONS" => "RAPIDJSON_HAS_STDSTRING \
                                                                 RAPIDJSON_HAS_CXX11_RVALUE_REFS \
                                                                 RAPIDJSON_HAS_CXX11_NOEXCEPT \
                                                                 RAPIDJSON_HAS_CXX11_TYPETRAITS \
                                                                 RAPIDJSON_HAS_CXX11_RANGE_FOR",
                                  "HEADER_SEARCH_PATHS" => "\"${PODS_TARGET_SRCROOT}/\" \
                                                        \"${PODS_TARGET_SRCROOT}/examples/json_parser/\" \
                                                        \"${PODS_TARGET_SRCROOT}/public/\" \
                                                        \"${PODS_TARGET_SRCROOT}/src/\" \
                                                        \"${PODS_TARGET_SRCROOT}/third_party/rapidjson/\" \
                                                        \"${PODS_TARGET_SRCROOT}/third_party/rapidjson/include/\"",
                                  "CLANG_CXX_LANGUAGE_STANDARD" => "c++17"
                                }

  end
end

