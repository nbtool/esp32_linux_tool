/*************************************************************************
	> File Name: app_driver.c
	> Author: 
	> Mail: 
	> Created Time: Sun 04 Dec 2022 16:09:33 HKT
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "esp_qcloud_utils.h"
#include "esp_qcloud_storage.h"

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "ic_tm1650.h"

static const char *TAG = "app_driver";



#define GPIO_OUTPUT_LED_BLUE    18
#define GPIO_OUTPUT_LED_RED     19
#define GPIO_OUTPUT_LED_GREEN   21
#define GPIO_OUTPUT_MOTO_CTL    4
#define GPIO_OUTPUT_FAN_CTL     5
#define GPIO_OUTPUT_VALVE1_CTL  16
#define GPIO_OUTPUT_VALVE2_CTL  17
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_MOTO_CTL) | (1ULL<<GPIO_OUTPUT_VALVE1_CTL) | (1ULL<<GPIO_OUTPUT_VALVE2_CTL) | (1ULL<<GPIO_OUTPUT_LED_BLUE) | (1ULL<<GPIO_OUTPUT_LED_RED) | (1ULL<<GPIO_OUTPUT_LED_GREEN))

#define GPIO_INPUT_HELL1_BIT0    33
#define GPIO_INPUT_HELL1_BIT1    26
#define GPIO_INPUT_HELL1_BIT2    25
#define GPIO_INPUT_HELL2_BIT0    13
#define GPIO_INPUT_HELL2_BIT1    14
#define GPIO_INPUT_HELL2_BIT2    27
#define GPIO_INPUT_BUTTON1       34
#define GPIO_INPUT_BUTTON2       39
#define GPIO_INPUT_BUTTON3       32
#define GPIO_INPUT_BUTTON4       35
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_HELL1_BIT0) | (1ULL<<GPIO_INPUT_HELL1_BIT1) | (1ULL<<GPIO_INPUT_HELL1_BIT2) | (1ULL<<GPIO_INPUT_HELL2_BIT0) | (1ULL<<GPIO_INPUT_HELL2_BIT1) | (1ULL<<GPIO_INPUT_HELL2_BIT2) | (1ULL<<GPIO_INPUT_BUTTON1) | (1ULL<<GPIO_INPUT_BUTTON2) | (1ULL<<GPIO_INPUT_BUTTON3) | (1ULL<<GPIO_INPUT_BUTTON4))

static char wifi_state = 0;//0:not connect; 1:connect; 2:pairing
static char fragrance_state[2] = {0,0};//0:close;1:open

const unsigned char TUBE_TABLE_0[16] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};    //共阴，0~9~A~F
                                                                                                                             
int hell1 = 0;
int hell2 = 0;
// PWM -------------------------------------------------------------
// https://cloud.tencent.com/developer/article/1851818
/*
 * Prepare and set configuration of timers
 * that will be used by LED Controller
 */
ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    .freq_hz = 1,                      // frequency of PWM signal
    .speed_mode = LEDC_HIGH_SPEED_MODE,   // timer mode
    .timer_num = LEDC_TIMER_0,            // timer index
    .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
};

ledc_channel_config_t ledc_channel[1] = {
    {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 2000,//max 2^13 = 8000
        .gpio_num   = GPIO_OUTPUT_FAN_CTL,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0
    }
};
//--------------------------------------------------------------------

static void app_driver_wifi_state_run(void){
    static char state = 0xFF;
    static char onoff = 0;
    if(state != wifi_state){
        state = wifi_state;
        if(state == 1){//connect
            onoff = 1;
        }else if(state == 0){//not connect
            onoff = 0;
        }
    }

    if(state == 2){//pairing
        onoff = (onoff == 0?1:0);
    }
    
    //ESP_LOGW(TAG,"XXX:%d %d",onoff,state);
    gpio_set_level(GPIO_OUTPUT_LED_GREEN, onoff);
}

static void app_driver_fragrance_run(void){
    static char state[2] = {0xFF,0xFF};
    char on = 0;
    char ok = 0;
    if(state[0] != fragrance_state[0]){
        state[0] =fragrance_state[0];
        gpio_set_level(GPIO_OUTPUT_VALVE1_CTL, state[0]);                  
        gpio_set_level(GPIO_OUTPUT_LED_RED, state[0]);           
        if(state[0] == 1)
            on = 1;
        ok = 1;
    }
    if(state[1] != fragrance_state[1]){
        state[1] = fragrance_state[1];
        gpio_set_level(GPIO_OUTPUT_VALVE2_CTL, state[1]);                  
        gpio_set_level(GPIO_OUTPUT_LED_BLUE, state[1]);           
        if(state[1] == 1)
            on = 1;
        ok = 1;
    }

    if(ok){
        vTaskDelay(20 / portTICK_RATE_MS);
        gpio_set_level(GPIO_OUTPUT_MOTO_CTL, on);                  
        gpio_set_level(GPIO_OUTPUT_FAN_CTL, on);                  
    }
}

static void app_driver_hell_run(void){

    int btn = 0;
    int _hell1 = 0,_hell2 = 0;
    _hell1 |= gpio_get_level(GPIO_INPUT_HELL1_BIT0) == 0?0:1;
    _hell1 |= gpio_get_level(GPIO_INPUT_HELL1_BIT1) == 0?0:2;
    _hell1 |= gpio_get_level(GPIO_INPUT_HELL1_BIT2) == 0?0:4;
    _hell2 |= gpio_get_level(GPIO_INPUT_HELL2_BIT0) == 0?0:1;
    _hell2 |= gpio_get_level(GPIO_INPUT_HELL2_BIT1) == 0?0:2;
    _hell2 |= gpio_get_level(GPIO_INPUT_HELL2_BIT2) == 0?0:4;
    btn |= gpio_get_level(GPIO_INPUT_BUTTON1) == 0?0:1;
    btn |= gpio_get_level(GPIO_INPUT_BUTTON2) == 0?0:2;
    btn |= gpio_get_level(GPIO_INPUT_BUTTON3) == 0?0:4;
    btn |= gpio_get_level(GPIO_INPUT_BUTTON4) == 0?0:8;
    
    //https://zhuanlan.zhihu.com/p/158634045
    //I7 -> HELL1
    static uint8_t level[8] = {0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x00};
    _hell1 = level[_hell1];
    _hell2 = level[_hell2];

    // if have changed report by blue
    extern void bt_server_cmd(uint8_t *datas, uint8_t len);
    if(_hell1 != hell1 || _hell2 != hell2){
        hell1 = _hell1;
        hell2 = _hell2;

        uint8_t send[14];
        memset(send,0,14);
        send[0] = 0x02;
        bt_server_cmd(send,14);
    }
    //ESP_LOGW(TAG,"hell sensor: %x %x", hell1, hell2);
    TM1650_print(0,TUBE_TABLE_0[hell1]);
    TM1650_print(1,TUBE_TABLE_0[hell2]);
    TM1650_print(2,TUBE_TABLE_0[btn]);
    /*
    static int num = 0;
    static unsigned char dig = 0;
    TM1650_print(dig,TUBE_TABLE_0[num]);
    ESP_LOGW(TAG,"->%d %d",dig,num);
    num++;
    if(num > 15){
        num = 0;
        dig++;
        if(dig > 3)
            dig = 0;
    }
    */
}

static void app_driver_task(void* arg){
    while(true) {
        app_driver_wifi_state_run();
        app_driver_fragrance_run();
        app_driver_hell_run();
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void app_driver_set_wifi_state(char state){
    wifi_state = state;
    ESP_LOGW(TAG,"set wifi state: %d", state);
}

void app_driver_set_fragrance_onoff(char device, bool on){
    ESP_LOGW(TAG,"DEVICE[%c]:%d",device,on);

    if(device == 'A'){
        fragrance_state[0] = on;
    }else if(device == 'B'){
        fragrance_state[1] = on;
    }
}

void app_driver_set_fan_speed(int speed){
    //ledc_set_freq(LEDC_HIGH_SPEED_MODE,LEDC_TIMER_0,(speed%10+1));
    //0~100
    //pwm 0~2^13 (//0~100
    ledc_channel[0].duty = (speed<<13)/100;
    ledc_set_fade_with_time(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, ledc_channel[0].duty, 0);
    ledc_fade_start(ledc_channel[0].speed_mode,
            ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
    ESP_LOGW(TAG,"FAN SPEED:%d DUTY:%d",speed,ledc_channel[0].duty);
}

esp_err_t app_driver_init(void){
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    //io_conf.pull_up_en = 1;
    gpio_config(&io_conf);


    gpio_set_level(GPIO_OUTPUT_LED_BLUE, 0);
    gpio_set_level(GPIO_OUTPUT_LED_RED, 0);
    gpio_set_level(GPIO_OUTPUT_LED_GREEN, 0);
    gpio_set_level(GPIO_OUTPUT_MOTO_CTL, 0);
    gpio_set_level(GPIO_OUTPUT_VALVE1_CTL, 0);
    gpio_set_level(GPIO_OUTPUT_VALVE2_CTL, 0);

    //PWM
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    // Set LED Controller with previously prepared configuration 
    ledc_channel_config(&ledc_channel[0]);
    // Initialize fade service.
    ledc_fade_func_install(0);

    app_driver_set_fan_speed(100);


    //4-LCD
    TM1650_init();

    xTaskCreate(app_driver_task, "app_driver_task", 2048, NULL, 10, NULL);

    return ESP_OK;
}

