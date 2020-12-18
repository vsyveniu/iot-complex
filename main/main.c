
#include "main.h"
#include "esp_wifi.h"
#include "uart_console.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "esp_err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "lwip/apps/sntp.h"
#include "wifi_connection.h"

// #include "bitmap.h"
#include "esp_err.h"
#include "esp_camera.h"
// #include "movement.h"
#include "custom_http_server.h"

#define TAG ""

void app_main()
{
    xMutex = xSemaphoreCreateMutex();
    
    esp_err_t err;
    err = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM);
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    
    server = NULL;
    server2 = NULL;

    nvs_flash_init();

    uart_console_start();
    register_cmnd_set();

    gpio_config_t butt_1_conf = {
        .pin_bit_mask = GPIO_SEL_39,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&butt_1_conf);

    scan_mutex = xSemaphoreCreateMutex();

    err = wifi_init();
    if (err != ESP_OK)
    {
        printf("%s\n", "WIFI initialization failed");
    }

    wifi_register_events();

    wifi_sta_info_s wifi_sta_info = {
        .wifi_reconnect_count = 0,
        .state = "DISCONNECTED",
        .ssid = "",
        .fallback_ssid = "",
        .fallback_passwd = "",
        .channel = 0,
        .rssi = 0,
        .ip = NULL,
        .is_connected = false,
    };

    memset(wifi_sta_info.fallback_ssid, 0, 32);
    memset(wifi_sta_info.fallback_passwd, 0, 63); 
    memset(wifi_sta_info.passwd, 0, 63); 
    memset(wifi_sta_info.ssid_str, 0, 32); 
    wifi_get_nvs_data(&wifi_sta_info);

    wifi_info_queue = xQueueCreate(1, sizeof(wifi_sta_info_s));

    UBaseType_t is_filled = 0;
    is_filled = uxQueueMessagesWaiting(wifi_info_queue);

    if (!is_filled)
    {
        xQueueSend(wifi_info_queue, &wifi_sta_info, 10);
    }
    wifi_scan_queue = xQueueCreate( 1, sizeof(char) * 1024);

    if(strlen(wifi_sta_info.ssid_str) > 0) {
        wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);
    }
    else
    {
        wifi_scan_aps();
        esp_netif_create_default_wifi_ap();
        esp_wifi_set_mode(WIFI_MODE_APSTA);
         custom_http_server_init();
        
    }
   
    // hc_sr501_init();
    
    control_buttons_init();

    static camera_config_t camera_config = {
        .pin_pwdn  = -1,                        // power down is not used
        .pin_reset = 2,              // software reset will be performed
        .pin_xclk = 4,
        .pin_sscb_sda = 23,
        .pin_sscb_scl = 22,

        .pin_d7 = 14,
        .pin_d6 = 27,
        .pin_d5 = 25,
        .pin_d4 = 26,
        .pin_d3 = 32,
        .pin_d2 = 33,
        .pin_d1 = 34,
        .pin_d0 = 35,
        .pin_vsync = 19,
        .pin_href = 18,
        .pin_pclk = 5,

        //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_RGB565,
        .frame_size = FRAMESIZE_QQVGA,
        .jpeg_quality = 12, //0-63 lower number means higher quality
        .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
    };

    err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }
    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "Camera demo ready");
    printf("Error 5\n");
    //websocket_init();
    hc_sr501_init();
    control_buttons_init();
    start_webserver();
    //return err;
}


     ////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////////////////////




