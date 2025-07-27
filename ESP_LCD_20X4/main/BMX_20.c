// BMX_20.c
#include "BMX_20.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

static const char *TAG = "BMX20";

// --- low‑level I2C helpers ----------------------------------------------------

static esp_err_t i2c_write_reg(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
      i2c_master_write_byte(cmd, reg, true);
      i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static esp_err_t i2c_read_bytes(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *buf, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // set register pointer
    i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
      i2c_master_write_byte(cmd, reg, true);
    // read data back
    i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
      if (len > 1) {
          i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
      }
      i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

// --- BMP280 init & temperature ------------------------------------------------

static esp_err_t bmp280_init(bmx20_t *dev)
{
    uint8_t calib[6];
    // Read T1,T2,T3 calibration registers (0x88..0x8D)
    esp_err_t err = i2c_read_bytes(dev->port, dev->addr_bmp280, 0x88, calib, sizeof(calib));
    if (err) {
        ESP_LOGE(TAG, "BMP280 cal read failed");
        return err;
    }
    dev->dig_T1 = (uint16_t)(calib[0] | (calib[1] << 8));
    dev->dig_T2 = (int16_t)(calib[2] | (calib[3] << 8));
    dev->dig_T3 = (int16_t)(calib[4] | (calib[5] << 8));

    // Configure: osrs_t = 1 (<<5), osrs_p = 0, mode = Normal (3)
    return i2c_write_reg(dev->port, dev->addr_bmp280, 0xF4, (1 << 5) | 3);
}

esp_err_t bmx20_read_temperature(bmx20_t *dev, float *temperature)
{
    uint8_t data[3];
    esp_err_t err = i2c_read_bytes(dev->port, dev->addr_bmp280, 0xFA, data, sizeof(data));
    if (err) return err;

    int32_t adc_T = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | (data[2] >> 4);
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dev->dig_T1 << 1))) * dev->dig_T2) >> 11;
    int32_t var2 = (((((adc_T >> 4) - dev->dig_T1) * ((adc_T >> 4) - dev->dig_T1)) >> 12) * dev->dig_T3) >> 14;
    dev->t_fine  = var1 + var2;
    int32_t T100 = (dev->t_fine * 5 + 128) >> 8;  // T * 100
    *temperature = T100 / 100.0f;
    return ESP_OK;
}

// --- AHT20 humidity ------------------------------------------------------------

static esp_err_t aht20_trigger_measure(bmx20_t *dev)
{
    const uint8_t cmd[3] = { 0xAC, 0x33, 0x00 };
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    i2c_master_start(c);
      i2c_master_write_byte(c, (dev->addr_aht20 << 1) | I2C_MASTER_WRITE, true);
      i2c_master_write(c, (uint8_t *)cmd, sizeof(cmd), true);
    i2c_master_stop(c);
    esp_err_t err = i2c_master_cmd_begin(dev->port, c, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(c);
    return err;
}

esp_err_t bmx20_read_humidity(bmx20_t *dev, float *humidity)
{
    esp_err_t err = aht20_trigger_measure(dev);
    if (err) return err;

    // Wait for measurement (~80ms)
    esp_rom_delay_us(80000);

    uint8_t buf[6];
    err = i2c_read_bytes(dev->port, dev->addr_aht20, 0x00, buf, sizeof(buf));
    if (err) return err;

    uint32_t raw_h = ((uint32_t)(buf[1] & 0x0F) << 16)
                   | ((uint32_t)buf[2] << 8)
                   |  (uint32_t)buf[3];
    *humidity = (raw_h * 100.0f) / (1 << 20);
    return ESP_OK;
}

// --- public init ---------------------------------------------------------------

esp_err_t bmx20_init(bmx20_t *dev,
                     i2c_port_t port,
                     gpio_num_t sda_gpio,
                     gpio_num_t scl_gpio,
                     uint32_t clk_speed)
{
    // CHANGE: removed bus setup here; I2C is already configured once in main.c
    dev->port        = port;
    dev->addr_aht20  = AHT20_I2C_ADDR;
    dev->addr_bmp280 = BMP280_I2C_ADDR;

    // Initialize BMP280 (reads calibration + sets control register)
    esp_err_t err = bmp280_init(dev);
    if (err) {
        ESP_LOGE(TAG, "BMP280 init failed");
        return err;
    }
    // AHT20 needs no additional init
    return ESP_OK;
}
