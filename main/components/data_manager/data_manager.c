#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "data_manager.h"
#include "esp_log.h"

static const char *TAG = "DATA_MANAGER";

// Private static variables, only accessible within this component
static data_params_t s_latest_pid_params = {0};

static SemaphoreHandle_t s_context_mutex;
static SemaphoreHandle_t s_data_ready_sem;

esp_err_t data_manager_init(void) {
    
    s_context_mutex = xSemaphoreCreateMutex();
    s_data_ready_sem = xSemaphoreCreateBinary();

    if (s_context_mutex == NULL || s_data_ready_sem == NULL) {
        ESP_LOGI(TAG, "Failed to create mutex or semaphore.");
        return ESP_FAIL;
    }
    
    // Initialize the shared parameters to a default state
    s_latest_pid_params = (data_params_t) {
        .max_temperature = 60.0f,
        .min_temperature = 40.0f,
        .is_on = 0
    };

    // A good practice: ensure the semaphore starts in the 'empty' state
    xSemaphoreTake(s_data_ready_sem, 0); 

    ESP_LOGI(TAG, "Data Manager initialized.");

    return ESP_OK;
}

void data_manager_get_params(data_params_t *params_out) {
    if (xSemaphoreTake(s_context_mutex, portMAX_DELAY) == pdTRUE) {
        *params_out = s_latest_pid_params;
        xSemaphoreGive(s_context_mutex);
    } else {
        ESP_LOGE(TAG, "Failed to get mutex in pid_manager_get_params!");
    }
}

void data_manager_set_params(const data_params_t *params_in) {
    if (xSemaphoreTake(s_context_mutex, portMAX_DELAY) == pdTRUE) {
        s_latest_pid_params = *params_in;
        xSemaphoreGive(s_context_mutex);
    } else {
        ESP_LOGE(TAG, "Failed to get mutex in pid_manager_set_params!");
    }
}

SemaphoreHandle_t data_manager_get_data_ready_semaphore(void) {
    return s_data_ready_sem;
}