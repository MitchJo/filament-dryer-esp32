#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include "esp_err.h"

#define SHT4X_SDA_GPIO CONFIG_SHT4X_I2C_SDA 
#define SHT4X_SCL_GPIO CONFIG_SHT4X_I2C_SCL
#define SHT4X_CLK_SPEED CONFIG_SHT4X_I2C_CLK_SPEED_HZ 
#define SHT4X_I2C_NUM CONFIG_SHT4X_I2C_NUM

#define RELAY_PIN GPIO_NUM_17
#define RELAY_ON 1
#define RELAY_OFF 0

esp_err_t init_temperature_sensor(void);
void temperature_manager_task(void * pvParameter);

#endif