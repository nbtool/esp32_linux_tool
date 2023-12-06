/*************************************************************************
	> File Name: wifi_server.c
	> Author: 
	> Mail: 
	> Created Time: Sun 05 Mar 2023 19:29:08 HKT
 ************************************************************************/
#include "cJSON.h"
#include "string.h"

#include "esp_qcloud_log.h"
#include "esp_qcloud_console.h"
#include "esp_qcloud_storage.h"
#include "esp_qcloud_iothub.h"
#include "esp_qcloud_prov.h"

#include "app_driver.h"
#include "app_fragrance.h"

static const char *TAG = "cmp_xxj_wifi_server";


/* Callback to handle commands received from the QCloud cloud */
static esp_err_t wifi_cmd_get_param(const char *id, esp_qcloud_param_val_t *val){
#if 0
    if (!strcmp(id, "power_switch")) {
        val->b = light_driver_get_switch();
    } else if (!strcmp(id, "value")) {
        val->i = light_driver_get_value();
    } else if (!strcmp(id, "hue")) {
        val->i = light_driver_get_hue();
    } else if (!strcmp(id, "saturation")) {
val->i = light_driver_get_saturation();
    }
#endif
    ESP_LOGI(TAG, "Report id: %s, val: %d", id, val->i);

    return ESP_OK;
}

/* Callback to handle commands received from the QCloud cloud */
static esp_err_t wifi_cmd_set_param(const char *id, const esp_qcloud_param_val_t *val){
    esp_err_t err = ESP_FAIL;
    ESP_LOGI(TAG, "Received id: %s, val: %d", id, val->i);

    if (!strcmp(id, "onoff")) {
        err = app_fragrance_set_onoff(val->b);
    } else if (!strcmp(id, "work_model")) {
        err = app_fragrance_set_work_model(val->obj);
    } else if (!strcmp(id,"wind")){
        err = app_fragrance_set_fan_speed(val->i);
    } else {
        ESP_LOGW(TAG, "This parameter is not supported");
    }

    return err;
}

/* Event handler for catching QCloud events */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data){
    switch (event_id) {
        case QCLOUD_EVENT_IOTHUB_INIT_DONE:
            esp_qcloud_iothub_report_device_info();
            ESP_LOGI(TAG, "QCloud Initialised");
            break;

        case QCLOUD_EVENT_IOTHUB_BOND_DEVICE:
            ESP_LOGI(TAG, "Device binding successful");
            break;

        case QCLOUD_EVENT_IOTHUB_UNBOND_DEVICE:
            ESP_LOGW(TAG, "Device unbound with iothub");
            esp_qcloud_wifi_reset();
            esp_restart();
            break;

        case QCLOUD_EVENT_IOTHUB_BIND_EXCEPTION:
            ESP_LOGW(TAG, "Device bind fail");
            esp_qcloud_wifi_reset();
            esp_restart();
            break;
            
        case QCLOUD_EVENT_IOTHUB_RECEIVE_STATUS:
            ESP_LOGI(TAG, "receive status message: %s",(char*)event_data);
            break;

        default:
            ESP_LOGW(TAG, "Unhandled QCloud Event: %d", event_id);
    }
}

static esp_err_t get_wifi_config(wifi_config_t *wifi_cfg, uint32_t wait_ms){
    ESP_QCLOUD_PARAM_CHECK(wifi_cfg);

    if (esp_qcloud_storage_get("wifi_config", wifi_cfg, sizeof(wifi_config_t)) == ESP_OK) {

#ifdef CONFIG_BT_ENABLE
    esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
#endif

        return ESP_OK;
    }

    /**< Reset wifi and restart wifi */
    esp_wifi_restore();
    esp_wifi_start();

    /**< The yellow light flashes to indicate that the device enters the state of configuring the network */
    app_driver_set_wifi_state(2); /**< 0- NOT CONNECT; 1-CONNECT; 2-PAIRING*/

    /**< Note: Smartconfig and softapconfig working at the same time will affect the configure network performance */

#ifdef CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG
    char softap_ssid[32 + 1] = CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG_SSID;
    // uint8_t mac[6] = {0};
    // ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
    // sprintf(softap_ssid, "tcloud_%s_%02x%02x", light_driver_get_type(), mac[4], mac[5]);

    esp_qcloud_prov_softapconfig_start(SOFTAPCONFIG_TYPE_ESPRESSIF_TENCENT,
                                       softap_ssid,
                                       CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG_PASSWORD);
    esp_qcloud_prov_print_wechat_qr(softap_ssid, "softap");
#endif

#ifdef CONFIG_LIGHT_PROVISIONING_SMARTCONFIG
    esp_qcloud_prov_smartconfig_start(SC_TYPE_ESPTOUCH_AIRKISS);
#endif

#ifdef CONFIG_LIGHT_PROVISIONING_BLECONFIG
    char local_name[32 + 1] = CONFIG_LIGHT_PROVISIONING_BLECONFIG_NAME;
    esp_qcloud_prov_bleconfig_start(BLECONFIG_TYPE_ESPRESSIF_TENCENT, local_name);
#endif

    ESP_ERROR_CHECK(esp_qcloud_prov_wait(wifi_cfg, wait_ms));

#ifdef CONFIG_LIGHT_PROVISIONING_SMARTCONFIG
    esp_qcloud_prov_smartconfig_stop();
#endif

#ifdef CONFIG_LIGHT_PROVISIONING_SOFTAPCONFIG
    esp_qcloud_prov_softapconfig_stop();
#endif

    /**< Store the configure of the device */
    esp_qcloud_storage_set("wifi_config", wifi_cfg, sizeof(wifi_config_t));

    /**< Configure the network successfully to stop the light flashing */
    app_driver_set_wifi_state(1); /**< stop blink */

    return ESP_OK;
}


void app_wifi_server_init(void){
    /*
     * @breif Create a device through the server and obtain configuration parameters
     *        server: https://console.cloud.tencent.com/iotexplorer
     */
    /**< Create and configure device authentication information */
    ESP_ERROR_CHECK(esp_qcloud_create_device());
    /**< Configure the version of the device, and use this information to determine whether to OTA */
    ESP_ERROR_CHECK(esp_qcloud_device_add_fw_version("0.0.5"));
    /**< Register the properties of the device */
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("onoff", QCLOUD_VAL_TYPE_BOOLEAN));
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("work_model", QCLOUD_VAL_TYPE_STRUCT));
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("a_left", QCLOUD_VAL_TYPE_INTEGER));
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("b_left", QCLOUD_VAL_TYPE_INTEGER));
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("wind", QCLOUD_VAL_TYPE_INTEGER));
    /**< The processing function of the communication between the device and the server */
    ESP_ERROR_CHECK(esp_qcloud_device_add_property_cb(wifi_cmd_get_param, wifi_cmd_set_param));
    
    /**
     * @brief Initialize Wi-Fi.
     */
    ESP_ERROR_CHECK(esp_qcloud_wifi_init());
    ESP_ERROR_CHECK(esp_event_handler_register(QCLOUD_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    /**
     * @brief Get the router configuration
     */
    wifi_config_t wifi_cfg = {0};
    ESP_ERROR_CHECK(get_wifi_config(&wifi_cfg, portMAX_DELAY));

    /**
     * @brief Connect to router
     */
    ESP_ERROR_CHECK(esp_qcloud_wifi_start(&wifi_cfg));
    ESP_ERROR_CHECK(esp_qcloud_timesync_start());

    /**
     * @brief Connect to Tencent Cloud Iothub
     */
    ESP_ERROR_CHECK(esp_qcloud_iothub_init());
    ESP_ERROR_CHECK(esp_qcloud_iothub_start());
    ESP_ERROR_CHECK(esp_qcloud_iothub_ota_enable());
}
