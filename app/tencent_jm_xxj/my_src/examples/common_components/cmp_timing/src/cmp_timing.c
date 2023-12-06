/*************************************************************************
	> File Name: cmp_timing.c
	> Author: 
	> Mail: 
	> Created Time: Mon 27 Feb 2023 00:31:19 HKT
 ************************************************************************/
#include "string.h"
#include "cmp_rtc.h"
#include "cmp_timing.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"



void ty_timing_init(struct ITiming *pITiming, unsigned char num);
void ty_timing_set(struct ITiming *pITiming,unsigned char enable,unsigned short time,unsigned char repet,unsigned char day,unsigned char (*cb)(unsigned char index));
void ty_timing_kill(struct ITiming *pITiming);


static const char *TAG = "cmp_timing";

///////////////////////////////////////////////////////////////////////
struct ITimingFunc mITimingFunc = {
    .init = &ty_timing_init,
    .set = &ty_timing_set,
    .kill = &ty_timing_kill,
};

static unsigned char timing_num = 0;
struct ITiming *pITimings = NULL;
///////////////////////////////////////////////////////////////////////


static void cmp_timing_timer_cb(void *timer){
    static unsigned int cyc_send_time=0;
    static unsigned char pre_minute = 61;//防止同一分钟执行多次动作

    //1.获取当前星期和Hour:Minute
    cmp_rtc_date_s date;
    cmp_rtc.get_date(&date);

    if(pre_minute == date.minute || date.year == 0)return;

    unsigned short time = (unsigned short)date.hour*60+(unsigned short)date.minute;
    pre_minute = date.minute;
    
    for(unsigned char i=0;i<timing_num;i++){
        struct ITiming *p = &pITimings[i];
        if(p->enable == 1){
//                ESP_LOGI(TAG, "[T_N] %x \r\n", time);
//                ESP_LOGI(TAG, "[T_T] %x \r\n", p->time);
            //2.比较是否到达定时
            if(time == p->time){
                //ESP_LOGI(TAG, "[T=T]\r\n");
                //3.分别处理重复定时和非重复定时
                if(p->repet == 1){
                    //4.判断week是否匹配
                    if((((p->day)>>(8-date.weekday))&0x01) == 0x01){
                        //5.如果匹配执行动作
                        if(p->timing_end_cb != NULL)
                            p->timing_end_cb(i);
                    }
                }else{
                    //4.判断week是否匹配
                    if((((p->day)>>(7-date.weekday))&0x01) == 0x01){
                        //5.如果匹配执行动作
                        if(p->timing_end_cb != NULL)
                            p->timing_end_cb(i);
                        //6.失能
                        p->enable = 0;
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////
//对外函数
///////////////////////////////////////////////////////////
void ty_timing_init(struct ITiming *pITiming, unsigned char num){
    for(unsigned char i=0;i<num;i++){
        memset((unsigned char *)&pITiming[i],0,sizeof(struct ITiming)); 
    }
    timing_num = num;
    pITimings = pITiming;

    static TimerHandle_t g_timing_timer    = NULL;
    if(g_timing_timer == NULL){
        g_timing_timer =  xTimerCreate("cmp_timing timer", 20000, true, NULL, cmp_timing_timer_cb);//20s
        xTimerStart(g_timing_timer,0);
    }
}

void ty_timing_set(struct ITiming *pITiming,unsigned char enable,unsigned short time,unsigned char repet,unsigned char day,unsigned char (*cb)(unsigned char index)){
    ESP_LOGI(TAG, "%d %d %d %d\n",enable,time,repet,day);
    pITiming->enable = (enable == 0?0:1);
    pITiming->time = time;
    pITiming->repet = (repet == 0?0:1);
    pITiming->day = day;
    pITiming->timing_end_cb = cb;
}

void ty_timing_kill(struct ITiming *pITiming){
    pITiming->enable = 0;
}

