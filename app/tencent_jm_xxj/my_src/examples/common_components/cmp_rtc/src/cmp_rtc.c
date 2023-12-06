/*************************************************************************
	> File Name: cmp_rtc.c
	> Author: 
	> Mail: 
	> Created Time: Mon 20 Feb 2023 23:57:15 HKT
 ************************************************************************/

#include "cmp_rtc.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

unsigned char cmp_rtc_init(cmp_rtc_params_s *param);
void cmp_rtc_set_date(cmp_rtc_date_s date);
void cmp_rtc_get_date(cmp_rtc_date_s *date);
void cmp_rtc_date_to_utc(cmp_rtc_date_s date, unsigned int *utc_time);   
void cmp_rtc_utc_to_date(unsigned int utc_time, cmp_rtc_date_s *date);
unsigned char cmp_rtc_is_same(cmp_rtc_date_s *time_stamp1, cmp_rtc_date_s *time_stamp2, cmp_rtc_cmp_kind_s kind);
void cmp_rtc_run(void);

typedef struct{
    unsigned int cur_utc_time;
}_this_s;

static _this_s _this = {
    .cur_utc_time = 0,
};
static _this_s *p_this = &_this;

static const char *TAG = "cmp_rtc";

///////////////////////////////////////////////////////////
//对外功能函数实现
///////////////////////////////////////////////////////////
cmp_rtc_s cmp_rtc = {
    .init = &cmp_rtc_init,
    .set_date = &cmp_rtc_set_date,
    .get_date = &cmp_rtc_get_date,
    .date_to_utc = &cmp_rtc_date_to_utc,
    .utc_to_date = &cmp_rtc_utc_to_date,
    .is_same = &cmp_rtc_is_same,
};

///////////////////////////////////////////////////////////
//UTC和普通时间进行转换的内部函数
///////////////////////////////////////////////////////////
//1970/1/1 8:0:0   0x0000000   星期4
#define UTC_TIME2000 946656000
typedef struct
{
    unsigned char second; // 0-59
    unsigned char minute; // 0-59
    unsigned char hour;   // 0-23
    unsigned char day;    // 1-31
    unsigned char month;  // 1-12
    unsigned char year;   // 0-99 (representing 2000-2099)
}date_time_t;

static unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

unsigned int date_time_to_epoch(date_time_t* date_time)
{
    unsigned int second = date_time->second;  // 0-59
    unsigned int minute = date_time->minute;  // 0-59
    unsigned int hour   = date_time->hour;    // 0-23
    unsigned int day    = date_time->day-1;   // 0-30
    unsigned int month  = date_time->month-1; // 0-11
    unsigned int year   = date_time->year;    // 0-99
    return (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
}

void epoch_to_date_time(date_time_t* date_time,unsigned int epoch)
{
    date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    unsigned int years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    unsigned int year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    unsigned int month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->day   = epoch-days[year][month]+1;
}

static void cmp_rtc_timer_cb(void *timer){
    if(p_this->cur_utc_time < UTC_TIME2000)return;
    static unsigned char times = 0;
	
    times++;
    if(times > 4){
        times = 0;
        p_this->cur_utc_time++;

        //ESP_LOGI(TAG, "%d\n",p_this->cur_utc_time);
    }
}
///////////////////////////////////////////////////////////
//对外函数
///////////////////////////////////////////////////////////
unsigned char cmp_rtc_init(cmp_rtc_params_s *param){
    TimerHandle_t g_rtc_timer    = NULL;
    g_rtc_timer =  xTimerCreate("cmp_rtc timer", 200, true, NULL, cmp_rtc_timer_cb);//200ms
    xTimerStart(g_rtc_timer, 0);                                                                                   

    return 0; 
}

void cmp_rtc_set_date(cmp_rtc_date_s date){
    unsigned int cur_utc_time;
    cmp_rtc_date_to_utc(date,&cur_utc_time);
    p_this->cur_utc_time = cur_utc_time;
}

void cmp_rtc_get_date(cmp_rtc_date_s *date){
    cmp_rtc_utc_to_date(p_this->cur_utc_time,date);
    date->weekday = ((p_this->cur_utc_time+8*60*60)/(24*60*60) + 4)%7;
    if(date->weekday == 0)date->weekday = 7;
}

void cmp_rtc_date_to_utc(cmp_rtc_date_s date, unsigned int *utc_time){
    if(date.year < 2000){
        *utc_time = 0;
        return;
    }
    date_time_t now_time;
    now_time.second = date.second;
    now_time.minute = date.minute;
    now_time.hour = date.hour;
    now_time.day = date.monthday;
    now_time.month = date.month;
    now_time.year = date.year - 2000;//from 2000/01/01/00/00/00 epoch time
    
    unsigned int epoch;
    epoch = date_time_to_epoch(&now_time);
    epoch += UTC_TIME2000;
    
    *utc_time = epoch;
}

void cmp_rtc_utc_to_date(unsigned int utc_time, cmp_rtc_date_s *date){
    if(utc_time < UTC_TIME2000){
        memset(date,0,sizeof(cmp_rtc_date_s));
        return;
    } 

    utc_time -= UTC_TIME2000;
    date_time_t now_time;
    epoch_to_date_time(&now_time,utc_time);

    date->second = now_time.second;
    date->minute = now_time.minute;
    date->hour = now_time.hour;
    date->monthday = now_time.day;
    date->month = now_time.month;
    date->year = (unsigned short)now_time.year + 2000;
}

unsigned char cmp_rtc_is_same(cmp_rtc_date_s *time_stamp1, cmp_rtc_date_s *time_stamp2, cmp_rtc_cmp_kind_s kind){
    switch(kind){
        case CMP_RTC_IS_SAME_SECOND:
            if(time_stamp1->second != time_stamp2->second)return 0;
        case CMP_RTC_IS_SAME_MINUTE:
            if(time_stamp1->minute != time_stamp2->minute)return 0;
        case CMP_RTC_IS_SAME_HOUR:
            if(time_stamp1->hour != time_stamp2->hour)return 0;
        case CMP_RTC_IS_SAME_DAY:
            if(time_stamp1->monthday != time_stamp2->monthday)return 0;
        case CMP_RTC_IS_SAME_MONTH:
            if(time_stamp1->month != time_stamp2->month)return 0;
        case CMP_RTC_IS_SAME_YEAR:
            if(time_stamp1->year != time_stamp2->year)return 0;
            else return 1;
        default:break;
    }
    return 0;
}

