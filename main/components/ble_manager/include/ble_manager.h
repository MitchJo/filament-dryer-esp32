#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define GATTS_TAG "GATTS"

#define GATTS_SERVICE_UUID   0x00FF
#define GATTS_CHAR_UUID      0xFF01
#define GATTS_DESCR_UUID     0x3333
#define GATTS_NUM_HANDLE    4

// #define GATTS_SERVICE_UUID_B   0x00EE
// #define GATTS_CHAR_UUID_B      0xEE01
// #define GATTS_DESCR_UUID_B     0x2222
// #define GATTS_NUM_HANDLE_B     4

#define DEVICE_NAME            "Filament-"
#define MAX_BLE_NAME_LEN (29 + 1)
#define GATTS_CHAR_VAL_LEN_MAX 0x40

#define PREPARE_BUF_MAX_SIZE 1024

#define MAX_JSON_DATA_SIZE 256

#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)


#define PROFILE_NUM 1
#define PROFILE_APP_ID 0
// #define PROFILE_APP_ID_B 1

extern QueueHandle_t ble_data_queue;

typedef struct {
    char json_data[MAX_JSON_DATA_SIZE];
} ble_data_message_t;

esp_err_t ble_init(void);

void send_notification(int profile_id, const float *data, size_t len);

#endif