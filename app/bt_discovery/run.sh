#!/bin/bash

#I don't like to set environment variables in the system, 
#so I put the environment variables in run.sh.
#Every time I use run.sh, the enviroment variables will be set, after use that will be unsetted.


PROJECT_ROOT=../..
TOOLS_PATH=$PROJECT_ROOT/tool
SDK_PATH=$PROJECT_ROOT/sdk
APP_PATH=$PROJECT_ROOT/app

XTENSA_ESP32_ELF_PATH=$TOOLS_PATH/xtensa-esp32-elf
ESP_IDF_PATH=$SDK_PATH/esp-idf

the_sdk_path=`cd $ESP_IDF_PATH; pwd`
the_tool_chain_path=`cd $XTENSA_ESP32_ELF_PATH/bin; pwd`

export PATH="$PATH:$the_tool_chain_path"
export IDF_PATH="$the_sdk_path"


if [ "$1" == "config" ]; then
    make menuconfig
elif [ "$1" == "build" ]; then
    make all
elif [ "$1" == "flash" ]; then
    make flash   
elif [ "$1" == "build-app" ]; then
    make app   
elif [ "$1" == "flash-app" ]; then
    make app-flash
elif [ "$1" == "monitor" ]; then
    make monitor   
elif [ "$1" == "clean" ]; then
    make clean
elif [ "$1" == "help" ]; then
    echo "bash run.sh config"
    echo "      |- basic configuration by GUI, if we use -j4 to build and flash, we must first config then build or flash!!!"
    echo "bash run.sh build"
    echo "      |- build all"
    echo "bash run.sh flash"
    echo "      |- build all and flash the program"
    echo "bash run.sh build-app"
    echo "      |- just build app, not build bootloader and partition table"
    echo "bash run.sh flash-app"
    echo "      |- just flash app, when bootloader and partition table have not changed, no need to flash"
    echo "      |- more infomation:https://docs.espressif.com/projects/esp-idf/zh_CN/v3.1.1/get-started/make-project.html"
    echo "bash run.sh monitor"
    echo "      |- monitor the program, 'Ctrl+]' to stop"
    echo "      |- IDF Monitor:https://docs.espressif.com/projects/esp-idf/zh_CN/v3.1.1/get-started/idf-monitor.html"
else
    echo "error, try bash run.sh help"
fi

