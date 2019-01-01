#!/bin/bash

set -e

PROJECT_ROOT=..
TOOLS_PATH=$PROJECT_ROOT/tool
SDK_PATH=$PROJECT_ROOT/sdk
APP_PATH=$PROJECT_ROOT/app

XTENSA_ESP32_ELF_PATH=$TOOLS_PATH/xtensa-esp32-elf
ESP_IDF_PATH=$SDK_PATH/esp-idf

XTENSA_ESP32_ELF_LINK=https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
ESP_IDF_LINK=https://github.com/espressif/esp-idf.git   

#--------------------------------------------------------------------------
function install_tool_chain(){
    echo "> install tool chain ..."
    echo "> web page: https://docs.espressif.com/projects/esp-idf/zh_CN/v3.1.1/get-started/linux-setup.html"
    if [ ! -d $XTENSA_ESP32_ELF_PATH ]; then
        wget $XTENSA_ESP32_ELF_LINK 
        tar -xzf xtensa-esp32-elf*.tar.gz
        rm xtensa-esp32-elf*.tar.gz
    fi
}

function install_esp_idf(){
    echo "> install esp idf ..."
    echo "> web page: https://github.com/espressif/esp-idf"
    if [ ! -d $ESP_IDF_PATH ]; then
        git clone $ESP_IDF_LINK 
        mv esp-idf $SDK_PATH/
    fi
}

function create_project(){
    if [ "$1" == "" ] || [ "$2" == "" ]; then
        echo "input error"
    elif [ -d $1 ] && [ ! -d "$APP_PATH/$2" ]; then
        cp -r $1 $APP_PATH/$2
       

        file=$APP_PATH/$2/run.sh
        the_sdk_path=`cd $ESP_IDF_PATH; pwd`
        the_tool_chain_path=`cd $XTENSA_ESP32_ELF_PATH/bin; pwd`

cat > $file <<EOF
#!/bin/bash

#I don't like to set environment variables in the system, 
#so I put the environment variables in run.sh.
#Every time I use run.sh, the enviroment variables will be set, after use that will be unsetted.


PROJECT_ROOT=../..
TOOLS_PATH=\$PROJECT_ROOT/tool
SDK_PATH=\$PROJECT_ROOT/sdk
APP_PATH=\$PROJECT_ROOT/app

XTENSA_ESP32_ELF_PATH=\$TOOLS_PATH/xtensa-esp32-elf
ESP_IDF_PATH=\$SDK_PATH/esp-idf

the_sdk_path=\`cd \$ESP_IDF_PATH; pwd\`
the_tool_chain_path=\`cd \$XTENSA_ESP32_ELF_PATH/bin; pwd\`

export PATH="\$PATH:\$the_tool_chain_path"
export IDF_PATH="\$the_sdk_path"


if [ "\$1" == "config" ]; then
    make menuconfig
elif [ "\$1" == "build" ]; then
    make all
elif [ "\$1" == "flash" ]; then
    make flash   
elif [ "\$1" == "build-app" ]; then
    make app   
elif [ "\$1" == "flash-app" ]; then
    make app-flash
elif [ "\$1" == "monitor" ]; then
    make monitor   
elif [ "\$1" == "clean" ]; then
    make clean
elif [ "\$1" == "help" ]; then
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

EOF

        chmod +x $file
        ls -all $APP_PATH/$2
    fi
}
#--------------------------------------------------------------------------

function tool(){
    if [ ! -d $SDK_PATH ]; then
        mkdir $SDK_PATH
    fi
    if [ ! -d $APP_PATH ]; then
        mkdir $APP_PATH
    fi

    install_tool_chain
    install_esp_idf
}

function clean(){
    echo "cleaning ...."
    rm -rf $XTENSA_ESP32_ELF_PATH
    rm -rf $ESP_IDF_PATH
    rm -rf $SDK_PATH
}

if [ "$1" == "clean" ]; then
    clean
elif [ "$1" == "tool" ]; then
    tool
elif [ "$1" == "create" ]; then
    create_project $2 $3
elif [ "$1" == "help" ]; then
    echo "bash run.sh tool"
    echo "      |- create the build enviroment, including sdk and tool chain"
    echo "bash run.sh clean"
    echo "      |- clean all the sdk and tools, thats download form web-page when 'bash run.sh tool'"
    echo "bash run.sh create path_of_example_in_sdk new_name_project"
    echo "      |- copy the example in the sdk to app directory, and rename it new_name_project"
else
    echo "error, try bash run.sh help"
fi
