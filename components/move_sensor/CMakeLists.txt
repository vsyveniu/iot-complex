idf_component_register(SRCS "movement.c"
                       INCLUDE_DIRS "include"
                       REQUIRES defines esp32-camera camera http_server)