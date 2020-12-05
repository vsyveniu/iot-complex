#ifndef DEFINES_H
#define DEFINES_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "freertos/timers.h"
#include "driver/timer.h"
#include "esp_http_server.h"

#define UART_NUMBER         UART_NUM_1 //purge it after new uart RX/TX
#define UART_TX_PIN         17 //purge it after new uart RX/TX
#define UART_RX_PIN         16 //purge it after new uart RX/TX
#define WIFI_RECONNECT_MAX  8



typedef struct wifi_sta_info_t
{
    uint8_t ssid[32];
    int8_t wifi_reconnect_count;
    int8_t rssi;
    uint8_t channel;
    char* state;
    char ssid_str[33];
    char passwd[64];
    char fallback_ssid[33];
    char fallback_passwd[64];
    char* ip;
    bool is_connected;

} wifi_sta_info_s;

//purge it after new uart RX/TX
typedef struct uart_saved_input_t
{
    char* p_saved;

} uart_saved_input_s;
// end purge

QueueHandle_t wifi_info_queue;
QueueHandle_t uart_save_input_queue; //purge it after new uart RX/TX
QueueHandle_t uart_is_saved; //purge it after new uart RX/TX
xSemaphoreHandle uart_mutex_output;//purge it after new uart RX/TX
xSemaphoreHandle scan_mutex;
nvs_handle_t wifi_nvs_handle;
QueueHandle_t wifi_scan_queue;
httpd_handle_t server;

#endif