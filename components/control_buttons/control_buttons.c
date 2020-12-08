#include "control_buttons.h"

static void IRAM_ATTR factory_reset_intr(void *args){

	uint32_t pin = (uint32_t) args;
	xQueueSendFromISR(factory_reset_queue, &pin, NULL);
}

void factory_reset_pushed()
{
	uint32_t pin;

	while (true)
	{
		if(xQueueReceive(factory_reset_queue, &pin, portMAX_DELAY))
		{
            printf("button interrupt:  %d\n", gpio_get_level(FACTORY_BUTTON));
			if(gpio_get_level(FACTORY_BUTTON) == 1)
			{
				//gpio_set_level(LED_1, 1);
			}
		}
        vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

esp_err_t control_buttons_init()
{
    gpio_config_t factory_reset_conf = {
		.pin_bit_mask = GPIO_SEL_39,
		.mode = GPIO_MODE_INPUT,
		// .pull_up_en = GPIO_PULLUP_ENABLE
		//.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_POSEDGE,
	};

    gpio_config(&factory_reset_conf);

	gpio_set_direction(FACTORY_BUTTON, GPIO_MODE_INPUT);

	gpio_isr_handler_add(FACTORY_BUTTON, factory_reset_intr, (void *)FACTORY_BUTTON);

	factory_reset_queue = xQueueCreate(10, sizeof(int));

	xTaskCreate(factory_reset_pushed, "reset_pushed", 2048, NULL, 1, NULL);

    return (ESP_OK);
}
