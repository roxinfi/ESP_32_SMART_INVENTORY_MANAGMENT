// BMX_20.h
#ifndef BMX_20_H
#define BMX_20_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AHT20_I2C_ADDR    0x38
#define BMP280_I2C_ADDR   0x77

typedef struct {
    i2c_port_t port;
    uint8_t    addr_aht20;
    uint8_t    addr_bmp280;
    // BMP280 calibration params:
    uint16_t   dig_T1;
    int16_t    dig_T2;
    int16_t    dig_T3;
    int32_t    t_fine;
} bmx20_t;

/**
 * @brief Initialize I2C peripheral and both sensors.
 * @param dev         Pointer to your bmx20_t struct
 * @param port        I2C_NUM_0 or I2C_NUM_1
 * @param sda_gpio    GPIO_NUM_x for SDA line
 * @param scl_gpio    GPIO_NUM_x for SCL line
 * @param clk_speed   e.g. 100000 (100 kHz)
 * @return ESP_OK on success
 */
esp_err_t bmx20_init(bmx20_t *dev,
                     i2c_port_t port,
                     gpio_num_t sda_gpio,
                     gpio_num_t scl_gpio,
                     uint32_t clk_speed);

/**
 * @brief Read temperature (°C) from BMP280
 */
esp_err_t bmx20_read_temperature(bmx20_t *dev, float *temperature);

/**
 * @brief Read relative humidity (%) from AHT20
 */
esp_err_t bmx20_read_humidity(bmx20_t *dev, float *humidity);

#ifdef __cplusplus
}
#endif

#endif // BMX_20_H
