/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "app_entry.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"

// #define PRODUCT_KEY                  NULL
#define PRODUCT_KEY                     "a1QynQ2BCLJ"
#define PRODUCT_SECRET                  "GGhRmKHLS632t8WX"
#define DEVICE_NAME                     "device1"
#define DEVICE_SECRET                   "TbJkO9e2cMusqkDzXdasoVVJe7OdgjXx"
      
/* These are pre-defined topics */
#define TOPIC_UPDATE                    "/"PRODUCT_KEY"/"DEVICE_NAME"/user/update"
#define TOPIC_ERROR                     "/"PRODUCT_KEY"/"DEVICE_NAME"/user/update/error"
#define TOPIC_GET                       "/"PRODUCT_KEY"/"DEVICE_NAME"/user/get"

#define DEVICE_PROPERTY_POST            "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define DEVICE_PROPERTY_POST_REPLY      "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define DEVICE_PROPERTY_SET             "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define DEVICE_INFO_UPDATE              "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/deviceinfo/update"

#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char   **user_argv;

static const char *TAG = "MQTT";

static char update_flag = 1;
typedef struct{
    bool PowerSwitch;
    bool OilShortage;
    char SprayLevel;
    char timer[41];
    char time_syn[21];
}dps_s;

dps_s dps = {
    .PowerSwitch = 0,
    .OilShortage = 0,
    .SprayLevel = 0,
    .timer = "0123456789012345678901234567890123456789",
    .time_syn = "1512038504",
};


char get_value(const char *jsonRoot){
    // jsonRoot 是您要剖析的数据
    //首先整体判断是否为一个json格式的数据
	cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
	//如果是否json格式数据
	if (pJsonRoot !=NULL) {
        cJSON *pParams = cJSON_GetObjectItem(pJsonRoot, "params");
        if(pParams != NULL){
            cJSON *pValue =  cJSON_GetObjectItem(pParams, "PowerSwitch");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: PowerSwitch:%d",pValue->valueint);
                dps.PowerSwitch = pValue->valueint;
            }
            pValue =  cJSON_GetObjectItem(pParams, "OilShortage");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: OilShortage:%d",pValue->valueint);
                dps.OilShortage = pValue->valueint;
            }
            pValue =  cJSON_GetObjectItem(pParams, "SprayLevel");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: SprayLevel:%d",pValue->valueint);
                dps.SprayLevel = pValue->valueint;
            }
            pValue =  cJSON_GetObjectItem(pParams, "timer");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: timer:%s",pValue->valuestring);
                memset(dps.timer,0,sizeof(dps.timer));
                memcpy(dps.timer,pValue->valuestring,strlen(pValue->valuestring)); 
            }
            pValue =  cJSON_GetObjectItem(pParams, "time_syn");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: time_syn:%s",pValue->valuestring);
                memset(dps.time_syn,0,sizeof(dps.time_syn));
                memcpy(dps.time_syn,pValue->valuestring,strlen(pValue->valuestring));
            }
            update_flag = 1;
        }
    }    

    return 1;
}


void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt     ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("----");
            EXAMPLE_TRACE("PacketId: %d", ptopic_info->packet_id);
            EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                          ptopic_info->topic_len,
                          ptopic_info->ptopic,
                          ptopic_info->topic_len);
            EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                          ptopic_info->payload_len,
                          ptopic_info->payload,
                          ptopic_info->payload_len);
            get_value(ptopic_info->payload);
            EXAMPLE_TRACE("----");
            break;
        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

int mqtt_client(void)
{
    int rc, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[256];

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        return -1;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //send data to topic
    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "update: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = IOT_MQTT_Publish(pclient, TOPIC_UPDATE, &topic_msg);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("error occur when publish");
        return -1;
    }

    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_UPDATE, topic_msg.payload, rc);

    //////////////////////////////////////////////////////////////////////////////////////////////
    //Subscribe a topic, then send a top
    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_PROPERTY_SET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_INFO_UPDATE, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_GET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_PROPERTY_POST_REPLY, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }

    IOT_MQTT_Yield(pclient, 200);

    HAL_SleepMs(2000);

    /* Initialize topic information */
    memset(msg_pub, 0x0, 256);
    strcpy(msg_pub, "data: hello! start!");
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = IOT_MQTT_Publish(pclient, TOPIC_UPDATE, &topic_msg);
    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_UPDATE, topic_msg.payload, rc);

    IOT_MQTT_Yield(pclient, 200);

    //////////////////////////////////////////////////////////////////////////////////////////////
    //then send a top

    do {
        /* Generate topic message */
        cnt++;
        if(update_flag){
            update_flag = 0;
            msg_len = snprintf(msg_pub, sizeof(msg_pub), 
                "{\"method\":\"thing.event.property.post\",\"id\":\"7\",\"version\":\"1.0\",\"params\":{\"PowerSwitch\":%d,\"OilShortage\":%d,\"SprayLevel\":%d,\"timer\":\"%s\",\"time_syn\":\"%s\"}}",
                dps.PowerSwitch, dps.OilShortage, dps.SprayLevel, dps.timer, dps.time_syn);

            if (msg_len < 0) {
                EXAMPLE_TRACE("Error occur! Exit program");
                return -1;
            }

            topic_msg.payload = (void *)msg_pub;
            topic_msg.payload_len = msg_len;

            rc = IOT_MQTT_Publish(pclient, DEVICE_PROPERTY_POST, &topic_msg);
            if (rc < 0) {
                EXAMPLE_TRACE("error occur when publish");
            }
            EXAMPLE_TRACE("packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);
        }

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 1000);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            //HAL_SleepMs(2000);
            //cnt = 0;
        }
        ESP_LOGI(TAG, "min:%u heap:%u", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
    } while (cnt);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Unsubscribe(pclient, DEVICE_PROPERTY_SET);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

    return 0;
}

int linkkit_main(void *paras)
{
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = 0;
    user_argv = NULL;

    if (paras != NULL) {
        app_main_paras_t *p = (app_main_paras_t *)paras;
        user_argc = p->argc;
        user_argv = p->argv;
    }

    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);
    HAL_SetProductSecret(PRODUCT_SECRET);
    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login  Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    mqtt_client();
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);

    EXAMPLE_TRACE("out of sample!");

    return 0;
}
