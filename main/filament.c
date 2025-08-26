#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "sdkconfig.h"

#include "ble_manager.h"
#include "storage_manager.h"
#include "data_manager.h"
#include "temperature_manager.h"

static const char *TAG = "Filament";

QueueHandle_t ble_data_queue;

void app_main(void)
{

    ESP_LOGI(TAG, "Application Starting. Creating tasks...");

    esp_err_t ret;

    ret = storage_init();

    ESP_ERROR_CHECK(ret);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get initialize storage manager.");
        return;
    }

    ret = data_manager_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Cannot initialize data manager.");
        return;
    }

    ble_data_queue = xQueueCreate(5, sizeof(ble_data_message_t));

    if (ble_data_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create Queue.");
        return;
    }

    ESP_LOGI(TAG, "Quque created successfully. Handle: %p", ble_data_queue);

    if ( ble_init() != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to initialize BLE.");
        return;
    }

    if( init_temperature_sensor() != ESP_OK){
        ESP_LOGE(TAG, "Failed to initialize Temperature sensor.");
        return;
    }

    xTaskCreate(&temperature_manager_task, 
                "temperature_manager_tasks", 
                4096, 
                NULL, 
                4, 
                NULL);


    xTaskCreate(&storage_manager_task, 
                "storage_manager_tasks", 
                4096, 
                NULL, 
                3, 
                NULL);

    ESP_LOGI(TAG, "Tasks created. Application running.");

    return;
    
}
