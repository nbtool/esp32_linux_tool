/*************************************************************************
	> File Name: ic_tm1650_config.h
	> Author: https://blog.csdn.net/luliplus/article/details/124023411
	> Mail: 
	> Created Time: Mon 05 Dec 2022 01:21:11 HKT
 ************************************************************************/

#ifndef _IC_TM1650_CONFIG_H
#define _IC_TM1650_CONFIG_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>

#define GPIO_I2C_DG_SCL         23
#define GPIO_I2C_DG_SDA         22 

//==========【配置IIC驱动引脚】========
 
//配置驱动SCL的gpio为开漏输出模式
#define  TM1650_IIC_SCL_MODE_OD   \
do{\
    gpio_set_direction(GPIO_I2C_DG_SCL, GPIO_MODE_INPUT_OUTPUT_OD);\
    gpio_set_pull_mode(GPIO_I2C_DG_SCL, GPIO_FLOATING);\
}while(0)
 
//配置驱动SDA的gpio为开漏输出模式
#define  TM1650_IIC_SDA_MODE_OD  \
do{ \
    gpio_set_direction(GPIO_I2C_DG_SDA, GPIO_MODE_INPUT_OUTPUT_OD);\
    gpio_set_pull_mode(GPIO_I2C_DG_SDA, GPIO_FLOATING);\
}while(0)
 
//=====================================
 
 
//========【配置IIC总线的信号读写和时序】=======
//主机拉高SCL
#define TM1650_IIC_SCL_HIGH     gpio_set_level(GPIO_I2C_DG_SCL,1)
 
//主机拉低SCL
#define TM1650_IIC_SCL_LOW      gpio_set_level(GPIO_I2C_DG_SCL,0)
 
 
//主机拉高SDA
#define TM1650_IIC_SDA_HIGH     gpio_set_level(GPIO_I2C_DG_SDA,1)
 
//主机拉低SDA
#define TM1650_IIC_SDA_LOW      gpio_set_level(GPIO_I2C_DG_SDA,0)
 
//参数b为0时主机拉低SDA，非0则拉高SDA
#define TM1650_IIC_SDA_WR(b)    do{                                       \
                               if(b) gpio_set_level(GPIO_I2C_DG_SDA,1);   \
                               else  gpio_set_level(GPIO_I2C_DG_SDA,0); \
                              }while(0)
 
 
//主机读取SDA线电平状态，返回值为0为低电平，非0则为高电平
#define TM1650_IIC_SDA_RD()    gpio_get_level(GPIO_I2C_DG_SDA) 
 
//软件延时2us
#define TM1650_IIC_DELAY_2US   ets_delay_us(2)
 
//软件延时4us
#define TM1650_IIC_DELAY_4US   ets_delay_us(4)
//================================
 

#endif
