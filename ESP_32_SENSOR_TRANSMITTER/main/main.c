// main.c  —— Secondary ESP32: reads IR/spill + T/H, sends to primary over TCP

#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"

// I2C + GPIO for sensors
#include "driver/i2c.h"
#include "driver/gpio.h"

// your sensor abstraction (IR, spill, BMX20)
#include "sensors.h"

static const char *TAG       = "SECONDARY";
static const char *TAG_WIFI  = "WIFI";

#define AP_SSID       "ESPBarTest"
#define AP_PASS       "test1234"
#define PRIMARY_IP    "192.168.4.1"
#define PRIMARY_PORT  3333

// — Wi‑Fi Station setup —  
static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wcfg = {
        .sta = {
            .ssid            = AP_SSID,
            .password        = AP_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = { .capable = true, .required = false },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "Connecting to AP \"%s\"…", AP_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}

// — Send task: read sensors, send CSV over TCP —  
static void send_task(void *arg)
{
    sensor_data_t d;
    char msg[128];
    struct sockaddr_in dest = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PRIMARY_PORT),
        .sin_addr.s_addr = inet_addr(PRIMARY_IP)
    };

    for (;;) {
        // 1) Sample all sensors
        if (sensors_read(&d) != ESP_OK) {
            ESP_LOGW(TAG, "sensors_read() failed");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // 2) Build CSV: P0..P9,SPILL,TEMP,HUM\n
        int off = 0;
        for (int i = 0; i < PROX_COUNT; i++) {
            off += snprintf(msg + off, sizeof(msg) - off, "%d,", d.prox[i] ? 1 : 0);
        }
        off += snprintf(msg + off, sizeof(msg) - off, "%d,", d.spill ? 1 : 0);
        off += snprintf(msg + off, sizeof(msg) - off, "%.2f,%.2f\n",
                        d.temperature, d.humidity);

        // 3) Open socket, connect, send
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket() errno %d", errno);
        } else if (connect(sock, (struct sockaddr*)&dest, sizeof(dest)) != 0) {
            ESP_LOGE(TAG, "connect() errno %d", errno);
        } else {
            int len = strlen(msg);
            if (write(sock, msg, len) != len) {
                ESP_LOGE(TAG, "send() errno %d", errno);
            } else {
                ESP_LOGI(TAG, "Sent: %s", msg);
            }
        }
        if (sock >= 0) {
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    // 1) Configure I2C bus for BMX20 (AHT20 + BMP280)
    i2c_config_t i2c_conf = {
        .mode               = I2C_MODE_MASTER,
        .sda_io_num         = GPIO_NUM_21,
        .scl_io_num         = GPIO_NUM_22,
        .master.clk_speed   = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0));

    // 2) Initialize all sensors (IR, spill, BMX20)
    if (sensors_init() != ESP_OK) {
        ESP_LOGW(TAG, "sensors_init() failed, continuing anyway");
    }

    // 3) Bring up Wi‑Fi in STA mode
    wifi_init_sta();

    // 4) Start the send task
    xTaskCreate(send_task, "send_task", 4096, NULL, 5, NULL);
}
