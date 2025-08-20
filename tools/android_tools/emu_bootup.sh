#!/bin/bash
# Copyright 2021 The Lynx Authors. All rights reserved.

set -e
script_dir="$(cd "$(dirname "$0")"; pwd -P)"
source ${script_dir}/android_env.sh

avd_name=$1
if [ x"${1}" == x"--verbose" ];then
  set -x
  avd_name=""
fi
devices_list=($(adb devices | grep "device$" | awk '{print $1}'))
devices_nums=${#devices_list[@]}
if [ ${devices_nums} -gt 0 ];then
  # re-use the existed devices
  adb devices
  exit 0
fi

# if no active emulator exits, bootup the first avd in list
if [ -z "${avd_name}" ];then
  avd_list=($(emulator -list-avds))
  avd_nums=${#avd_list[@]}
  if [ ${avd_nums} -lt 1 ];then
    echo "can not find emulator to bootup"
    exit 1
  fi
  avd_name=${avd_list[0]}
fi

nohup emulator -no-window -writable-system -avd ${avd_name} &
if [ $? -ne 0 ];then
  echo "start emulator failed, please check nohup.out"
fi

# make sure server available
adb start-server

# wait devices to bootup
retry=20
devices_nums=0
while [ $retry -gt 0 -a ${devices_nums} -lt 1 ]
do
  echo "wait device to bootup $retry"
  retry=$(($retry-1))
  sleep 5
  devices_list=($(adb devices | grep "device$" | awk '{print $1}'))
  devices_nums=${#devices_list[@]}
done

if [ ${devices_nums} -lt 1 ];then
  echo "device bootup failed!"
  exit -1
fi
# wait devices to boot_completed
retry=20
booted=0
while [ $retry -gt 0 -a $booted -eq 0 ]
do
  echo "wait device to boot_completed $retry"
  retry=$(($retry-1))
  sleep 5
  booted=$(adb shell getprop sys.boot_completed)
  if [ -z "${booted}" ];then
    booted=0
  fi
done

adb devices
