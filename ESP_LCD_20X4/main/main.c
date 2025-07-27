// main.c — ESP_LCD_20X4 (Primary Controller)

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/sockets.h"

#include "item_sorting.h"
#include "lcd_20x4_driver.h"
#include "shelf_manager.h"

static const char *TAG      = "BARCODE_TEST";
static const char *TAG_SENS = "SENSOR_LISTENER";

#define AP_SSID     "ESPBarTest"
#define AP_PASS     "test1234"
#define TCP_PORT    3334   // barcode scans
#define SENS_PORT   3333   // occupancy + T/H + spill

// Buttons
#define SW1_GPIO    GPIO_NUM_2    // toggle scan mode
#define SW2_GPIO    GPIO_NUM_5    // re‑display last scan

// LED indicators
#define LED_TEMP_GPIO   GPIO_NUM_12
#define LED_HUM_GPIO    GPIO_NUM_13
#define LED_SPILL_GPIO  GPIO_NUM_27

// Thresholds (your choice)
#define TEMP_LIMIT      25.0f  // °C
#define HUM_LIMIT       90.0f  // %

#define DEGREE_SYMBOL   0xDF   // custom ° character code for LCD

// Your LG_spill slot lives at index 9 (0–9 total slots)
#define LG_SPILL_INDEX 9

static item_info_t  s_last_info;
static char         s_last_code[16];
static bool         s_has_last   = false;
static int          s_last_slot  = -1;
static float        s_temp       = 0.0f;
static float        s_hum        = 0.0f;
static bool         s_spill      = false;

// ─── Helpers ─────────────────────────────────────────────────────────────────
static bool is_digits(const char *s) 
{
    for (; *s; ++s) if (*s < '0' || *s > '9') return false;
    return true;
}
static item_size_t  decode_size(char d)  
{
    if      (d >= '0' && d <= '2') return SIZE_SMALL;
    else if (d >= '3' && d <= '6') return SIZE_MEDIUM;
    else                            return SIZE_LARGE;
}
static item_type_t  decode_type(char d)  
{
    return (d=='0') ? TYPE_FROZEN : TYPE_DRY;
}
static item_phase_t decode_phase(char d) 
{
    return (d <= '5') ? PHASE_LIQUID : PHASE_SOLID;
}

// ─── Wi‑Fi SoftAP ─────────────────────────────────────────────────────────────
static void wifi_init_softap(void) // Initialize Wi‑Fi in SoftAP mode 
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t apcfg = 
    {
        .ap = {
            .ssid_len      = strlen(AP_SSID),
            .password      = AP_PASS,
            .max_connection= 4,
            .authmode      = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    memcpy(apcfg.ap.ssid, AP_SSID, strlen(AP_SSID));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apcfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "SoftAP started. SSID:%s PW:%s", AP_SSID, AP_PASS);
}

// ─── LCD, Buttons & LEDs ──────────────────────────────────────────────────────
static void peripherals_init(lcd_20x4_driver_t *lcd) {
    // LCD
    ESP_ERROR_CHECK(lcd20x4_init(lcd,I2C_NUM_0,GPIO_NUM_21, GPIO_NUM_22, 100000, 0x27, true, 4, 20));
    lcd20x4_clear(lcd);
    lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Barcode Sorting");
    lcd20x4_set_cursor(lcd,0,1); lcd20x4_write_string(lcd,"Waiting for scan:");

    // Buttons
    gpio_config_t btn = 
    {
        .pin_bit_mask = (1ULL<<SW1_GPIO)|(1ULL<<SW2_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&btn);

    // LEDs
    gpio_config_t led_cfg = 
    {
        .pin_bit_mask = (1ULL<<LED_TEMP_GPIO)|(1ULL<<LED_HUM_GPIO)|(1ULL<<LED_SPILL_GPIO),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&led_cfg);
    gpio_set_level(LED_TEMP_GPIO,  0);
    gpio_set_level(LED_HUM_GPIO,   0);
    gpio_set_level(LED_SPILL_GPIO, 0);
}

// ─── Scan Task ─────────────────────────────────────────────────────────────────
static void scan_task(void *arg) 
{
    lcd_20x4_driver_t *lcd = arg;
    struct sockaddr_in addr = 
    {
        .sin_family      = AF_INET,
        .sin_port        = htons(TCP_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    int ls = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    bind(ls, (struct sockaddr*)&addr, sizeof(addr));
    listen(ls, 1);

    bool scanning = false;
    for (;;) // similar to while(1) but can be changed later according to needs 
    {
        // ── Idle: show live T/H ─────────────────────────
        if (!scanning)
        {
            char l2[21], l3[21];
            snprintf(l2, sizeof(l2), "Temp: %.1f%cC", s_temp, DEGREE_SYMBOL); // snprintf is used for safe string formatting to avoid buffer overflows
            snprintf(l3, sizeof(l3), "Hum:  %.1f %%",    s_hum);
            lcd20x4_set_cursor(lcd,0,2); 
            lcd20x4_write_string(lcd,l2);
            lcd20x4_set_cursor(lcd,0,3); 
            lcd20x4_write_string(lcd,l3);
            vTaskDelay(pdMS_TO_TICKS(1000)); // delay for 1 second
        }

        // ── SW1 toggles scan mode ────────────────────────
        if (gpio_get_level(SW1_GPIO)==0) 
        {
            scanning = !scanning;
            ESP_LOGI(TAG,"SW1 → scanning=%d", scanning);
            while (gpio_get_level(SW1_GPIO)==0) vTaskDelay(10);
            lcd20x4_clear(lcd);
            if (scanning) 
            {
                lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Scan mode:");
                lcd20x4_set_cursor(lcd,0,1); lcd20x4_write_string(lcd,"Waiting for TCP");
            } else 
            {
                lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Aborted scan");
                vTaskDelay(pdMS_TO_TICKS(1000));
                lcd20x4_clear(lcd);
                lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Barcode Sorting");
                lcd20x4_set_cursor(lcd,0,1); lcd20x4_write_string(lcd,"Waiting for scan:");
            }
        }
        if (!scanning) 
        { 
            vTaskDelay(pdMS_TO_TICKS(50)); continue; 
        }

        // ── Accept one barcode ───────────────────────────
        int acceptbar = accept(ls, NULL, NULL);
        if (acceptbar<0) 
        { 
            vTaskDelay(pdMS_TO_TICKS(50)); 
            continue; 
        }

        char buf[64]; 
        int byterecieve = recv(acceptbar,buf,sizeof(buf)-1,0); // here the 'n' variable is used to store the number of bytes received
        if (byterecieve>0) 
        {
            buf[byterecieve]='\0';
            while(byterecieve>0&&(buf[byterecieve-1]=='\r'||buf[byterecieve-1]=='\n')) buf[--byterecieve]='\0';
            ESP_LOGI(TAG,"Received '%s'", buf);

            item_info_t info;
            bool ok = item_sorting_parse(buf,&info);
            if (!ok && byterecieve==8 && is_digits(buf)) 
            {
                info.size  = decode_size (buf[0]);
                info.type  = decode_type(buf[1]);
                info.phase = decode_phase(buf[2]);
                ok = true;
            }

            if (ok) 
            {
                // save last
                strncpy(s_last_code,buf,sizeof(s_last_code));
                s_last_info = info;  s_has_last = true;

                // decide slot
                int slot = -1;
                if (info.type==TYPE_FROZEN) 
                {
                    slot = -2;  // Frozen Section
                }
                else if (info.phase==PHASE_LIQUID) 
                {
                    // always LG_spill at index 9
                    if (!shelf_manager_is_slot_occupied(LG_SPILL_INDEX)) 
                    {
                        slot = LG_SPILL_INDEX;
                        shelf_manager_mark_occupied(LG_SPILL_INDEX);
                    }
                }
                else 
                {
                    slot = shelf_manager_find_slot(&info);
                    if (slot>=0) shelf_manager_mark_occupied(slot);
                }
                s_last_slot = slot;

                // draw
                lcd20x4_clear(lcd);
                lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,buf);
                char line1[21];
                snprintf(line1, sizeof(line1), "%s/%s/%s", item_sorting_size_string(info.size), item_sorting_type_string(info.type), item_sorting_phase_string(info.phase));
                lcd20x4_set_cursor(lcd,0,1); 
                lcd20x4_write_string(lcd,line1);

                char line2[21];
                if (slot == -2) 
                {
                    snprintf(line2, sizeof(line2), "To: Frozen Section");
                } 
                else if (slot == LG_SPILL_INDEX) 
                {
                    snprintf(line2, sizeof(line2), "To:Liquid_Section");
                } else if (slot >= 0) 
                {
                    snprintf(line2, sizeof(line2), "Slot: %s",
                            shelf_manager_slot_string(slot));
                } 
                else 
                {
                    if (info.phase == PHASE_LIQUID) 
                    {
                        snprintf(line2, sizeof(line2), "Liquid Section FULL");
                    } else 
                    {
                        snprintf(line2, sizeof(line2), "%s FULL",
                                item_sorting_size_string(info.size));
                    }
                }
                lcd20x4_set_cursor(lcd, 0, 2);
                lcd20x4_write_string(lcd, line2);

            }

            else 
            {
                lcd20x4_clear(lcd);
                lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Invalid barcode");
            }
        }

        shutdown(acceptbar,0); close(acceptbar);
        scanning = false;
        vTaskDelay(pdMS_TO_TICKS(1500));
        lcd20x4_clear(lcd);
        lcd20x4_set_cursor(lcd,0,0); lcd20x4_write_string(lcd,"Barcode Sorting");
        lcd20x4_set_cursor(lcd,0,1); lcd20x4_write_string(lcd,"Waiting for scan:");
    }
}

// ─── Sensor Task ──────────────────────────────────────────────────────────────
static void sensor_task(void *arg) 
{
    (void)arg;
    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(SENS_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };

    int ls = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // here the 'ls' variable is used to store the socket file descriptor
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    bind(ls, (struct sockaddr*)&addr, sizeof(addr));
    listen(ls, 1);
    ESP_LOGI(TAG_SENS, "Listening on port %d", SENS_PORT);

    for (;;) 
    {
        int c = accept(ls, NULL, NULL); // here the 'c' variable is used to store the client socket file descriptor
        if (c<0) 
        { 
            vTaskDelay(pdMS_TO_TICKS(100)); continue; 
        }

        char buf[128];
        int len = recv(c, buf, sizeof(buf)-1, 0);
        if (len>0) 
        {
            buf[len]='\0';

            // 1) occupancy
            bool ir[SHELF_SLOTS];
            char *tok = strtok(buf, ",");
            for (int i=0;i<SHELF_SLOTS && tok;++i) 
            {
                ir[i] = (tok[0]=='1');
                tok   = strtok(NULL,",");
            }
            shelf_manager_update_from_sensors(ir);

            // 2) spill
            if (tok) 
            {
                s_spill = (tok[0]=='1');
                gpio_set_level(LED_SPILL_GPIO, s_spill);
                tok = strtok(NULL,",");
            }

            // 3) temp
            float t=0,h=0;
            if (tok) 
            {
                t = strtof(tok, &tok);
            }
            // 4) hum
            if (tok) 
            {
                h = strtof(tok+1, NULL);
            }
            s_temp = t;  s_hum = h;

            // LEDs
            gpio_set_level(LED_TEMP_GPIO, (t>TEMP_LIMIT));
            gpio_set_level(LED_HUM_GPIO,  (h> HUM_LIMIT));

            ESP_LOGI(TAG_SENS,"Updated occ + spill=%d + T=%.1f°C H=%.1f%%", s_spill, t, h);
        }
        shutdown(c,0); close(c);
    }
}

void app_main(void) 
{
    shelf_manager_init();
    wifi_init_softap();
    static lcd_20x4_driver_t lcd;
    peripherals_init(&lcd);

    xTaskCreate(scan_task,   "scan",   4096, &lcd,  5, NULL);
    xTaskCreate(sensor_task, "sensor", 4096, NULL, 5, NULL);
}
