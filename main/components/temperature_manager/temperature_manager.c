#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "sdkconfig.h"

#include "driver/gpio.h"
#include "sht4x.h"

#include "data_manager.h"
#include "temperature_manager.h"
#include "storage_manager.h"
#include "ble_manager.h"

static const char *TAG = "Temp_Manager";

static int relay_state = RELAY_OFF;

i2c_master_dev_handle_t sht4x_handle;

i2c_master_bus_handle_t i2c_bus_init(uint8_t sda_io, uint8_t scl_io)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = SHT4X_I2C_NUM,
        .sda_io_num = sda_io,
        .scl_io_num = scl_io,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
    ESP_LOGI(TAG, "I2C master bus created");

    return bus_handle;
}

esp_err_t init_temperature_sensor(void)
{

    i2c_master_bus_handle_t bus_handle = i2c_bus_init(SHT4X_SDA_GPIO, SHT4X_SCL_GPIO);
    sht4x_handle = sht4x_device_create(bus_handle, SHT4X_I2C_ADDR_0, SHT4X_CLK_SPEED);

    ESP_LOGI(TAG, "Sensor initialization success");
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);

    esp_err_t err = i2c_master_probe(bus_handle, SHT4X_I2C_ADDR_0, 200);

    return err;
}

static void control_relay(data_params_t *current_params, float temperature)
{
    ESP_LOGI(TAG, "IS_ON: %d", current_params->is_on);

    if (current_params->is_on == RELAY_OFF && relay_state == RELAY_OFF)
    {
        return;
    }

    if (current_params->is_on == RELAY_OFF)
    {
        ESP_LOGI(TAG, "BLE CMD -> RELAY OFF");
        relay_state = RELAY_OFF;
        gpio_set_level(RELAY_PIN, relay_state);
        return;
    }

    ESP_LOGI(TAG, "Checking Temperature\n %.2f | %.2f | %.2f", current_params->min_temperature, temperature, current_params->max_temperature);

    if(relay_state == RELAY_ON && temperature > current_params->max_temperature) {
        ESP_LOGI(TAG, "Exceed Temperature -> Turning OFF...");
        relay_state = RELAY_OFF;
        gpio_set_level(RELAY_PIN, relay_state);
        return;
    }


    if (relay_state == RELAY_OFF && temperature < current_params->min_temperature)
    {
        ESP_LOGI(TAG, "Below minimum temperature, Turning ON...");
        relay_state = RELAY_ON;
        gpio_set_level(RELAY_PIN, relay_state);
    }

    return;
}

void temperature_manager_task(void *pvParameters)
{
    ESP_LOGI(TAG, "TEMPERATURE Manager task started.");

    data_params_t current_params;

    SemaphoreHandle_t data_ready_sem = data_manager_get_data_ready_semaphore();

    TickType_t last_wakeup = xTaskGetTickCount();
    float temperature, humidity;

    while (1)
    {

        if (xSemaphoreTake(data_ready_sem, 0) == pdTRUE)
        {
            data_manager_get_params(&current_params);
        }

        esp_err_t err = sht4x_start_measurement(sht4x_handle, SHT4X_CMD_READ_MEASUREMENT_HIGH);
        vTaskDelay(pdMS_TO_TICKS(50));

        err = sht4x_read_measurement(sht4x_handle, &temperature, &humidity);

        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);

            // if(relay_state != current_params.is_on){
            control_relay(&current_params, temperature);
            // }

            float notify_data[3] = {temperature, humidity, (float) relay_state };
            send_notification(PROFILE_APP_ID, notify_data, 3);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read temperature and humidity");
        }

        xTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Temperature task stopped");

    sht4x_device_delete(sht4x_handle);

    vTaskDelete(NULL);
}