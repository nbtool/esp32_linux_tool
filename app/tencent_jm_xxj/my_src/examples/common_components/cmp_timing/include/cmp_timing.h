/*************************************************************************
	> File Name: cmp_timing.h
	> Author: 
	> Mail: 
	> Created Time: Mon 27 Feb 2023 00:31:07 HKT
 ************************************************************************/

#ifndef _CMP_TIMING_H
#define _CMP_TIMING_H

//
//enum{
//    TIMING_STATE_NO = 0x00,
//	TIMING_STATE_IN = 0x01,
//	TIMING_STATE_END = 0x02,
//};

//
struct ITiming{
    unsigned char enable:1;                    //是否使能该定时动作
    unsigned char repet:1;                     //是否重复定时
    unsigned char timing_state:4;              //当前定时的状态
    unsigned char day;                         //一周中有哪些天是重复定时
    unsigned short time;                       //分钟来计算的一天的时间
    unsigned char (*timing_end_cb)(unsigned char index);  //定时触发回调函数
};

struct ITimingFunc{
    void (*init)(struct ITiming *pITiming, unsigned char num);
    void (*set)(struct ITiming *pITiming,unsigned char enable,unsigned short time,unsigned char repet,unsigned char day,unsigned char (*cb)(unsigned char index));
    void (*kill)(struct ITiming *pITiming);
};


extern struct ITimingFunc       mITimingFunc;



#endif
