#include <stdio.h>
#include "storage_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "ble_manager.h"
#include "data_manager.h"

#include "esp_log.h"

#include <cJSON.h>

static const char *TAG = "FlashManager";

nvs_handle_t nvs_handler;

esp_err_t storage_init(void)
{

    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = nvs_open("storage", NVS_READWRITE, &nvs_handler);

    return ret;
}

void set_data(cJSON *json_data, data_params_t *out)
{

    cJSON *is_on_item = cJSON_GetObjectItemCaseSensitive(json_data, "is_on");
    cJSON *min_temp_item = cJSON_GetObjectItemCaseSensitive(json_data, "min_temp");
    cJSON *max_temp_item = cJSON_GetObjectItemCaseSensitive(json_data, "max_temp");

    // cJSON *kp_item = cJSON_GetObjectItemCaseSensitive(json_data, "kp");
    // cJSON *kd_item = cJSON_GetObjectItemCaseSensitive(json_data, "kd");
    // cJSON *ki_item = cJSON_GetObjectItemCaseSensitive(json_data, "ki");

    if (is_on_item && cJSON_IsNumber(is_on_item))
    {
        out->is_on = is_on_item->valueint;
    }

    if (min_temp_item && cJSON_IsNumber(min_temp_item))
    {
        out->min_temperature = (float)min_temp_item->valuedouble;;
    }

    if (max_temp_item && cJSON_IsNumber(max_temp_item))
    {
        out->max_temperature = (float)max_temp_item->valuedouble;
    }

    // if (kp_item && cJSON_IsNumber(kp_item))
    // {
    //     out->pid_ctrl_params.kp = (float)kp_item->valuedouble;
    // }

    // if (ki_item && cJSON_IsNumber(ki_item))
    // {
    //     out->pid_ctrl_params.ki = (float)ki_item->valuedouble;
    // }

    // if (kd_item && cJSON_IsNumber(kd_item))
    // {
    //     out->pid_ctrl_params.kd = (float)kd_item->valuedouble;
    // }

    return;
}

esp_err_t get_data(data_params_t *value)
{

    if (value == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err;

    size_t required_size;
    err = nvs_get_str(nvs_handler, NVS_DATA_KEY, NULL, &required_size);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "KEY not found");
        return ESP_ERR_NOT_FOUND;
    }

    char *json_string = (char *)malloc(required_size);

    if (json_string == NULL)
    {
        ESP_LOGE(TAG, "NO Memory to allocate json string");
        return ESP_ERR_NO_MEM;
    }

    err = nvs_get_str(nvs_handler, NVS_DATA_KEY, json_string, &required_size);

    if (err != ESP_OK)
    {
        free(json_string);
        return ESP_ERR_NOT_FOUND;
    }

    cJSON *root = cJSON_Parse(json_string);
    free(json_string);

    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *max_temp_item = cJSON_GetObjectItem(root, "max_temp");
    cJSON *min_temp_item = cJSON_GetObjectItem(root, "min_temp");
    // cJSON *pid_params_item = cJSON_GetObjectItem(root, "pid_ctrl_params");

  

    if (max_temp_item && cJSON_IsNumber(max_temp_item))
    {
        value->max_temperature = (float)max_temp_item->valuedouble;
    }

     if (min_temp_item && cJSON_IsNumber(min_temp_item))
    {
        value->min_temperature = (float)min_temp_item->valuedouble;
    }

    // if (pid_params_item && cJSON_IsObject(pid_params_item))
    // {
    //     cJSON *kp_item = cJSON_GetObjectItem(pid_params_item, "kp");
    //     if (kp_item && cJSON_IsNumber(kp_item))
    //     {
    //         value->pid_ctrl_params.kp = (float)kp_item->valuedouble;
    //     }

    //     cJSON *ki_item = cJSON_GetObjectItem(pid_params_item, "ki");
    //     if (ki_item && cJSON_IsNumber(ki_item))
    //     {
    //         value->pid_ctrl_params.ki = (float)ki_item->valuedouble;
    //     }

    //     cJSON *kd_item = cJSON_GetObjectItem(pid_params_item, "kd");
    //     if (kd_item && cJSON_IsNumber(kd_item))
    //     {
    //         value->pid_ctrl_params.kd = (float)kd_item->valuedouble;
    //     }
    // }

    cJSON_Delete(root);

    return err;
}

esp_err_t save_data(void)
{
    esp_err_t err;

    data_params_t data_params;
    data_manager_get_params(&data_params);

    cJSON *root = cJSON_CreateObject();
    // cJSON *pid_params_json = cJSON_CreateObject();

    // cJSON_AddNumberToObject(pid_params_json, "kp", data_params.pid_ctrl_params.kp);
    // cJSON_AddNumberToObject(pid_params_json, "ki", data_params.pid_ctrl_params.ki);
    // cJSON_AddNumberToObject(pid_params_json, "kd", data_params.pid_ctrl_params.kd);

    // cJSON_AddNumberToObject(root, "is_on", data_params.is_on);
    cJSON_AddNumberToObject(root, "max_temp", data_params.max_temperature);
    cJSON_AddNumberToObject(root, "min_temp", data_params.min_temperature);
    // cJSON_AddItemToObject(root, "pid_ctrl_params", pid_params_json);

    char *json_string = cJSON_PrintUnformatted(root);

    err = nvs_set_str(nvs_handler, NVS_DATA_KEY, json_string);

    if (err == ESP_OK)
    {
        nvs_commit(nvs_handler);
    }

    cJSON_Delete(root);
    free(json_string);

    return err;
}

static void parse_data(const char *json_string, data_params_t *out)
{

    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to parse JSON data.");
        return;
    }

    cJSON *cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    cJSON *cmd_data = cJSON_GetObjectItemCaseSensitive(root, "data");

    if (cJSON_IsNumber(cmd))
    {

        switch (cmd->valueint)
        {
        case 234:
            set_data(cmd_data, out);
            break;

        case 236:
            save_data();
            break;

        default:
            ESP_LOGW(TAG, "LED Manager: INVALID COMMAND: %d", cmd->valueint);
            break;
        }
    }
    else
    {
        ESP_LOGW(TAG, "LED Manager: JSON missing CMD/Data or wrong type.");
    }

    cJSON_Delete(root);

    return;
}

void storage_manager_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Storage Manager Task started.");

    ble_data_message_t received_message;
    SemaphoreHandle_t data_ready_sem = data_manager_get_data_ready_semaphore();
    data_params_t data_params;

    esp_err_t res = get_data(&data_params);
    
    ESP_LOGI(TAG, "Max temp: %.2f", data_params.max_temperature);
    ESP_LOGI(TAG, "Min temp: %.2f", data_params.min_temperature);

    if(res == ESP_OK){
        data_manager_set_params(&data_params);
    }

    if(res != ESP_OK){
        data_manager_get_params(&data_params);
    }
    
    xSemaphoreGive(data_ready_sem);

    while (1)
    {
        if (xQueueReceive(ble_data_queue, (void *)&received_message, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "Received message from queue: %s", received_message.json_data);

            parse_data(received_message.json_data, &data_params);

            data_manager_set_params(&data_params);

            xSemaphoreGive(data_ready_sem);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to receive message from queue (should not happen with portMAX_DELAY).");
        }
    }

    vTaskDelete(NULL);
}