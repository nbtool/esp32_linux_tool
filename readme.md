
➜  esp32_linux_tool  cd tool 
➜  tool  bash run.sh help
bash run.sh tool
      |- create the build enviroment, including sdk and tool chain
bash run.sh clean
      |- clean all the sdk and tools, thats download form web-page when 'bash run.sh tool'
bash run.sh create path_of_example_in_sdk new_name_project
      |- copy the example in the sdk to app directory, and rename it new_name_project
➜  tool  bash run.sh tool

➜  esp32_linux_tool  tree -L 2
.
├── app
├── readme.md
├── sdk
│   └── esp-idf
└── tool
    ├── run.sh
    └── xtensa-esp32-elf

5 directories, 2 files

➜  tool  bash run.sh create ../sdk/esp-idf/examples/get-started/hello_world hello_world
➜  tool  cd ../app/hello_world

➜  hello_world  ./run.sh help 
bash run.sh config
      |- basic configuration by GUI, if we use -j4 to build and flash, we must first config then build or flash!!!
bash run.sh build
      |- build all
bash run.sh flash
      |- build all and flash the program
bash run.sh build-app
      |- just build app, not build bootloader and partition table
bash run.sh flash-app
      |- just flash app, when bootloader and partition table have not changed, no need to flash
      |- more infomation:https://docs.espressif.com/projects/esp-idf/zh_CN/v3.1.1/get-started/make-project.html
bash run.sh monitor
      |- monitor the program, 'Ctrl+]' to stop
      |- IDF Monitor:https://docs.espressif.com/projects/esp-idf/zh_CN/v3.1.1/get-started/idf-monitor.html


➜  hello_world  ./run.sh flash
➜  hello_world  ./run.sh monitor

