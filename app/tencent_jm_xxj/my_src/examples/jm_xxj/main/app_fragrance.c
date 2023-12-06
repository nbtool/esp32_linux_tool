/*************************************************************************
	> File Name: app_fragrance.c
	> Author: 
	> Mail: 
	> Created Time: Sat 03 Dec 2022 22:03:22 HKT
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

#include "cJSON.h"

#include "app_driver.h"

static const char *TAG = "app_fragrance";

typedef struct{
    bool onoff;
    char device1;
    int start1;
    int stop1;
    char device2;
    int start2;
    int stop2;
}work_model_s;

static work_model_s app_work_model;
static TimerHandle_t g_fragrance_timer    = NULL;
static int count = 0;



static void app_fragrance_timer_cb(void *timer){
    if(app_work_model.onoff == 1){
        if(count == 0){//第一个设备，开始工作
            app_driver_set_fragrance_onoff(app_work_model.device1,1);                      
        }
        if(count == app_work_model.start1){//第一个设备，开始休息
            app_driver_set_fragrance_onoff(app_work_model.device1,0);
        }
        if(count == app_work_model.start1 + app_work_model.stop1){//第二个设备，开始工作
            app_driver_set_fragrance_onoff(app_work_model.device2,1);
        }
        if(count == app_work_model.start1 + app_work_model.stop1 + app_work_model.start2){//第二个设备，开始休息
            app_driver_set_fragrance_onoff(app_work_model.device2,0);
        }
        if(count >= app_work_model.start1 + app_work_model.stop1 + app_work_model.start2 + app_work_model.stop2){//第一个设备，开始工作
            count = -1;
        }

        count++;
    } 
}

esp_err_t app_fragrance_set_onoff(bool on){
    esp_err_t ret     = ESP_OK;

    ESP_LOGW(TAG, "SET ONOFF:%d",on);
    app_work_model.onoff = on;
    
    if(on == 0){
        app_driver_set_fragrance_onoff('A',0);       
        app_driver_set_fragrance_onoff('B',0);       
    }

    return ESP_OK;
}

esp_err_t app_fragrance_set_fan_speed(int speed){
    app_driver_set_fan_speed(speed);
    return ESP_OK;   
}

esp_err_t app_fragrance_set_work_model(cJSON *work_model){
    esp_err_t ret     = ESP_OK;
    
    work_model_s *p_work_model = &app_work_model;
    for (cJSON *item = work_model; item; item = item->next) {
        char *key = item->string;
        ESP_LOGW(TAG,"SET WORK MODEL:%s",key);
        if (!strcmp(key, "fragrance1")) {
            p_work_model->device1 = item->valuestring[0];
        }else if (!strcmp(key, "work_time1")) {
            p_work_model->start1 = item->valueint;
        }else if (!strcmp(key, "stop_time1")) {
            p_work_model->stop1 = item->valueint;
        }else if (!strcmp(key, "fragrance2")) {
            p_work_model->device2 = item->valuestring[0];
        }else if (!strcmp(key, "work_time2")) {
            p_work_model->start2 = item->valueint;
        }else if (!strcmp(key, "stop_time2")) {
            p_work_model->stop2 = item->valueint;
        }
    }

    app_driver_set_fragrance_onoff('A',0);                      
    app_driver_set_fragrance_onoff('B',0);                      
    count = 0;

    ESP_LOGW(TAG, "SET WORK MODEL:[ONOFF:%d]-[%c %ds %ds]-[%c %ds %ds]",
            p_work_model->onoff,
            p_work_model->device1,
            p_work_model->start1,
            p_work_model->stop1,
            p_work_model->device2,
            p_work_model->start2,
            p_work_model->stop2);
        
    return ESP_OK;
}


esp_err_t app_fragrance_init(void){
    g_fragrance_timer = xTimerCreate("app fragrance timer", 1000, true, NULL, app_fragrance_timer_cb);//1S
    xTimerStart(g_fragrance_timer, 0);

    return ESP_OK;
}
