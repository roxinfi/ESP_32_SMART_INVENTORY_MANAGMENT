// lcd_20x4_driver.c

#include "lcd_20x4_driver.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_rom_sys.h"

// Low‐level I²C write
static esp_err_t _i2c_write_byte(lcd_20x4_driver_t *lcd, uint8_t data) {
    return i2c_master_write_to_device(lcd->port, lcd->address, &data, 1, pdMS_TO_TICKS(100));
}

// Pulse the enable bit
static esp_err_t _strobe(lcd_20x4_driver_t *lcd, uint8_t data) {
    // EN_BIT high
    _i2c_write_byte(lcd, data | EN_BIT | (lcd->backlight ? BL_BIT : 0));
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE_US);
    // EN_BIT low
    _i2c_write_byte(lcd, data           | (lcd->backlight ? BL_BIT : 0));
    esp_rom_delay_us(LCD_DELAY_ENABLE_SETTLE_US);
    return ESP_OK;
}

// Write 4 bits + RS + backlight
static esp_err_t _write_nibble(lcd_20x4_driver_t *lcd, uint8_t nibble, bool rs) {
    uint8_t cmd = (nibble & 0x0F) << 4;
    if (rs)            cmd |= RS_BIT;
    if (lcd->backlight) cmd |= BL_BIT;
    _i2c_write_byte(lcd, cmd);
    return _strobe(lcd, cmd);
}

// Write full byte as two nibbles
static esp_err_t _write_byte(lcd_20x4_driver_t *lcd, uint8_t val, bool rs) {
    _write_nibble(lcd, val >> 4, rs);
    return _write_nibble(lcd, val & 0x0F, rs);
}

// Public API

esp_err_t lcd20x4_init(lcd_20x4_driver_t *lcd,
                       i2c_port_t i2c_port,
                       gpio_num_t sda_gpio,
                       gpio_num_t scl_gpio,
                       uint32_t clk_speed,
                       uint8_t lcd_addr,
                       bool backlight,
                       uint8_t rows,
                       uint8_t cols)
{
    lcd->port      = i2c_port;
    lcd->address   = lcd_addr;
    lcd->backlight = backlight;
    lcd->rows      = rows;
    lcd->cols      = cols;

    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .master.clk_speed = clk_speed,
    };
    i2c_param_config(i2c_port, &cfg);
    i2c_driver_install(i2c_port, cfg.mode, 0, 0, 0);

    esp_rom_delay_us(LCD_DELAY_POWER_ON_US);

    // 4‑bit init sequence
    _write_nibble(lcd, 0x03, false);
    esp_rom_delay_us(LCD_DELAY_INIT1_US);
    _write_nibble(lcd, 0x03, false);
    esp_rom_delay_us(LCD_DELAY_INIT2_US);
    _write_nibble(lcd, 0x03, false);
    esp_rom_delay_us(LCD_DELAY_INIT3_US);
    _write_nibble(lcd, 0x02, false);

    // configure display
    lcd20x4_function_set      (lcd, false, true, false);
    lcd20x4_display_control   (lcd, false, false, false);
    lcd20x4_clear             (lcd);
    lcd20x4_set_entry_mode    (lcd, true, false);
    lcd20x4_display_control   (lcd, true, false, false);

    return ESP_OK;
}

esp_err_t lcd20x4_clear(lcd_20x4_driver_t *lcd) {
    esp_err_t r = _write_byte(lcd, CMD_CLEAR_DISPLAY, false);
    esp_rom_delay_us(LCD_DELAY_CLEAR_US);
    return r;
}

esp_err_t lcd20x4_home(lcd_20x4_driver_t *lcd) {
    esp_err_t r = _write_byte(lcd, CMD_RETURN_HOME, false);
    esp_rom_delay_us(LCD_DELAY_HOME_US);
    return r;
}

esp_err_t lcd20x4_set_entry_mode(lcd_20x4_driver_t *lcd, bool inc, bool shift) {
    uint8_t cmd = CMD_ENTRY_MODE_SET
                | (inc   ? ENTRY_INC       : ENTRY_DEC)
                | (shift ? ENTRY_SHIFT_ON  : ENTRY_SHIFT_OFF);
    return _write_byte(lcd, cmd, false);
}

esp_err_t lcd20x4_display_control(lcd_20x4_driver_t *lcd, bool disp, bool cursor, bool blink) {
    uint8_t cmd = CMD_DISPLAY_CONTROL
                | (disp   ? DISP_ON    : DISP_OFF)
                | (cursor ? CURSOR_ON  : CURSOR_OFF)
                | (blink  ? BLINK_ON   : BLINK_OFF);
    return _write_byte(lcd, cmd, false);
}

esp_err_t lcd20x4_shift(lcd_20x4_driver_t *lcd, bool display_not_cursor, bool left) {
    uint8_t cmd = CMD_SHIFT
                | (display_not_cursor ? SHIFT_DISP : SHIFT_CUR)
                | (left                ? SHIFT_LEFT : SHIFT_RIGHT);
    return _write_byte(lcd, cmd, false);
}

esp_err_t lcd20x4_function_set(lcd_20x4_driver_t *lcd, bool eight_bit, bool two_line, bool font_5x10) {
    uint8_t cmd = CMD_FUNCTION_SET
                | (eight_bit ? FUNC_8BIT  : FUNC_4BIT)
                | (two_line  ? FUNC_2LINE : FUNC_1LINE)
                | (font_5x10 ? FUNC_5x10  : FUNC_5x8);
    return _write_byte(lcd, cmd, false);
}

esp_err_t lcd20x4_set_cgram_addr(lcd_20x4_driver_t *lcd, uint8_t addr) {
    return _write_byte(lcd, CMD_SET_CGRAM_ADDR | (addr & 0x3F), false);
}

esp_err_t lcd20x4_set_ddram_addr(lcd_20x4_driver_t *lcd, uint8_t addr) {
    return _write_byte(lcd, CMD_SET_DDRAM_ADDR | (addr & 0x7F), false);
}

static const uint8_t _row_offsets[4] = { 0x00, 0x40, 0x14, 0x54 };
esp_err_t lcd20x4_set_cursor(lcd_20x4_driver_t *lcd, uint8_t col, uint8_t row) {
    if (row >= lcd->rows) row = lcd->rows - 1;
    return lcd20x4_set_ddram_addr(lcd, col + _row_offsets[row]);
}

esp_err_t lcd20x4_write_char(lcd_20x4_driver_t *lcd, char c) {
    return _write_byte(lcd, (uint8_t)c, true);
}

esp_err_t lcd20x4_write_string(lcd_20x4_driver_t *lcd, const char *str) {
    esp_err_t err = ESP_OK;
    while (*str && err == ESP_OK) {
        err = lcd20x4_write_char(lcd, *str++);
    }
    return err;
}

esp_err_t lcd20x4_set_backlight(lcd_20x4_driver_t *lcd, bool on) {
    lcd->backlight = on;
    // to actually update the expander, you could re-send the last nibble, etc.
    return ESP_OK;
}
