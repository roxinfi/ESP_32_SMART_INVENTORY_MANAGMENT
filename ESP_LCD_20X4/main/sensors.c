#include "sensors.h"
#include "BMX_20.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "SENSORS";

// IR sensors: small[0..2], medium[3..5], large[6..8], liquid bin[9]
static const gpio_num_t ir_gpio[SHELF_SLOTS] = {
    GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14,   // small
    GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17,   // medium
    GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_23,   // large
    GPIO_NUM_25                              // large+liquid
};

static bmx20_t s_bmx20_dev;

esp_err_t sensors_init(void)
{
    // 1) Configure IR inputs (active‑high with pull‑up)
    gpio_config_t io_conf = {
        .mode           = GPIO_MODE_INPUT,
        .pull_up_en     = GPIO_PULLUP_ENABLE,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .intr_type      = GPIO_INTR_DISABLE,
    };
    for (int i = 0; i < SHELF_SLOTS; i++) {
        io_conf.pin_bit_mask = 1ULL << ir_gpio[i];
        gpio_config(&io_conf);
    }

    // 2) Spill detector (also active‑high)
    io_conf.pin_bit_mask = 1ULL << SPILL_GPIO;
    gpio_config(&io_conf);

    // 3) Initialize BMX20 (assumes I²C already set up in main)
    esp_err_t err = bmx20_init(&s_bmx20_dev, I2C_NUM_0,
                               GPIO_NUM_21, GPIO_NUM_22, 100000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "BMX20 init failed: %s", esp_err_to_name(err));
    }
    return err;
}

static void _read_all_ir(bool out_ir[SHELF_SLOTS])
{
    for (int i = 0; i < SHELF_SLOTS; i++) {
        // HIGH = occupied
        out_ir[i] = (gpio_get_level(ir_gpio[i]) == 1);
    }
}

static bool _read_spill(void)
{
    return (gpio_get_level(SPILL_GPIO) == 1);
}

esp_err_t sensors_read(sensor_data_t *out)
{
    if (!out) {
        return ESP_ERR_INVALID_ARG;
    }
    _read_all_ir(out->prox);
    out->spill = _read_spill();

    esp_err_t err = bmx20_read_temperature(&s_bmx20_dev, &out->temperature);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Temp read failed: %s", esp_err_to_name(err));
        return err;
    }
    err = bmx20_read_humidity(&s_bmx20_dev, &out->humidity);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Humidity read failed: %s", esp_err_to_name(err));
    }
    return err;
}
