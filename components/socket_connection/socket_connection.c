#include "socket_connection.h"
//#include "esp_tls.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include <errno.h>
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include <mbedtls/error.h>
#include "driver/uart.h"

static const char* ERRORTAG = "error: ";
static const char* INFOTAG = "info: ";

void make_json_payload(char *method, char* payload, char* host, char* query, int json_len,
                       char* json_str)
{
    if (strlen(query) > 0)
    {
        sprintf(payload,
                "%s /%s HTTP/1.0\r\nHost: %s\r\nContent-Type: "
                "application/json\r\nContent-Length: %d\r\n\r\n%s",
                method, query, host, json_len, json_str);
        ESP_LOGI(INFOTAG, "query payload \n%s", payload);
    }
    else
    {
        sprintf(payload,
                "%s / HTTP/1.0\r\nHost: %s\r\nContent-Length: "
                "%d\r\nContent-Type: application/json\r\n\r\n%s\r\n",
                method, host, json_len, json_str);
        ESP_LOGI(INFOTAG, "payload: \n%s", payload);

    }
}

esp_err_t socket_tls_create(char* host, char* query, uint8_t temp, uint8_t hum,
                            char* rx_buff, char *json_str, const char* port)
{
    static char errortext[256];
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_context ssl;

    mbedtls_ssl_init(&ssl);
    mbedtls_net_init(&server_fd);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    int32_t ret = 0;
    char* pers = "esp32_client";

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char*)pers,
                                     strlen(pers))) != 0)
    {
        ESP_LOGE(ERRORTAG, "mbedtls_ctr_drbg_seed failed %d\n", ret);
        return (ESP_FAIL);
    }

    if ((ret = mbedtls_net_connect(&server_fd, host, port,
                                   MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        ESP_LOGE(ERRORTAG, "mbedtls_net_connect failed %d\n", ret);
        return (ESP_FAIL);
    }

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(ERRORTAG, "mbedtls_ssl_config_defaults failed %d\n", ret);
        return (ESP_FAIL);
    }
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    ret = mbedtls_ssl_setup(&ssl, &conf);

    if (ret != 0)
    {
        mbedtls_strerror(ret, errortext, sizeof(errortext));
        ESP_LOGE(ERRORTAG, "error from mbedtls_ssl_setup: %d - %x - %s\n", ret,
                 ret, errortext);
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, host)) != 0)
    {
        ESP_LOGE(ERRORTAG, "mbedtls_ssl_set_hostname failed %d\n", ret);
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv,
                        NULL);
    esp_err_t err;
    unsigned char rx_buffer[1024];
    unsigned char pay_buff[1024];
   
    make_json_payload("POST", (char *)pay_buff, host, query, strlen(json_str), json_str);

    err = mbedtls_ssl_write(&ssl, pay_buff, strlen((char *)pay_buff));

    if (err < 0)
    {
        ESP_LOGE(ERRORTAG, "Error occurred during sending: errno %d\n", err);
    }
    else
    {
        int32_t len = mbedtls_ssl_read(&ssl, rx_buffer, sizeof(rx_buffer) - 1);
        rx_buffer[len] = '\0';

        if (len < 0)
        {
            ESP_LOGE(ERRORTAG, "recv failed: errno %d\n", err);
        }
        else
        {
            memcpy(rx_buff, rx_buffer, strlen((char*)rx_buffer));
        }
    }
    mbedtls_net_free(&server_fd);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return (ESP_OK);
}
