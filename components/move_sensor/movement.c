#include "movement.h"

#define DATA_HCSR501 GPIO_NUM_12

QueueHandle_t hc_sr501_queue = NULL;

static void IRAM_ATTR hc_sr501_intr(void *args) {
    uint32_t pin = (uint32_t) args;
    xQueueSendFromISR(hc_sr501_queue, &pin, NULL);
}

void hc_sr501_fired() {
    uint32_t pin;

    while (true) {
        printf("%d\n", gpio_get_level(DATA_HCSR501));
        if(xQueueReceive(hc_sr501_queue, &pin, portMAX_DELAY)) {
            printf("interrupt:  %d\n", gpio_get_level(DATA_HCSR501));
            while(gpio_get_level(DATA_HCSR501) != 0)
            {
                 ws_send_data("ESP_INTRUSION");
                 vTaskDelay(2000/portTICK_PERIOD_MS);
            }
            ws_send_data("ESP_INTRUSION_END");
        }
        vTaskDelay(50);
    }
}

esp_err_t hc_sr501_init() {
    printf("%s\n", "hc init");

    gpio_config_t hc_sr501_conf = {
        .pin_bit_mask = GPIO_SEL_12,
        .mode = GPIO_MODE_INPUT,
        //.pull_up_en = GPIO_PULLUP_DISABLE,
        // .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    printf("JOIsdfjo isdjg %d\n\n", gpio_config(&hc_sr501_conf));
    gpio_isr_handler_add(DATA_HCSR501, hc_sr501_intr, (void *)DATA_HCSR501);

    hc_sr501_queue = xQueueCreate(10, sizeof(int));
    int count = 0;
    // while(true) {
    //   printf("%d %d\n", count, gpio_get_level(DATA_HCSR501));
    //   vTaskDelay(20);
    //   count++;
    // }
     xTaskCreate(hc_sr501_fired, "hc_fired", 2048, NULL, 1, NULL);
    return (ESP_OK);
}
