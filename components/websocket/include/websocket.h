#ifndef WEBSOCKET_H
# define WEBSOCKET_H

#include "defines.h"
#include "esp_websocket_client.h"


esp_err_t websocket_init();
esp_err_t ws_send_data(void *data);

#endif
