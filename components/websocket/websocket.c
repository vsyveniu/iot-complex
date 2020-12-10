#include "esp_log.h"
#include "esp_err.h"
#include "websocket.h"



static const char *TAG = "WEBSOCKET: ";

void ws_connected_handler()
{
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");

}

void ws_got_data_handler( esp_websocket_event_data_t *data)
{
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_GOT_DATA");
     ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ws_connected_handler();
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        
/*         if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            */
            ws_got_data_handler(data);
      //  }
       // ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

       // xTimerReset(shutdown_signal_timer, portMAX_DELAY);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}


esp_err_t ws_send_data(void *data_income)
{
    esp_err_t err;
    char *data = (char *)data_income;

     err = esp_websocket_client_start(client);
	if(err != ESP_OK)
	{
		printf("%s\n", esp_err_to_name(err));
	}
    else
    {
        printf("%s\n", "try send data");
        esp_websocket_client_send(client, data, strlen(data), 10);
        esp_websocket_client_stop(client);
         return ESP_OK;
    }

   return ESP_FAIL;
}


esp_err_t websocket_init()
{
	esp_err_t err;
    esp_websocket_client_config_t websocket_cfg = {
		//.uri = "ws://echo.websocket.org",
		.uri = "ws://46.101.189.225",
		//.host = "iot-track.vsyveniu.com",
		.port = 6001,
		//.path = "stream",
		//.
	};

  /*   shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
                                         pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary(); */

	client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    
    //ws_send_data(&client);
   
    
    return (ESP_OK);
}


