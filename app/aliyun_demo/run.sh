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

GIT_IOTKIT_EMBEDDED_LINK=https://github.com/oldprogram/iotkit-embedded.git
GIT_ESP_ALIYUN_LINK=https://github.com/oldprogram/esp-aliyun.git
#GIT_ESP_ALIYUN_LINK=https://github.com/espressif/esp-aliyun.git
#GIT_IOTKIT_EMBEDDED_LINK=https://github.com/aliyun/iotkit-embedded.git

#--------------------------------------------------------------------------
function install_project_from_github(){
    echo "> install project form github ..."
    if [ ! -d "./esp-aliyun/" ]; then
        git clone $GIT_ESP_ALIYUN_LINK
        
        # remove the iotkit-git link
        # make a link iotkit-embedded->../iotkit-embedded to ./esp-aliyun/iotkit-embedded
        rm -rf ./esp-aliyun/iotkit-embedded/
        ln -s ../iotkit-embedded ./esp-aliyun/iotkit-embedded

        # remove the mqtt_example.c
        # move cp mqtt_example.c replace pre-one
        rm -rf ./esp-aliyun/examples/mqtt_example/main/mqtt_example.c
        cp ./my_src/mqtt_example.c ./esp-aliyun/examples/mqtt_example/main/
    fi

    if [ ! -d "./iotkit-embedded/" ]; then
        git clone $GIT_IOTKIT_EMBEDDED_LINK

        # cp config.esp32.aos to ./iotkit-embedded/src/board/
        cp ./my_src/config.esp32.aos ./iotkit-embedded/src/board/
    fi

    cd $ESP_IDF_PATH
    git checkout release/v3.3
    python --version
    python -m pip install -r $ESP_IDF_PATH/requirements.txt
    git submodule update
    cd -
}

function sdk_op(){
    echo ">> APP_OP   "$1

    cd ./iotkit-embedded/
    if [ "$1" == "reconfig" ]; then
        make reconfig
    elif [ "$1" == "config" ]; then
        make menuconfig
    elif [ "$1" == "make" ]; then
        make 
    elif [ "$1" == "clean" ]; then
        make distclean
    else
        echo "error, try bash run.sh help"   
    fi
    cd -
}

function app_op(){
    echo ">> SDK_OP   "$1

    cd  ./esp-aliyun/examples/mqtt_example/
    if [ "$1" == "defconfig" ]; then
        make defconfig
    elif [ "$1" == "config" ]; then
        make menuconfig
    elif [ "$1" == "make" ]; then
        make
    elif [ "$1" == "erase" ]; then
        make erase
    elif [ "$1" == "flash" ]; then
        make flash
    elif [ "$1" == "monitor" ]; then
        make monitor
    elif [ "$1" == "clean" ]; then
        make clean
    else
        echo "error, try bash run.sh help"
    fi
    cd -
}

if [ "$1" == "create" ]; then
    install_project_from_github
elif [ "$1" == "sdk" ]; then
    sdk_op $2
elif [ "$1" == "app" ];then
    app_op $2
elif [ "$1" == "help" ];then
    echo "|----------------------------------------------------"
    echo "| ./run.sh op param"
    echo "| op:"
    echo "|   create : downloads iotkit and aliyun-esp32 from github and make some change"
    echo "|   sdk : param = reconfig/config/make/clean"
    echo "|   app : param = deconfig/config/make/erase/flash/monitor/clean"
    echo "| examples:"
    echo "|   first create sdk lib : create -> sdk reconfig -> sdk config -> sdk make"
    echo "|   second create app : config -> make -> flash -> monitor"
    echo "|----------------------------------------------------"
else
    echo "error, try bash run.sh help"   
fi

