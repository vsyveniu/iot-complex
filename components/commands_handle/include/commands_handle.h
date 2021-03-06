#ifndef COMMANDS_HANDLE_H
# define COMMANDS_HANDLE_H

#include <unistd.h>
#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <string.h>
#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_console.h"
#include "commands.h"
#include "uart_utils_funcs.h"
#include "freertos/semphr.h"
#include "defines.h"
#include "argtable3/argtable3.h"
#include "wifi_connection.h"

void handle_ssid_set(struct arg_str *ssid, struct arg_str *passwd);
void handle_connection_status();
void handle_disconnect();
void handle_http_get(char* host, char* query);
//void handle_wifi_params_set(struct arg_str* ssid, struct arg_str* passwd);
void handle_wipe();
void handle_help();
void handle_clear();

#endif