/*************************************************************************
	> File Name: timing_work_mode.c
	> Author:
	> Mail:
	> Created Time: Tue 19 Nov 2019 17:03:36 CST
 ************************************************************************/
#include "cmp_timing.h"
#include "cmp_rtc.h"
#include "ic_ds1302.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "app_driver.h"

static const char *TAG = "app_timing";

typedef struct{
    unsigned char level;
    unsigned char work;
    unsigned char not_work;
    unsigned char count;
    unsigned char is_timing;
}work_s;

#define MAX_WORK_NUM 8
work_s work[MAX_WORK_NUM];
struct ITiming timing[2*MAX_WORK_NUM];

unsigned char timing_start_cb(unsigned char index){
    ESP_LOGI(TAG,"TIMING[%d] CB!",index);
    cmp_rtc_date_s date2;
    cmp_rtc.get_date(&date2);

    ESP_LOGI(TAG,"%04d/%02d/%02d %02d:%02d:%02d [WEEK:%d]",
            date2.year,
            date2.month,
            date2.monthday,
            date2.hour,
            date2.minute,
            date2.second,
            date2.weekday);

    //bee_work_mode_set('A'+work[index/2].level);
    //bee_work_mode_start(index/2);
    work[index/2].is_timing = 1;
    app_driver_set_fragrance_onoff((index/2)<3?'A':'B',1);       
    return 1;
}

unsigned char timing_end_cb(unsigned char index){
    ESP_LOGI(TAG,"TIMING[%d] CB!",index);
    cmp_rtc_date_s date2;
    cmp_rtc.get_date(&date2);

    ESP_LOGI(TAG,"%04d/%02d/%02d %02d:%02d:%02d [WEEK:%d]",
            date2.year,
            date2.month,
            date2.monthday,
            date2.hour,
            date2.minute,
            date2.second,
            date2.weekday);
    //bee_work_mode_set('X');
    //bee_work_mode_stop(index/2);
    app_driver_set_fragrance_onoff((index/2)<3?'A':'B',0);       
    work[index/2].is_timing = 0;
    return 1;
}

static void work_timer_cb(void *timer){
    for(unsigned char i=0;i<MAX_WORK_NUM;i++){
        if(work[i].is_timing == 1){
            work[i].count++;
            if(work[i].count == work[i].work){//change to not work
                //ESP_LOGI(TAG,"WORK[%d] OFF",i); 
                app_driver_set_fragrance_onoff(i<3?'A':'B',0);       
            }else if(work[i].count == work[i].work+work[i].not_work){//change to work
                //ESP_LOGI(TAG,"WORK[%d] ON",i); 
                app_driver_set_fragrance_onoff(i<3?'A':'B',1);       
                work[i].count = 0;
            }
        }
    }
}

///////////////////////////////////////////////////////////
//¶ÔÍâº¯Êý
///////////////////////////////////////////////////////////
void app_timing_init(void){
    cmp_rtc_params_s cmp_rtc_params;
    cmp_rtc.init(&cmp_rtc_params);
    cmp_rtc_date_s date1 = {
        .year =2019,
        .month = 8,
        .monthday = 3,
        .hour = 21,
        .minute = 33,
        .second = 40,
    };
    gc1302_init(&date1);
    cmp_rtc.set_date(date1);
    
    //work mathine
    memset(work,0,sizeof(work));

    TimerHandle_t g_work_timer    = NULL;
    g_work_timer =  xTimerCreate("work timer", 1000, true, NULL, work_timer_cb);//1000ms
    xTimerStart(g_work_timer, 0);                                                                                   

    //timing init
    mITimingFunc.init(timing,2*MAX_WORK_NUM);
}

unsigned char timing_work_mode_set(unsigned char timer_id,unsigned char is_on,unsigned short time_from,unsigned short time_to,unsigned char is_repet,unsigned char day, unsigned char level,unsigned short _work, unsigned short _not_work){
    timer_id--;
    if(timer_id >= 8)return 0;

    mITimingFunc.set(&timing[2*timer_id],1,time_from,is_repet,day,&timing_start_cb);
    mITimingFunc.set(&timing[2*timer_id+1],1,time_to,is_repet,day,&timing_end_cb);
    work[timer_id].level = level+'0';
    work[timer_id].work = _work;
    work[timer_id].not_work = _not_work;

    cmp_rtc_date_s date2;
    cmp_rtc.get_date(&date2);
    ESP_LOGI(TAG,"->%04d/%02d/%02d %02d:%02d:%02d [WEEK:%d]",
            date2.year,
            date2.month,
            date2.monthday,
            date2.hour,
            date2.minute,
            date2.second,
            date2.weekday);
    if(((day>> (8-date2.weekday)) & 0x01) == 0x01){
        unsigned short cur_time = date2.hour*60+ date2.minute;
        if(time_from<cur_time && cur_time<time_to){
            timing_start_cb(2*timer_id);
        }
    }
    return 0;
}
