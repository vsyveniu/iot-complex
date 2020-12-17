#ifndef CUSTOM_HTTP_SERVER_H
# define CUSTOM_HTTP_SERVER_H

#include "defines.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include <string.h>
#include <unistd.h>

httpd_handle_t custom_http_server_init(void);
httpd_handle_t start_webserver(void);

#endif
