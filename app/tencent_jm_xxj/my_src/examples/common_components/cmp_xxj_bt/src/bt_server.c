/*************************************************************************
	> File Name: bt_cmd.c
	> Author: 
	> Mail: 
	> Created Time: Mon 27 Feb 2023 01:51:58 HKT
 ************************************************************************/
#include <stdio.h>
#include "string.h"
#include "cmp_rtc.h"
#include "bt_gap.h"

#include "host/ble_hs.h"
#include "esp_qcloud_storage.h"

void bt_server_cmd(uint8_t *datas, uint8_t len);

char WORK_DATA_KEY[12] = "work_data_0";   
static uint8_t _check_sum(uint8_t *datas, uint8_t len){
    uint8_t sum = 0;
    for(uint8_t i=0;i<len;i++){
        sum += datas[i];
    }
    return sum;
}

void bt_server_init(void){
    // use flash data init device
    uint8_t work_data[20];
    WORK_DATA_KEY[10] = '0';
    if(esp_qcloud_storage_get(WORK_DATA_KEY, work_data, 14) == ESP_OK){
        bt_server_cmd(work_data,14);
    }
    WORK_DATA_KEY[10] = '3';
    if(esp_qcloud_storage_get(WORK_DATA_KEY, work_data, 14) == ESP_OK){
        bt_server_cmd(work_data,14);
    }
}

void bt_server_cmd(uint8_t *datas, uint8_t len){
    MODLOG_DFLT(INFO, "write:[%d]={",len);
    for(int i=0;i<len;i++){
        printf("%02x ",datas[i]);
    }
    printf("\n");

    uint8_t send[14];
	memset(send,0,14);

    if(len == 14){
        //hal_uart_send(&p->value,len);
		uint8_t cmd = datas[0];
		uint8_t* params = &(datas[1]);

        switch(cmd){
            case 0x01:{//start stop
                uint8_t moto_onoff = 1;
            
                if(params[0] == 0){//stop
                    moto_onoff = 0;   
                }else if(params[0] == 1){//start
                    moto_onoff = 1;
                }

                send[0] = 0x01;
                send[1] = moto_onoff;
                send[13] = _check_sum(send,13);
                app_bt_gatt_send(send,14);
            }
            break;
            case 0x02:{//read level
                extern int hell1;
                extern int hell2;
                send[0] = 0x02;
                send[1] = hell1;
                send[2] = hell2;
                send[13] = _check_sum(send,13);
				app_bt_gatt_send(send,14);
            }
            break;
            case 0x04:{//fan control
                uint8_t fan_level = 0;
                if(params[0] < 0x04){
                    fan_level = params[0];
                }
                send[0] = 0x04;
                send[1] = fan_level;
                send[13] = _check_sum(send,13);
				app_bt_gatt_send(send,14);
            }
            break;
            case 0x05:{//set work
                extern unsigned char timing_work_mode_set(unsigned char timer_id,unsigned char is_on,unsigned short time_from,unsigned short time_to,unsigned char is_repet,unsigned char day, unsigned char level,unsigned short _work, unsigned short _not_work);
                uint8_t index = params[0];
                uint16_t time_from = params[1]*60+params[2];
                uint16_t time_to = params[3]*60+params[4];
                uint16_t _work = params[6]*256+params[5];
                uint16_t _not_work = params[8]*256+params[7];
                uint8_t week = params[9];
                uint8_t level = params[10];
                uint8_t is_on = 1;
                uint8_t is_repet = 1;
                timing_work_mode_set(index,is_on,time_from,time_to,is_repet,week,level,_work,_not_work);

				MODLOG_DFLT(INFO,"SET WORK:[%d] %d->%d ^%dv%d W[%x] P[%d]\n",
										index,
										time_from,
										time_to,
										_work,
										_not_work,
										week,
										level);
                WORK_DATA_KEY[10] = '0'+index;
                if(esp_qcloud_storage_set(WORK_DATA_KEY,datas , 14) == ESP_OK){
                    MODLOG_DFLT(INFO, "WRITE OK:%s\n",WORK_DATA_KEY);
                }
            }
            break;
            case 0x06:{//read work
                uint8_t index = params[0];
                WORK_DATA_KEY[10] = '0'+index;
                if(esp_qcloud_storage_get(WORK_DATA_KEY, send, 14) != ESP_OK){
                    memset(send,0,14);
                }else{
                    MODLOG_DFLT(INFO, "READ OK:%s\n",WORK_DATA_KEY);
                }
                send[0] = 0x06;
                send[13] = _check_sum(send,13);
                MODLOG_DFLT(INFO, "upload:[14]={");
                for(int i=0;i<14;i++){
                    printf("%02x ",send[i]);
                }
                printf("\n");

				app_bt_gatt_send(send,14);
            }
            break;
            case 0x07:{//set time
                cmp_rtc_date_s date1;
				date1.year = params[0]*100+params[1];
				date1.month = params[2];
				date1.monthday = params[3];
                date1.weekday = 0;
				date1.hour = params[4];
				date1.minute = params[5];
				date1.second = params[6];
				cmp_rtc.set_date(date1);

				MODLOG_DFLT(INFO,"SET TIME:%04d/%02d/%02d %02d:%02d:%02d [XQ:%d]\n",
										date1.year,
										date1.month,
										date1.monthday,
										date1.hour,
										date1.minute,
										date1.second,
										date1.weekday);

            }
            break;
            case 0x08:{//read time
                cmp_rtc_date_s date2;
				cmp_rtc.get_date(&date2);

				MODLOG_DFLT(INFO,"GET TIME:%04d/%02d/%02d %02d:%02d:%02d [XQ:%d]\n",
						date2.year,
						date2.month,
						date2.monthday,
						date2.hour,
						date2.minute,
						date2.second,
						date2.weekday);

				send[0] = 0x08;
				send[1] = date2.year/100;
				send[2] = date2.year%100;
				send[3] = date2.month;
				send[4] = date2.monthday;
				send[5] = date2.hour;
				send[6] = date2.minute;
				send[7] = date2.second;
                send[13] = _check_sum(send,13);
				app_bt_gatt_send(send,14);
            }
            break;
            default:break;
        }
    }
}
