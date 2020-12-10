
#include "main.h"
#include "esp_wifi.h"
#include "uart_console.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "lwip/apps/sntp.h"
#include "wifi_connection.h"
#include "esp_err.h"
#include "custom_http_server.h"


void app_main()
{
    esp_err_t err;

    nvs_flash_init();

    uart_console_init();
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

    wifi_connect("ucode student", ">#ucodeworld");

   /*  if(strlen(wifi_sta_info.ssid_str) > 0)
    {
        wifi_connect(wifi_sta_info.ssid_str, wifi_sta_info.passwd);
    }
    else
    {
        wifi_scan_aps();
        esp_netif_create_default_wifi_ap();
        esp_wifi_set_mode(WIFI_MODE_APSTA);
        custom_http_server_init();
    } */

    gpio_install_isr_service(0);

    //hc_sr501_init();

    //control_buttons_init();

    //esp_now_install();
    //websocket_init();
    

    while (true)
    {
       // printf("%d\n", gpio_get_level(HC_SR501));
        //printf("%d\n", gpio_get_level(FACTORY_BUTTON));
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
