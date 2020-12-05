#include "commands_handle.h"
#include "driver/periph_ctrl.h"
#include "soc/timer_group_struct.h"
#include <sys/time.h>
#include "sntp.h"
#include "argtable3/argtable3.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include <errno.h>
#include "socket_connection.h"
#include "esp_tls.h"
#include "esp_wifi.h"

void handle_ssid_set(struct arg_str* ssid, struct arg_str* passwd)
{
    int32_t ssid_len;
    int32_t passwd_len;

    ssid_len = strlen(*ssid->sval);
    passwd_len = strlen(*passwd->sval);

    char* ssid_copy;
    char* passwd_copy;

    ssid_copy = calloc(ssid_len + 1, sizeof(char));
    passwd_copy = calloc(passwd_len + 1, sizeof(char));

    if(ssid_copy != NULL)
    {
        memcpy(ssid_copy, *ssid->sval, ssid_len);
    }
    else
    {
        ssid_copy = "";
    }
    if(passwd_copy != NULL)
    {
        memcpy(passwd_copy, *passwd->sval, passwd_len);
    }
    else
    {
        passwd_copy = "";
    }

    wifi_sta_info_s wifi_sta_info[1];
    xQueuePeek(wifi_info_queue, &wifi_sta_info, 10);

    memcpy(wifi_sta_info->ssid_str, ssid_copy, strlen((char *)ssid));
    wifi_sta_info->ssid_str[strlen((char *)ssid)] = '\0';  
    memcpy(wifi_sta_info->passwd, passwd_copy, strlen((char *)passwd));
    wifi_sta_info->passwd[strlen((char *)passwd)] = '\0';  
    wifi_sta_info->wifi_reconnect_count = 0;

    xQueueOverwrite(wifi_info_queue, &wifi_sta_info);

    esp_wifi_disconnect();

    uart_print_str(UART_NUMBER, "\r\n");
    wifi_connect(ssid_copy, passwd_copy);
}

void handle_connection_status() { wifi_display_info(); }

void handle_disconnect() { esp_wifi_disconnect(); }

void handle_wipe()
{
   wifi_full_wipe_info();
}

void handle_help()
{
    uart_print_str(UART_NUMBER, "\n\rExamples of commands you may use:");
    uart_print_str(UART_NUMBER, "\n\rconnect -s=AP ssid  -p=password");
    uart_print_str(UART_NUMBER, "\n\rconnect -s AP ssid  -p password");
    uart_print_str(UART_NUMBER, "\n\rconnection-status");
    uart_print_str(UART_NUMBER, "\n\rdisconnect");
    uart_print_str(UART_NUMBER, "\n\rset-wifi-params -s \"ssid\" -p \"passwd\"");
}