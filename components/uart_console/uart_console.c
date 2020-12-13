#include "uart_console.h"
#include <ctype.h>

static void del_symbol_inside_str(char *str, int position) {
    int len = strlen(str);

    for (int i = position; i <= len; i++) {
        str[i - 1] = str[i];
    }
}

static void add_symbol_inside_str(char *str, int position, char c) {
    char tmp = '\0';
    int len = strlen(str);

    for (int i = position; i <= len; i++) {
        tmp = str[i];
        str[i] = c;
        c = tmp;
    }
}

static void esc_to_do(uint8_t *buf, t_flag *f) {
    if (buf[2] == 'D' && f->position > 0) {
        uart_write_bytes(UART_NUM, (char *)buttons.left, 1);
        f->position--;
    }
    else if (buf[2] == 'C' && f->position < f->count_str_size) {
        uart_write_bytes(UART_NUM, (char *)buttons.right, 3);
        f->position++;
    }
}

static void data_to_do(char *str, uint8_t *buf, t_flag *f, int read) {
    if (f->position != f->count_str_size) {
        if (read == 1) {
            uart_write_bytes(UART_NUM, (char *)insert_one_space, 3);
            uart_write_bytes(UART_NUM, (char *)buf, read);
            add_symbol_inside_str(str, f->position, (char)buf[0]);
            f->count_str_size += read;
            f->position++;
        } else {
            for (int i = 0; i < read; i++) {
                uart_write_bytes(UART_NUM, (char *)insert_one_space, 3);
                uart_write_bytes(UART_NUM, (const char *) &(buf[i]), 1);
                add_symbol_inside_str(str, f->position, buf[i]);
                f->count_str_size++;
                f->position++;
            }
        }
    } else {
        strcat(str, (char *)buf);
        uart_write_bytes(UART_NUM, (char *)buf, read);
        f->count_str_size += read;
        f->position = f->count_str_size;
    }
}

static void enter_to_do(char *str, t_flag *f) {
    esp_err_t console_ret = 0;
    int ret = 0;

    console_ret = esp_console_run(str, &ret);

    if (console_ret == ESP_ERR_INVALID_ARG) // deleted || i = 0
    {
        uart_write_bytes(UART_NUM, (char *)buttons.enter, 4);
    }
    else if (console_ret == ESP_ERR_NOT_FOUND)
    {
        uart_write_bytes(UART_NUM, (char *)buttons.enter, 4);
        uart_print_str(UART_NUM, "Command not found");
        uart_write_bytes(UART_NUM, (char *)buttons.enter, 4);
    } else if (strcmp(str, "clear") != 0) {
        uart_write_bytes(UART_NUM, (char *) buttons.enter, 4);
    }
    uart_flush(UART_NUM);

    memset(str, '\0', strlen(str));
    f->position = 0;
    f->count_str_size = 0;
}

static void backspace_to_do(char *str, t_flag *f, int buf_size) {
    if (f->position == f->count_str_size && f->position != 0) {
        uart_write_bytes(UART_NUM, (char *)buttons.backspace, 4);
        f->count_str_size -= 1;
        f->position = f->count_str_size;
        str[f->count_str_size] = '\0'; //delete last symbol from str
    } else if (f->position != 0) {
        uart_write_bytes(UART_NUM, (char *)buttons.backspace, 4);
        del_symbol_inside_str(str, f->position);
        f->count_str_size--;
        f->position--;
    }
}

static void uart_data_handler(char *str, t_flag *f) {
    uint8_t *buf = NULL;
    int read = 0;
    size_t buf_size = 0;

    uart_get_buffered_data_len(UART_NUM, &buf_size);
    buf = malloc(sizeof(uint8_t) * (buf_size + 1));
    memset(buf, '\0', buf_size + 1);
    read = uart_read_bytes(UART_NUM, buf, buf_size + 1, 1);

    switch (buf[0]) {
        case 27:
            esc_to_do(buf, f);
            break;
        case 13:
            enter_to_do(str, f);
            break;
        case 127:
            backspace_to_do(str, f, read);
            break;
        default:
            data_to_do(str, buf, f, read);
    }
    free(buf);
    buf = NULL;
}

static void uart_start() {
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM, 2048, 2048, 20, &uart0_queue, 0);
    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void task_uart_event_handler(void **registered_commands) {
    uart_event_t event;
    char str[1024];
    t_flag f = {0, 0}; // check

    memset(str, 0, 1024);
    while (true) {
        if (xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {

            if (event.type == UART_DATA) {
                xSemaphoreTake(xMutex, (portTickType)portMAX_DELAY);
                xSemaphoreGive(xMutex);
                uart_data_handler(str, &f);

            }
            if (event.type == UART_BREAK) {
                printf("UART_BREAK------------------------------\n");
            }
            if (event.type == UART_PATTERN_DET) {
                printf("UART_PATTERN_DET------------------------------\n");
            }
            if (event.type == UART_EVENT_MAX) {
                printf("UART_EVENT_MAX------------------------------\n");
            }
            if (event.type == UART_DATA_BREAK) {

            }
        }
    }
    vTaskDelete(NULL);
}

static void uart_inti() {
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM, 2048, 2048, 20, &uart0_queue, 0);
    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, TX_UART_NUM, RX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void start_console_inti() {
    esp_err_t err;

    esp_console_config_t console_conf = {
            .max_cmdline_length = 256,
            .max_cmdline_args = 12,
            .hint_color = 37,
            .hint_bold = 1,
    };

    err = esp_console_init(&console_conf);
}

void uart_console_start() {
    uart_inti();
    start_console_inti();
    xTaskCreate(task_uart_event_handler, "task_uart_event_handler", 14096, NULL, 10, NULL);
}

