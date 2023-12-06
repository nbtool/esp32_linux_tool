/*************************************************************************
	> File Name: ic_ds1302_config.h
	> Author: 
	> Mail: 
	> Created Time: Tue 01 Aug 2023 15:21:40 CST
 ************************************************************************/

#ifndef _IC_DS1302_CONFIG_H
#define _IC_DS1302_CONFIG_H

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>


#define GPIO_I2C_GC_SCL         13
#define GPIO_I2C_GC_IO          32 
#define GPIO_I2C_GC_CE          27


//==========【配置IIC驱动引脚】========
#include "string.h"
#define GPIO_BIT(x) (1ULL << (x))
#define  DS1302_IIC_IO_INIT  \
do{ \
    gpio_config_t io_conf; \
    memset(&io_conf, 0, sizeof(gpio_config_t));\
    io_conf.mode = GPIO_MODE_OUTPUT;\
    io_conf.pin_bit_mask =\
            GPIO_BIT(GPIO_I2C_GC_CE) |\
            GPIO_BIT(GPIO_I2C_GC_IO) |\
            GPIO_BIT(GPIO_I2C_GC_SCL);\
    gpio_config(&io_conf);\
}while(0)

//=====================================

#define DS1302_IIC_IO_IN           do{\
    gpio_set_direction(GPIO_I2C_GC_IO,GPIO_MODE_INPUT);\
    gpio_set_pull_mode(GPIO_I2C_GC_CE,GPIO_PULLUP_ONLY);\
}while(0)
#define DS1302_IIC_IO_OUTPUT       do{\
    gpio_set_direction(GPIO_I2C_GC_IO,GPIO_MODE_OUTPUT);\
}while(0)

#define DS1302_IIC_SCL_HIGH        gpio_set_level(GPIO_I2C_GC_SCL,1)
#define DS1302_IIC_IO_HIGH         gpio_set_level(GPIO_I2C_GC_IO,1)
#define DS1302_IIC_CE_HIGH         gpio_set_level(GPIO_I2C_GC_CE,1)

#define DS1302_IIC_SCL_LOW         gpio_set_level(GPIO_I2C_GC_SCL,0)
#define DS1302_IIC_IO_LOW          gpio_set_level(GPIO_I2C_GC_IO,0)
#define DS1302_IIC_CE_LOW          gpio_set_level(GPIO_I2C_GC_CE,0)

#define DS1302_IIC_IO_WRITE(bit)   gpio_set_level(GPIO_I2C_GC_IO,bit) 
#define DS1302_IIC_IO_READ         gpio_get_level(GPIO_I2C_GC_IO)  

#define DS1302_IIC_DELAY_200US      ets_delay_us(200)
#define DS1302_IIC_DELAY_100US      ets_delay_us(100)

#endif
