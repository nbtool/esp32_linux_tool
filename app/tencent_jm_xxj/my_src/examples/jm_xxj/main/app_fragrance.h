/*************************************************************************
	> File Name: app_fragrance.h
	> Author: 
	> Mail: 
	> Created Time: Sat 03 Dec 2022 22:03:26 HKT
 ************************************************************************/

#ifndef _APP_FRAGRANCE_H
#define _APP_FRAGRANCE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/param.h>


esp_err_t app_fragrance_set_onoff(bool on);
esp_err_t app_fragrance_set_fan_speed(int speed);
esp_err_t app_fragrance_set_work_model(char *work_model);
esp_err_t app_fragrance_init(void);

#endif
