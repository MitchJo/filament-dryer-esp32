#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "esp_err.h"

#define NVS_DATA_KEY "data_params"

esp_err_t storage_init(void);

void storage_manager_task(void* pvParameter);

#endif