idf_component_register(SRCS "http_server.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines nvs_flash spiffs esp_http_server wifi_connection)