#!/bin/bash
# Copyright 2021 The Lynx Authors. All rights reserved.

echo $0 [install] [kvm]
script_dir="$(cd "$(dirname "$0")"; pwd -P)"
source ${script_dir}/android_env.sh

option=$1
# Install and Create Emulators using avdmanager and sdkmanager
if [ x"${option}" = x"install" ];then
   sdkmanager --install "system-images;android-28;google_apis;x86"
   avdmanager --verbose create avd --force --name "Nexus_5_API_28" \
      --package "system-images;android-28;google_apis;x86" \
      --force -d "Nexus 5"
fi

# android emulator must enable hardware acceleration
# which need to setup the qemu-kvm and adduser
# DO NOT use sudo for root account
xsudo=
if [ ! -z "${USER}" -a x"${USER}" != x"root" ];then
   xsudo=sudo
fi
$xsudo apt update -y
$xsudo apt install qemu-kvm -y
if [ ! -z "${USER}" -a x"${USER}" != x"root" ];then
   $xsudo adduser ${USER} kvm
   $xsudo chown -R ${USER} /dev/kvm
   grep kvm /etc/group
fi

ls -la /dev/kvm
