#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/queue.h"
#include "custom_http_server.h"

void hc_sr501_fired();
esp_err_t hc_sr501_init();
