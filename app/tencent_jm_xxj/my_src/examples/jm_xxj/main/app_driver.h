/*************************************************************************
	> File Name: app_driver.h
	> Author: 
	> Mail: 
	> Created Time: Sun 04 Dec 2022 16:09:36 HKT
 ************************************************************************/

#ifndef _APP_DRIVER_H
#define _APP_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/param.h>


esp_err_t app_driver_init(void);
void app_driver_set_wifi_state(char state);
void app_driver_set_fragrance_onoff(char device, bool on);
void app_driver_set_fan_speed(int speed);


#endif
