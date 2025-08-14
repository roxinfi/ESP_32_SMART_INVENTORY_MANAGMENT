/*=================================================================================================
File Name:	lcd_20x4_driver.c
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
© Fanshawe College, 2025

Description: This file contains the implementation of the LCD 20x4 driver,
including functions for initializing and controlling the display.
=================================================================================================*/

// lcd_20x4_driver.c

#include "lcd_20x4_driver.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_rom_sys.h"

// Low‐level I²C write

/*>>> _i2c_write_byte: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will write a single byte to the I²C device.
Input: 		- lcd: Pointer to the LCD driver structure
			- data: The byte data to write
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
static esp_err_t _i2c_write_byte(lcd_20x4_driver_t *lcd, uint8_t data) {
    return i2c_master_write_to_device(lcd->port, lcd->address, &data, 1, pdMS_TO_TICKS(100));
}// eo _i2c_write_byte::

// Pulse the enable bit
/*>>> _strobe: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will pulse the enable pin on the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- data: The data to send
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
static esp_err_t _strobe(lcd_20x4_driver_t *lcd, uint8_t data) {
    // EN_BIT high
    _i2c_write_byte(lcd, data | EN_BIT | (lcd->backlight ? BL_BIT : 0));
    esp_rom_delay_us(LCD_DELAY_ENABLE_PULSE_US);
    // EN_BIT low
    _i2c_write_byte(lcd, data           | (lcd->backlight ? BL_BIT : 0));
    esp_rom_delay_us(LCD_DELAY_ENABLE_SETTLE_US);
    return ESP_OK;
}// eo _strobe::

// Write 4 bits + RS + backlight
/*>>> _write_nibble: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will write a 4-bit nibble to the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- nibble: The 4-bit nibble to write
			- rs: Register select flag (true for data, false for command)
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
static esp_err_t _write_nibble(lcd_20x4_driver_t *lcd, uint8_t nibble, bool rs) {
    uint8_t cmd = (nibble & 0x0F) << 4;
    if (rs)            cmd |= RS_BIT;
    if (lcd->backlight) cmd |= BL_BIT;
    _i2c_write_byte(lcd, cmd);
    return _strobe(lcd, cmd);
}// eo _write_nibble::

// Write full byte as two nibbles
/*>>> _write_byte: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will write a full byte to the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- val: The byte value to write
			- rs: Register select flag (true for data, false for command)
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
static esp_err_t _write_byte(lcd_20x4_driver_t *lcd, uint8_t val, bool rs) {
    _write_nibble(lcd, val >> 4, rs);
    return _write_nibble(lcd, val & 0x0F, rs);
}

// Public API

/*>>> lcd20x4_init: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will initialize the LCD driver.
Input: 		- lcd: Pointer to the LCD driver structure
			- i2c_port: I²C port number
			- sda_gpio: SDA GPIO pin number
			- scl_gpio: SCL GPIO pin number
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
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


/*>>> lcd20x4_clear: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will clear the LCD display.
Input: 		- lcd: Pointer to the LCD driver structure
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_clear(lcd_20x4_driver_t *lcd) {
    esp_err_t r = _write_byte(lcd, CMD_CLEAR_DISPLAY, false);
    esp_rom_delay_us(LCD_DELAY_CLEAR_US);
    return r;
}// eo lcd20x4_clear::

/*>>> lcd20x4_home: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will return the cursor to the home position.
Input: 		- lcd: Pointer to the LCD driver structure
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_home(lcd_20x4_driver_t *lcd) {
    esp_err_t r = _write_byte(lcd, CMD_RETURN_HOME, false);
    esp_rom_delay_us(LCD_DELAY_HOME_US);
    return r;
}// eo lcd20x4_home::


/*>>> lcd20x4_set_entry_mode: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the entry mode for the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- inc: True for incrementing the cursor, false for decrementing
			- shift: True for shifting the display, false for no shift
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_set_entry_mode(lcd_20x4_driver_t *lcd, bool inc, bool shift) {
    uint8_t cmd = CMD_ENTRY_MODE_SET
                | (inc   ? ENTRY_INC       : ENTRY_DEC)
                | (shift ? ENTRY_SHIFT_ON  : ENTRY_SHIFT_OFF);
    return _write_byte(lcd, cmd, false);
}// eo lcd20x4_set_entry_mode::


/*>>> lcd20x4_display_control: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will control the display settings for the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- disp: True to turn on the display, false to turn it off
			- cursor: True to enable the cursor, false to disable it
			- blink: True to enable blinking, false to disable it
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_display_control(lcd_20x4_driver_t *lcd, bool disp, bool cursor, bool blink) {
    uint8_t cmd = CMD_DISPLAY_CONTROL
                | (disp   ? DISP_ON    : DISP_OFF)
                | (cursor ? CURSOR_ON  : CURSOR_OFF)
                | (blink  ? BLINK_ON   : BLINK_OFF);
    return _write_byte(lcd, cmd, false);
}// eo lcd20x4_display_control::

/*>>> lcd20x4_shift: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will shift the display or cursor.
Input: 		- lcd: Pointer to the LCD driver structure
			- display_not_cursor: True to shift the display, false to shift the cursor
			- left: True to shift left, false to shift right
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_shift(lcd_20x4_driver_t *lcd, bool display_not_cursor, bool left) {
    uint8_t cmd = CMD_SHIFT
                | (display_not_cursor ? SHIFT_DISP : SHIFT_CUR)
                | (left                ? SHIFT_LEFT : SHIFT_RIGHT);
    return _write_byte(lcd, cmd, false);
}

/*>>> lcd20x4_function_set: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the function control for the LCD, including the interface data length, number of display lines, and font size.
Input: 		- lcd: Pointer to the LCD driver structure
			- eight_bit: True for 8-bit mode, false for 4-bit mode
			- two_line: True for 2-line mode, false for 1-line mode
			- font_5x10: True for 5x10 dot font, false for 5x8 dot font
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_function_set(lcd_20x4_driver_t *lcd, bool eight_bit, bool two_line, bool font_5x10) {
    uint8_t cmd = CMD_FUNCTION_SET
                | (eight_bit ? FUNC_8BIT  : FUNC_4BIT)
                | (two_line  ? FUNC_2LINE : FUNC_1LINE)
                | (font_5x10 ? FUNC_5x10  : FUNC_5x8);
    return _write_byte(lcd, cmd, false);
}// eo lcd20x4_function_set::

/*>>> lcd20x4_set_cgram_addr: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the CGRAM address for the LCD, used for custom character generation, specifically for defining new characters.
Input: 		- lcd: Pointer to the LCD driver structure
			- addr: The CGRAM address to set (0-63)
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_set_cgram_addr(lcd_20x4_driver_t *lcd, uint8_t addr) {
    return _write_byte(lcd, CMD_SET_CGRAM_ADDR | (addr & 0x3F), false);
}// eo lcd20x4_set_cgram_addr::

/*>>> lcd20x4_set_ddram_addr: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the DDRAM address for the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- addr: The DDRAM address to set (0-127)
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_set_ddram_addr(lcd_20x4_driver_t *lcd, uint8_t addr) {
    return _write_byte(lcd, CMD_SET_DDRAM_ADDR | (addr & 0x7F), false);
}// eo lcd20x4_set_ddram_addr::

/*>>> lcd20x4_set_cursor: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the cursor position for the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- col: The column to set (0-19)
			- row: The row to set (0-3)
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
static const uint8_t _row_offsets[4] = { 0x00, 0x40, 0x14, 0x54 };
esp_err_t lcd20x4_set_cursor(lcd_20x4_driver_t *lcd, uint8_t col, uint8_t row) {
    if (row >= lcd->rows) row = lcd->rows - 1;
    return lcd20x4_set_ddram_addr(lcd, col + _row_offsets[row]);
}// eo lcd20x4_set_cursor::

/*>>> lcd20x4_write_char: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will write a single character to the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- c: The character to write
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_write_char(lcd_20x4_driver_t *lcd, char c) {
    return _write_byte(lcd, (uint8_t)c, true);
}// eo lcd20x4_write_char::

/*>>> lcd20x4_write_string: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will write a string to the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- str: The string to write
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_write_string(lcd_20x4_driver_t *lcd, const char *str) {
    esp_err_t err = ESP_OK;
    while (*str && err == ESP_OK) {
        err = lcd20x4_write_char(lcd, *str++);
    }
    return err;
}// eo lcd20x4_write_string::

/*>>> lcd20x4_set_backlight: ==========================================================
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
Desc:		This function will set the backlight state for the LCD.
Input: 		- lcd: Pointer to the LCD driver structure
			- on: True to turn on the backlight, false to turn it off
Returns:	ESP_OK on success, or an error code on failure.
 ============================================================================*/
esp_err_t lcd20x4_set_backlight(lcd_20x4_driver_t *lcd, bool on) {
    lcd->backlight = on;
    // to actually update the expander, you could re-send the last nibble, etc.
    return ESP_OK;
}// eo lcd20x4_set_backlight::
