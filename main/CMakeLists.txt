idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "include"
                        REQUIRES defines nvs_flash uart_console wifi_connection wifi_events http_server websocket control_buttons esp32-camera camera move_sensor
                      websocket)
spiffs_create_partition_image(spiffs ../html FLASH_IN_PROJECT)                      