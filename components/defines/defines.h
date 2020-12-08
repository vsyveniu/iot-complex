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
#include "driver/gpio.h"
#include "esp_now.h"

#define UART_NUMBER         UART_NUM_1 //purge it after new uart RX/TX
#define UART_TX_PIN         17 //purge it after new uart RX/TX
#define UART_RX_PIN         16 //purge it after new uart RX/TX
#define WIFI_RECONNECT_MAX  16

#define HC_SR501            GPIO_NUM_12
#define LED_R               GPIO_NUM_16
#define LED_G               GPIO_NUM_17
#define LED_B               GPIO_NUM_21
#define PHOTORESISTOR       GPIO_NUM_34
#define UART_RX             GPIO_NUM_17
#define UART_TX             GPIO_NUM_16
#define FACTORY_BUTTON      GPIO_NUM_39

#define I2C_SDA             GPIO_NUM_23
#define I2C_SCL             GPIO_NUM_22
#define XCLCK               GPIO_NUM_4
#define PCLCK               GPIO_NUM_5
#define VSYNC               GPIO_NUM_19
#define HREF                GPIO_NUM_18
#define CAM_RESET           GPIO_NUM_2

#define D0                  GPIO_NUM_34
#define D1                  GPIO_NUM_35
#define D2                  GPIO_NUM_32
#define D3                  GPIO_NUM_33                                                                                                                                                            
#define D4                  GPIO_NUM_25
#define D5                  GPIO_NUM_26
#define D6                  GPIO_NUM_27
#define D7                  GPIO_NUM_14

#define OV7670_ADDR_WRITE   0x42
#define OV7670_ADDR_READ    0x43 

xQueueHandle hc_sr501_queue;
xQueueHandle factory_reset_queue;

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