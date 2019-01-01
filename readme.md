#### 1. Brief introduction

The project provides a esp32 APP command-line development environment based on Linux system.

You can directly create、write、build、install APP(HEX/BIN) without using IDE.


#### 2. How to use

If it is the first time to compile, It is recommended to run goto tool directly, and then run the 'bash run.sh tool' to download gcc、SDK、build-tools etc.

	cd tool
    bash run.sh help
	bash run.sh tool

Create the project from the sdk examples(for example:hello_world):

    bash run.sh create ../sdk/esp-idf/examples/get-started/hello_world hello_world
    cd ../app/hello_world
    ./run.sh help

Install the APP:

    ./run.sh flash

Monitor and watch log:

    ./run.sh monitor

Clean:

    ./run.sh clean


**more：**[xxxxxxxxxx](xxxxxxxxxxxx)


#### 3. Demo-Apps

- bt_discovery : 10s-loop to discovery the ble device
