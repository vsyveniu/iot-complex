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

     

        printf("%s\n", "try send data");
        esp_websocket_client_send(client, data, strlen(data), 10);
       // esp_websocket_client_stop(client);
         return ESP_OK;

   return ESP_FAIL;
}


esp_err_t websocket_init()
{
	esp_err_t err;
    //const char *pem = "-----BEGIN CERTIFICATE-----MIIFZTCCBE2gAwIBAgISA4PIq5AIt+W91Riua0HxVuMyMA0GCSqGSIb3DQEBCwUAMEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQDExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0yMDExMTAwOTM2MzRaFw0yMTAyMDgwOTM2MzRaMCExHzAdBgNVBAMTFmlvdC10cmFjay52c3l2ZW5pdS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDu4L518334iSKqiR8mYMnD2rc7l7hbMSCSle9CBsgr5XcB2Bj7C3qT1odnOGinEG3RZu74iRqiydqS4tTSZFwOyQdBdjjWx5DhWNJTqSuXcyrE/ko6e33yp8NW2WBhQNdJmzp7ecAHKn5tKSxhMHqz++1HGZpPj042otGuhkwNTVglLvppTSEK3ypiXhcYYZz+pFaPCyJ06Lsm94LX+gkWsIlbRUPKgxnHJrlf5L/hkXwRS9EnmkzNva1xWIxZ1p9encyrdsPCnY1FeTch/yuy7Z6Eno4W1VsCJU9HIaanj3GfbzXXPu0hrK2GjMpDmNHdP3aq0+IlBU33GBT68R99AgMBAAGjggJsMIICaDAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFK8NT0LiolVJ2+bAIy+Rm2BGvjtlMB8GA1UdIwQYMBaAFKhKamMEfd265tE5t6ZFZe/zqOyhMG8GCCsGAQUFBwEBBGMwYTAuBggrBgEFBQcwAYYiaHR0cDovL29jc3AuaW50LXgzLmxldHNlbmNyeXB0Lm9yZzAvBggrBgEFBQcwAoYjaHR0cDovL2NlcnQuaW50LXgzLmxldHNlbmNyeXB0Lm9yZy8wIQYDVR0RBBowGIIWaW90LXRyYWNrLnZzeXZlbml1LmNvbTBMBgNVHSAERTBDMAgGBmeBDAECATA3BgsrBgEEAYLfEwEBATAoMCYGCCsGAQUFBwIBFhpodHRwOi8vY3BzLmxldHNlbmNyeXB0Lm9yZzCCAQUGCisGAQQB1nkCBAIEgfYEgfMA8QB3AFzcQ5L+5qtFRLFemtRW5hA3+9X6R9yhc5SyXub2xw7KAAABdbG6jJkAAAQDAEgwRgIhAMU3GljVGmmkHu1lA373I0qA4QNjC1aOb6fmCoeKaCbvAiEA/2kKj6YWKwyTiP0l3KosqBa5qGBdpI5kwKipq+j15fIAdgD2XJQv0XcwIhRUGAgwlFaO400TGTO/3wwvIAvMTvFk4wAAAXWxuoyXAAAEAwBHMEUCIBe95Kic7c/pgYIbuvJPMHDwkKI8d3Bf0dp6vLIEZ5XoAiEAtKWim8CG/0Z3xuGgmQtNHiPLD7uKmvs99DaQduMzX+4wDQYJKoZIhvcNAQELBQADggEBAGK9RLKdHfUYdc/UOVOO0Ce+tI6dmf3fpGkE3QD83PaazZEnS6D5wsBszWZqj/KshariBLLQYkwLn40x0qwDbMHDAok7Et2U9eghalrfwQFfeu27srgNBuVebC2fAR31MT2jVxCOPH9aIo3USJ77Oxwun2Q9mHt6UDnBod7ea3BQ2qoF0LOvbjEdoS6v40e6/ZcHGFCZMYPxtvhBCiPThIVETbjigT+tvcW/Pg9QgN3mlgNZZQQaQkQhrN9PKxqAq7TLYVHQ7GXLaCIHoSwIA4AMoVn08yqcH9GJ7wEyBanCyM1Yp8D4cghQEh9ut0bzganKFWdfiTr6vpkt3cTNroU=-----END CERTIFICATE----------BEGIN CERTIFICATE-----MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMTDkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0NlowSjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMTGkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EFq6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWAa6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIGCCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNvbTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9kc3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAwVAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcCARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAzMDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwuY3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsFAAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJouM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwuX4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlGPfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==-----END CERTIFICATE-----";

    esp_websocket_client_config_t websocket_cfg = {
		.uri = "ws://46.101.189.225",
		.port = 6003,
        //.cert_pem = pem,
	};

  /*   shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
                                         pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary(); */

	client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    err = esp_websocket_client_start(client);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ws_send_data("ESP_INTRUSION");

    return (ESP_OK);
}


