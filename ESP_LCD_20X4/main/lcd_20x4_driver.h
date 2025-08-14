/*======================================================================================================
File Name:	lcd_20x4_driver.h
Author:		Vamseedhar Reddy, Samip Patel
Date:		10/07/2025
Modified:	None
© Fanshawe College, 2025

Description: This file contains the interface for the LCD 20x4 driver,
including function declarations for initializing and controlling the display.
====================================================================================================*/

// lcd_20x4_driver.h
#ifndef LCD_20X4_DRIVER_H
#define LCD_20X4_DRIVER_H

#include <stdbool.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

// HD44780 commands
#define CMD_CLEAR_DISPLAY       0x01 // Clear display
#define CMD_RETURN_HOME         0x02 // Return home
#define CMD_ENTRY_MODE_SET      0x04 // Entry mode set
#define CMD_DISPLAY_CONTROL     0x08 // Display control
#define CMD_SHIFT               0x10 // Shift
#define CMD_FUNCTION_SET        0x20 // Function set
#define CMD_SET_CGRAM_ADDR      0x40 // Set CGRAM address
#define CMD_SET_DDRAM_ADDR      0x80 // Set DDRAM address

// Entry mode flags
#define ENTRY_INC       0x02 // Increment
#define ENTRY_DEC       0x00 // Decrement
#define ENTRY_SHIFT_ON  0x01 // Shift on
#define ENTRY_SHIFT_OFF 0x00 // Shift off

// Display control flags
#define DISP_ON     0x04 // Display on
#define DISP_OFF    0x00 // Display off
#define CURSOR_ON   0x02 // Cursor on
#define CURSOR_OFF  0x00 // Cursor off
#define BLINK_ON    0x01 // Blink on
#define BLINK_OFF   0x00 // Blink off

// Shift flags
#define SHIFT_DISP      0x08 // Display shift
#define SHIFT_CUR       0x00 // Cursor shift
#define SHIFT_LEFT      0x04 // Left shift
#define SHIFT_RIGHT     0x00 // Right shift

// Function set flags
#define FUNC_8BIT   0x10 // 8-bit interface
#define FUNC_4BIT   0x00 // 4-bit interface
#define FUNC_2LINE  0x08 // 2-line display
#define FUNC_1LINE  0x00 // 1-line display
#define FUNC_5x10   0x04 // 5x10 dot character font
#define FUNC_5x8    0x00 // 5x8 dot character font

// PCF8574 control bits
#define BL_BIT      0b00001000  // backlight
#define EN_BIT      0b00000100  // enable strobe
#define RW_BIT      0b00000010  // read/write
#define RS_BIT      0b00000001  // register select

// Timing (in µs)
#define LCD_DELAY_POWER_ON_US    50000 // Power on delay count
#define LCD_DELAY_INIT1_US        4500  // Initialization step 1 delay count
#define LCD_DELAY_INIT2_US        4500  // Initialization step 2 delay count
#define LCD_DELAY_INIT3_US         200  // Initialization step 3 delay count
#define LCD_DELAY_CLEAR_US       2000  // Clear display delay count
#define LCD_DELAY_HOME_US        2000  // Return home delay count
#define LCD_DELAY_ENABLE_PULSE_US   1 // Enable pulse delay count
#define LCD_DELAY_ENABLE_SETTLE_US  50 // Enable settle delay count

typedef struct {
    i2c_port_t    port;
    uint8_t       address;
    bool          backlight;
    uint8_t       rows;
    uint8_t       cols;
} lcd_20x4_driver_t; // LCD 20x4 driver structure

/**
 * @brief Initialize both the I²C bus and the LCD itself.
 * @param lcd         Pointer to driver state.
 * @param i2c_port    I2C_NUM_0 or _1
 * @param sda_gpio    GPIO_NUM_x for SDA
 * @param scl_gpio    GPIO_NUM_x for SCL
 * @param clk_speed   Bus speed in Hz (e.g. 100000)
 * @param lcd_addr    7‑bit PCF8574 address (e.g. 0x27)
 * @param backlight   true=on, false=off
 * @param rows        e.g. 4
 * @param cols        e.g. 20
 */
esp_err_t lcd20x4_init(lcd_20x4_driver_t *lcd,
                       i2c_port_t i2c_port,
                       gpio_num_t sda_gpio,
                       gpio_num_t scl_gpio,
                       uint32_t clk_speed,
                       uint8_t lcd_addr,
                       bool backlight,
                       uint8_t rows,
                       uint8_t cols);

esp_err_t lcd20x4_clear(lcd_20x4_driver_t *lcd);
esp_err_t lcd20x4_home(lcd_20x4_driver_t *lcd);

esp_err_t lcd20x4_set_entry_mode(lcd_20x4_driver_t *lcd, bool inc, bool shift);
esp_err_t lcd20x4_display_control(lcd_20x4_driver_t *lcd, bool disp, bool cursor, bool blink);
esp_err_t lcd20x4_shift(lcd_20x4_driver_t *lcd, bool display_not_cursor, bool left);
esp_err_t lcd20x4_function_set(lcd_20x4_driver_t *lcd, bool eight_bit, bool two_line, bool font_5x10);

esp_err_t lcd20x4_set_cgram_addr(lcd_20x4_driver_t *lcd, uint8_t addr);
esp_err_t lcd20x4_set_ddram_addr(lcd_20x4_driver_t *lcd, uint8_t addr);

esp_err_t lcd20x4_set_cursor(lcd_20x4_driver_t *lcd, uint8_t col, uint8_t row);
esp_err_t lcd20x4_write_char(lcd_20x4_driver_t *lcd, char c);
esp_err_t lcd20x4_write_string(lcd_20x4_driver_t *lcd, const char *str);

esp_err_t lcd20x4_set_backlight(lcd_20x4_driver_t *lcd, bool on);

#endif // LCD_20X4_DRIVER_H
