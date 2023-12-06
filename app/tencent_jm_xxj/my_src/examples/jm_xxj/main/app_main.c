/* LED Light Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_qcloud_log.h"
#include "esp_qcloud_console.h"
#include "esp_qcloud_storage.h"
#include "esp_qcloud_iothub.h"
#include "esp_qcloud_prov.h"

#include "app_fragrance.h"
#include "app_timing.h"
#include "app_driver.h"
#include "bt_gap.h"
#include "wifi_server.h"


#ifdef CONFIG_BT_ENABLE
#include "esp_bt.h"
#endif

static const char *TAG = "app_main";

/*
 * driver -> app_fragrance -> wifi_server -> app_main
 * driver -> app_timing -> cmp_xxj_bt -> app_main
 */
void app_main()
{
    /**
     * @brief Add debug function, you can use serial command and remote debugging.
     */
    esp_qcloud_log_config_t log_config = {
        .log_level_uart = ESP_LOG_INFO,
    };
    ESP_ERROR_CHECK(esp_qcloud_log_init(&log_config));
    /**
     * @brief Set log level
     * @note  This function can not raise log level above the level set using
     * CONFIG_LOG_DEFAULT_LEVEL setting in menuconfig.
     */
    esp_log_level_set("*", ESP_LOG_VERBOSE);

#ifdef CONFIG_LIGHT_DEBUG
    ESP_ERROR_CHECK(esp_qcloud_console_init());
    esp_qcloud_print_system_info(10000);
#endif /**< CONFIG_LIGHT_DEBUG */


    /**< Continuous power off and restart more than five times to reset the device */
    if (esp_qcloud_reboot_unbroken_count() >= 3) {
        ESP_LOGW(TAG, "Erase information saved in flash");
        esp_qcloud_storage_erase(CONFIG_QCLOUD_NVS_NAMESPACE);
    } else if (esp_qcloud_reboot_is_exception(false)) {
        ESP_LOGE(TAG, "The device has been restarted abnormally");
    } else {
    }

    app_driver_init();
    app_fragrance_init();
    app_timing_init(); 
    app_bt_gap_init();
    app_wifi_server_init();
}
