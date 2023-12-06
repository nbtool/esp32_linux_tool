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

GIT_ESP_QCLOUD_LINK=https://github.com/oldprogram/esp-qcloud.git

#--------------------------------------------------------------------------
function install_project_from_github(){
    echo "> install project form github ..."
    if [ ! -d "./esp-qcloud/" ]; then
        git clone $GIT_ESP_QCLOUD_LINK
        cd esp-qcloud
        git checkout 43e218d
        cd -
        cp -r my_src/* ./esp-qcloud/ 
    fi

    cd $ESP_IDF_PATH
    if git rev-parse --verify release/v4.3;then
        echo "> in right brach"
    else
        echo "> change to right brach"
        git checkout release/v4.3
    fi

    python --version
    bash install.sh
    #. ./export.sh
    echo ''
    echo '> xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    echo '> you should run source '$TOOLS_PATH'/pvevn/bin/active in every new terminal'
    echo '> you should run . '$ESP_IDF_PATH'/export.sh in every new terminal'
    echo '> xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    cd -
}

function app_op(){
    #. $ESP_IDF_PATH/export.sh
    echo ">> SDK_OP   "$1

    cd  ./esp-qcloud/examples/jm_xxj/
    if [ "$1" == "make" ]; then
        idf.py build
    elif [ "$1" == "flash" ]; then
        idf.py flash
    elif [ "$1" == "monitor" ]; then
        idf.py monitor
    elif [ "$1" == "clean" ]; then
        idf.py fullclean
    else
        echo "error, try bash run.sh help"
    fi
    cd -
}

if [ "$1" == "create" ]; then
    install_project_from_github
elif [ "$1" == "app" ];then
    app_op $2
elif [ "$1" == "help" ];then
    echo "|----------------------------------------------------"
    echo "| ./run.sh op param"
    echo "| op:"
    echo "|   create : downloads qcloud and cp my_src to qcloud"
    echo "|   clean  : make clean"
    echo "| examples:"
    echo "|   first create project: create"
    echo "|   second : app make -> app flash -> app monitor"
    echo "|----------------------------------------------------"
    echo ''
    echo '> xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    echo '> you should run source '$TOOLS_PATH'/pvevn/bin/active in every new terminal'
    echo '> you should run . '$ESP_IDF_PATH'/export.sh in every new terminal'
    echo '> xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
else
    echo "error, try bash run.sh help"   
fi

