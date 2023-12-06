/*************************************************************************
	> File Name: ic_tm1650.h
	> Author: 
	> Mail: 
	> Created Time: Mon 05 Dec 2022 00:47:47 HKT
 ************************************************************************/

#ifndef _IC_TM1650_H
#define _IC_TM1650_H

//显示参数
#define TM1650_BRIGHT1       0x11   /*一级亮度，打开LED显示*/
#define TM1650_BRIGHT2       0x21   /*二级亮度，打开LED显示*/
#define TM1650_BRIGHT3       0x31   /*三级亮度，打开LED显示*/
#define TM1650_BRIGHT4       0x41   /*四级亮度，打开LED显示*/
#define TM1650_BRIGHT5       0x51   /*五级亮度，打开LED显示*/
#define TM1650_BRIGHT6       0x61   /*六级亮度，打开LED显示*/
#define TM1650_BRIGHT7       0x71   /*七级亮度，打开LED显示*/
#define TM1650_BRIGHT8       0x01   /*八级亮度，打开LED显示*/
#define TM1650_DSP_OFF       0x00   /*关闭LED显示*/
 
//数码管位选
#define TM1650_DIG1     0
#define TM1650_DIG2     1
#define TM1650_DIG3     2
#define TM1650_DIG4     3
 
void TM1650_init(void);
void TM1650_cfg_display(unsigned char param);
void TM1650_clear(void);
void TM1650_print(unsigned char dig,unsigned char seg_data);

#endif
