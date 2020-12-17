#ifndef UART_CONSOLE_H
# define UART_CONSOLE_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_console.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "freertos/semphr.h"
#include "uart_utils_funcs.h"
#include "commands.h"
#include "defines.h"

#define UART_NUM UART_NUM_1
#define TX_UART_NUM 17
#define RX_UART_NUM 16

QueueHandle_t uart0_queue;

typedef struct s_flags {
    int count_str_size;
    int position;
} t_flag;

struct buttons {
    char enter[5];
    uint8_t backspace[4];
    uint8_t left[3];
    uint8_t right[3];
};

const static uint8_t insert_one_space[3] = { 27, '[', '@',};

const static struct buttons buttons = {
        .enter = "\n\r>",
        .backspace = {0x08, 27, '[', 'P'},
        .left = {0x08, '[', 'D',},
        .right = {27, '[', 'C',},
};

void uart_console_start();

#endif
