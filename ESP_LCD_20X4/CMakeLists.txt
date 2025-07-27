# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html

# 1) Must be at least 3.16 for IDF v5
cmake_minimum_required(VERSION 3.16)

# 2) Include the IDF project helper
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 3) Then name your project
project(ESP_LCD_20X4)
