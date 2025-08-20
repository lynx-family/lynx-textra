# Copyright 2019 The Lynx Authors. All rights reserved.
# coding: utf-8

$variable_a = ENV["VALUE_A"]
$variable_b = ENV["VALUE_B"]

Pod::Spec.new do |s|
  s.name                      = 'gn_testing'
  s.version                   = '0.1.0'
  s.summary                   = 'A short description of gn_testing.'
  s.description               = <<-DESC
  TODO: Add long description of the pod here.
  DESC
  s.homepage                  = 'https://github.com/lynx-family/lynx'
  s.license                   = 'Apache-2.0'
  s.author                    = 'Lynx'
  s.source                    = { :git => 'https://github.com/lynx-family/lynx.git', :tag => s.version.to_s }
  s.ios.deployment_target     = '9.0'
  s.compiler_flags            = "-fno-rtti"


  s.subspec "GNTesting" do |sp|
    sp.compiler_flags         = "-Wall", "-Wextra", "-Werror" 
    sp.header_mappings_dir    = "gn_testing"
    sp.pod_target_xcconfig    = {
                                  "HEADER_SEARCH_PATHS" => "\"${PODS_TARGET_SRCROOT}/\""
                                }

    sp.dependency               "Lynx/Framework"

    sp.subspec "GNTestingSub" do |ssp|
      ssp.compiler_flags        = "-Wall", "-Wextra" 
      ssp.framework             = "Foundation" 
      ssp.libraries             = "gcc" 
      ssp.public_header_files   = "tools/gn_tools/testing/gn_testing.h"

      ssp.source_files          = "tools/gn_tools/testing/gn_testing.cc",
                                  "tools/gn_tools/testing/gn_testing.h"

      ssp.pod_target_xcconfig    = {
                                    "GCC_PREPROCESSOR_DEFINITIONS" => "GN_TESTING_DEFINE=1",
                                    "HEADER_SEARCH_PATHS" => "\"${PODS_TARGET_SRCROOT}/tools/gn_tools/testing/\""
                                  }

      ssp.dependency              "Lynx/Native"
    end
  end
end

