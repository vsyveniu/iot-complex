#include "HC_SR501.h"

static void IRAM_ATTR hc_sr501_intr(void *args){

	uint32_t pin = (uint32_t) args;
	xQueueSendFromISR(hc_sr501_queue, &pin, NULL);
}

void hc_sr501_fired()
{
	uint32_t pin;

	while (true)
	{
		if(xQueueReceive(hc_sr501_queue, &pin, portMAX_DELAY))
		{
            printf("interrupt:  %d\n", gpio_get_level(HC_SR501));
			if(gpio_get_level(HC_SR501) == 1)
			{
				//gpio_set_level(LED_1, 1);
			}
		}
        vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

esp_err_t hc_sr501_init()
{
    printf("%s\n", "hc init");

    gpio_config_t hc_sr501_conf = {
		.pin_bit_mask = GPIO_SEL_12,
		.mode = GPIO_MODE_INPUT,
		/* .pull_up_en = GPIO_PULLUP_DISABLE,*/
		//.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_POSEDGE,
	};

    gpio_config(&hc_sr501_conf);

	gpio_isr_handler_add(HC_SR501, hc_sr501_intr, (void *)HC_SR501);

	hc_sr501_queue = xQueueCreate(10, sizeof(int));

	xTaskCreate(hc_sr501_fired, "hc_fired", 2048, NULL, 1, NULL);
    return (ESP_OK);
}

esp_err_t hc_sr501_read()
{
  
    return (ESP_OK);
}

