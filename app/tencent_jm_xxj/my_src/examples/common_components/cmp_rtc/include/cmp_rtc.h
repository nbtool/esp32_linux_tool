/*************************************************************************
	> File Name: cmp_rtc.h
	> Author: 
	> Mail: 
	> Created Time: Mon 20 Feb 2023 23:57:19 HKT
 ************************************************************************/

#ifndef _CMP_RTC_H
#define _CMP_RTC_H


typedef struct{
	void *a;
}cmp_rtc_params_s;

//from 2000/01/01/00/00/00 epoch time
typedef struct{
  unsigned char second;//0~59
  unsigned char minute;//0~59
  unsigned char hour;//0~23
  unsigned char weekday;//1~7(7:星期天)
  unsigned char monthday;//1~31
  unsigned char month;//1~12
  unsigned short year;//
}cmp_rtc_date_s;

#define CMP_RTC_Month_January                ((unsigned char)0x01)
#define CMP_RTC_Month_February               ((unsigned char)0x02)
#define CMP_RTC_Month_March                  ((unsigned char)0x03)
#define CMP_RTC_Month_April                  ((unsigned char)0x04)
#define CMP_RTC_Month_May                    ((unsigned char)0x05)
#define CMP_RTC_Month_June                   ((unsigned char)0x06)
#define CMP_RTC_Month_July                   ((unsigned char)0x07)
#define CMP_RTC_Month_August                 ((unsigned char)0x08)
#define CMP_RTC_Month_September              ((unsigned char)0x09)
#define CMP_RTC_Month_October                ((unsigned char)0x10)
#define CMP_RTC_Month_November               ((unsigned char)0x11)
#define CMP_RTC_Month_December               ((unsigned char)0x12)

#define CMP_RTC_Weekday_Monday               ((unsigned char)0x01)
#define CMP_RTC_Weekday_Tuesday              ((unsigned char)0x02)
#define CMP_RTC_Weekday_Wednesday            ((unsigned char)0x03)
#define CMP_RTC_Weekday_Thursday             ((unsigned char)0x04)
#define CMP_RTC_Weekday_Friday               ((unsigned char)0x05)
#define CMP_RTC_Weekday_Saturday             ((unsigned char)0x06)
#define CMP_RTC_Weekday_Sunday               ((unsigned char)0x07)

#define CMP_RTC_IS_SECOND(N) ((N) < 60)
#define CMP_RTC_IS_MINUTE(N) ((N) < 60)
#define CMP_RTC_IS_HOUR(N) ((N) < 24)
#define CMP_RTC_IS_WEEKDAY(N) (((N) > 0) && ((N) < 8))
#define CMP_RTC_IS_MONTHDAY(N) (((N) > 0) && ((N) < 32))
#define CMP_RTC_IS_MONTH(N) (((N) > 0) && ((N) < 13))
#define CMP_RTC_IS_YEAR(N) (((N) > 0) && ((N) < 4000))

#define CMP_RTC_IS_MATCH_WEEKDAY(N) ((N) < 8)
#define CMP_RTC_IS_MATCH_MONTHDAY(N) ((N) < 32)
#define CMP_RTC_IS_MATCH_MONTH(N) ((N) < 13)
#define CMP_RTC_IS_MATCH_YEAR(N) ((N) < 4000)


#define cmp_rtc_cmp_kind_s                   unsigned char
#define CMP_RTC_IS_SAME_SECOND               ((unsigned char)0x00)
#define CMP_RTC_IS_SAME_MINUTE               ((unsigned char)0x02)
#define CMP_RTC_IS_SAME_HOUR                 ((unsigned char)0x03)
#define CMP_RTC_IS_SAME_DAY                  ((unsigned char)0x04)
#define CMP_RTC_IS_SAME_MONTH                ((unsigned char)0x05)
#define CMP_RTC_IS_SAME_YEAR                 ((unsigned char)0x06)

typedef struct{
    unsigned char (*init)(cmp_rtc_params_s *param);
    void (*set_date)(cmp_rtc_date_s date);
    void (*get_date)(cmp_rtc_date_s *date);
    void (*date_to_utc)(cmp_rtc_date_s date, unsigned int *utc_time);//utc - second
    void (*utc_to_date)(unsigned int utc_time, cmp_rtc_date_s *date);
    unsigned char (*is_same)(cmp_rtc_date_s *time_stamp1, cmp_rtc_date_s *time_stamp2, cmp_rtc_cmp_kind_s kind);
}cmp_rtc_s;

extern cmp_rtc_s cmp_rtc;


#endif
