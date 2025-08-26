#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "freertos/semphr.h"
#include "esp_err.h"

typedef struct{
    float max_temperature;
    float min_temperature;
    int is_on;
} data_params_t;

esp_err_t data_manager_init(void);

void data_manager_get_params(data_params_t *params_out);

void data_manager_set_params(const data_params_t *params_in);

SemaphoreHandle_t data_manager_get_data_ready_semaphore(void);


#endif