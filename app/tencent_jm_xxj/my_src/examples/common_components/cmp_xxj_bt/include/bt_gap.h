/*************************************************************************
	> File Name: bt_gap.h
	> Author: 
	> Mail: 
	> Created Time: Sun 19 Feb 2023 01:40:34 HKT
 ************************************************************************/

#ifndef _BT_GAP_H
#define _BT_GAP_H

#include "nimble/ble.h"
#include "modlog/modlog.h"

#ifdef __cplusplus
extern "C" {
#endif

/* XXJ configuration */
#define GATT_XXJ_UUID                           0xFCF0
#define GATT_XXJ_NOTIFY_UUID                    0xFCF1
#define GATT_XXJ_WRITE_NO_ACK_UUID              0xFCF2
#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

extern uint16_t xxj_handle;
extern int bt_gatt_init(void);


void app_bt_gap_init(void);
void app_bt_gatt_send(uint8_t *datas, uint8_t len);

#ifdef __cplusplus
}
#endif




#endif
