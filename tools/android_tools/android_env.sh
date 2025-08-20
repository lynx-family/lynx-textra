#!/bin/bash
# Copyright 2021 The Lynx Authors. All rights reserved.

if [ x"$1" = x"install" ];then
  config_file=~/.bashrc
  sed -i "/android-sdk config begin/,/android-sdk config end/ d" ${config_file}
  echo '#-------------------  android-sdk config begin ----------------#' >> ${config_file}
  echo 'export ANDROID_SDK_ROOT=${ANDROID_HOME}' >> ${config_file}
  echo 'export PATH=${ANDROID_SDK_ROOT}/platform-tools:$PATH' >> ${config_file}
  echo 'export PATH=${ANDROID_SDK_ROOT}/emulator:$PATH' >> ${config_file}
  echo 'export PATH=${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin:$PATH' >> ${config_file}
  echo 'export PATH=${ANDROID_SDK_ROOT}/tools/bin:$PATH' >> ${config_file}
  echo '# export ANDROID_AVD_HOME=~/.android/avd' >> ${config_file}
  echo '#-------------------  android-sdk config end ------------------#' >> ${config_file}
fi

# please set the ANDROID_HOME before hands
export ANDROID_SDK_ROOT=${ANDROID_HOME}
export PATH=${ANDROID_SDK_ROOT}/platform-tools:$PATH
export PATH=${ANDROID_SDK_ROOT}/emulator:$PATH
export PATH=${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin:$PATH
export PATH=${ANDROID_SDK_ROOT}/tools/bin:$PATH
# export ANDROID_AVD_HOME=~/.android/avd
