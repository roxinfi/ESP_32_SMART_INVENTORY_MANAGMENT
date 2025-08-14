/*==========================================================================================
File Name:	sensors.h
Author:		Vraj Patel, Samip Patel, Mihir Jariwala, Vamseedhar Reddy
Date:		10/07/2025
Modified:	None
© Fanshawe College, 2025

Description: This file contains the definitions and function prototypes for the sensor management
system (IR, spill), including the BMX-20 sensor.
===========================================================================================*/

#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#define SHELF_SLOTS   10
#define PROX_COUNT    SHELF_SLOTS
#define SPILL_GPIO    GPIO_NUM_32

typedef struct {
    bool     prox[PROX_COUNT];
    bool     spill;
    float    temperature;
    float    humidity;
} sensor_data_t;

/**
 * @brief   Configure IR inputs, spill GPIO, and hand off to BMX20 init.
 * @return  ESP_OK or error from bmx20_init()
 */
esp_err_t sensors_init(void);

/**
 * @brief   Read all 10 IR bits, the spill bit, and temp/humidity.
 */
esp_err_t sensors_read(sensor_data_t *out);

#endif // SENSORS_H
