#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_spiffs.h"
#include "wifi_connection.h"
#include "custom_http_server.h"
#include "esp_err.h"
#include "esp_camera.h"
#include "bitmap.h"

#define TAG ""
#define PART_BOUNDARY "123456789000000000000987654321"

#define BUFFER_LEN 512

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BMP_PART = "Content-Type: image/bitmap\r\nContent-Length: %u\r\n\r\n";
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";

// static ip4_addr_t get_ip_addr(void) {
//     tcpip_adapter_ip_info_t ip_info; 
    
//     // IP address.
//     tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);

//     return ip_info.ip;
// }


static esp_err_t write_gray_frame(httpd_req_t *req, camera_fb_t * fb) {
    char* buf;
    int x = 0;
    int size;
    esp_err_t err = ESP_OK;
    
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    /* To save RAM send the converted image in chunks of 512 bytes. */
    buf = malloc( BUFFER_LEN * 3 );
    
    if (!buf ) {
        ESP_LOGE(TAG, "Dinamic memory failed");
        return ESP_FAIL;    
    }

    while ( (x<fb->len) && (err == ESP_OK) ) {
        size = (fb->len >= BUFFER_LEN) ? BUFFER_LEN : fb->len;       

        /* To convert, match the RGB bytes to the value of the PGM byte. */
        for (int i=0; i<size; i++) {
            buf[i * 3 ] = fb->buf[i + x];
            buf[(i * 3) + 1 ] = fb->buf[i + x];
            buf[(i * 3) + 2 ] = fb->buf[i + x];        
        }

        err = httpd_resp_send_chunk(req, buf, size * 3);
        x += size;
    }

    free( buf );

    return err;
}

/* Convert the rgb565 format in a rgb bitmap */
static esp_err_t write_rgb565_frame(httpd_req_t *req, camera_fb_t * fb) {
    char* buf;
    int x = 0;
    int size;
    esp_err_t err = ESP_OK;
    int rgb_index = 0;
    uint8_t hb;
    uint8_t lb;
    
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    /* To save RAM send the converted image in chunks of 512 bytes. */
    buf = malloc( BUFFER_LEN * 3 );
    
    if (!buf ) {
        ESP_LOGE(TAG, "Dinamic memory failed");
        return ESP_FAIL;    
    }

    while ( (x<fb->len) && (err == ESP_OK) ) {
        size = (fb->len >= (BUFFER_LEN * 2)) ? (BUFFER_LEN * 2) : fb->len;       

        rgb_index = 0;

        /* Take two rgb565 bytes to build the rgb. */
        for (int i=0; i<size; i+=2) {
            hb = fb->buf[i + x];
            lb = fb->buf[i + x + 1];
            buf[ rgb_index++ ] = (lb & 0x1F) << 3;                      // red
            buf[ rgb_index++ ] = (hb & 0x07) << 5 | (lb & 0xE0) >> 3;   // green
            buf[ rgb_index++ ] = hb & 0xF8;                             // blue
        }

        err = httpd_resp_send_chunk(req, buf, rgb_index);
        x += size;
    }

    free( buf );

    return err;
}

static esp_err_t handle_rgb_bmp_stream(httpd_req_t *req) {
    char * part_buf[64];
    esp_err_t err = ESP_OK;

    camera_fb_t * fb = esp_camera_fb_get();
    sensor_t * sensor = esp_camera_sensor_get();

    bitmap_header_t* header = bmp_create_header(fb->width, fb->height);
    if (header == NULL) {
        return ESP_FAIL;
    }

    err = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);

    esp_camera_fb_return(fb);

    while (err == ESP_OK) {
        fb = esp_camera_fb_get();

        if (!fb) {
            ESP_LOGE(TAG, "Camera Capture Failed");
            err = ESP_FAIL;            
        }

        if (err == ESP_OK) {
            int len = fb->len;

            if(sensor->pixformat == PIXFORMAT_GRAYSCALE){
                len *= 3;
            } 
            
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_BMP_PART, len + sizeof(*header));

            err = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }

        if (err == ESP_OK) {
            err = httpd_resp_send_chunk(req, (const char*)header, sizeof(*header));
        }

        if (err == ESP_OK) {
            /* convert an image with a gray format of 8 bits to a 24 bit bmp. */            
            if(sensor->pixformat == PIXFORMAT_GRAYSCALE){
                err = write_gray_frame(req, fb);
            /* To save RAM and CPU in camera ISR use the rgb565 and convert to RGB in the APP */
            }else if(sensor->pixformat == PIXFORMAT_RGB565){
                err = write_rgb565_frame(req, fb);
            }else{
                err = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
            }
        }
        
        if (err == ESP_OK) {
            err = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        esp_camera_fb_return(fb);
    }
    free(header);
    return err;
}

static esp_err_t handle_rgb_bmp(httpd_req_t *req) {
    esp_err_t err = ESP_OK;

    // acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();

    sensor_t * sensor = esp_camera_sensor_get();
    
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    bitmap_header_t* header = bmp_create_header(fb->width, fb->height);
    if (header == NULL) {
        return ESP_FAIL;
    }

    err = httpd_resp_set_type(req, "image/bmp");
    
    if (err == ESP_OK){
        err = httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.bmp");
    }

    if (err == ESP_OK) {
        err = httpd_resp_send_chunk(req, (const char*)header, sizeof(*header));
    }

    free(header);

    if (err == ESP_OK) {
        /* convert an image with a gray format of 8 bits to a 24 bit bmp. */        
        if(sensor->pixformat == PIXFORMAT_GRAYSCALE){
            err = write_gray_frame(req, fb);
        /* To save RAM and CPU in camera ISR use the rgb565 and convert to RGB in the APP */
        }else if(sensor->pixformat == PIXFORMAT_RGB565){
            err = write_rgb565_frame(req, fb);
        }else{
            err = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
        }
    }

    /* buf_len as 0 to mark that all chunks have been sent. */
    if (err == ESP_OK) {
        err = httpd_resp_send_chunk(req, 0, 0);
    }

    esp_camera_fb_return(fb);

    return err;
}
 
httpd_uri_t bmp = {
    .uri       = "/bmp",
    .method    = HTTP_GET,
    .handler   = handle_rgb_bmp,
};

httpd_uri_t bmp_stream = {
    .uri       = "/bmp_stream",
    .method    = HTTP_GET,
    .handler   = handle_rgb_bmp_stream,
};

static ip4_addr_t get_ip_addr(void) {
    tcpip_adapter_ip_info_t ip_info; 
   	
    // IP address.
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);

    return ip_info.ip;
}

httpd_handle_t start_webserver(void) {
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    // ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");

        printf("Error 1\n");
        sensor_t * sensor = esp_camera_sensor_get();
        printf("Error 2\n");

        ip4_addr_t s_ip_addr = get_ip_addr();
       
        printf("Error 3\n");
        if (sensor->pixformat == PIXFORMAT_GRAYSCALE) {
            httpd_register_uri_handler(server, &bmp);
            httpd_register_uri_handler(server, &bmp_stream);

            ESP_LOGI(TAG, "Open http://" IPSTR "/bmp for a single image/bmp gray image", IP2STR(&s_ip_addr));
            ESP_LOGI(TAG, "Open http://" IPSTR "/bmp_stream for multipart/x-mixed-replace stream of gray bitmaps", IP2STR(&s_ip_addr));
            ESP_LOGI(TAG, "Open http://" IPSTR "/pgm for a single image/x-portable-graymap image", IP2STR(&s_ip_addr));
        } else if ((sensor->pixformat == PIXFORMAT_RGB565) || (sensor->pixformat == PIXFORMAT_RGB888)) {
            httpd_register_uri_handler(server, &bmp);
            httpd_register_uri_handler(server, &bmp_stream);
            
            ESP_LOGI(TAG, "Open http://" IPSTR "/bmp for single image/bitmap image", IP2STR(&s_ip_addr));
            ESP_LOGI(TAG, "Open http://" IPSTR "/bmp_stream for multipart/x-mixed-replace stream of bitmaps", IP2STR(&s_ip_addr));
        } else if (sensor->pixformat == PIXFORMAT_JPEG) {   
     
            ESP_LOGI(TAG, "Open http://" IPSTR "/jpg for single image/jpg image", IP2STR(&s_ip_addr));
            ESP_LOGI(TAG, "Open http://" IPSTR "/jpg_stream for multipart/x-mixed-replace stream of JPEGs", IP2STR(&s_ip_addr));
        }
        printf("Error 4\n");
        return server;
    }
    
    printf("Error 4\n");
    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}




esp_err_t get_handler(httpd_req_t* req) {
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_vfs_spiffs_register(&config);

    char path[600];

    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    { 
        sprintf(path, "/spiffs%s", req->uri);
    }

    char *ptr = strrchr(path, '.');
    if(strcmp(ptr,".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
   
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        httpd_resp_send_404(req);
    }
    else
    {
        UBaseType_t is_filled = 0;
        char lineRead[256];

        is_filled = uxQueueMessagesWaiting(wifi_scan_queue);

        httpd_resp_set_hdr(req, "Connection", "close");
        
        if (is_filled)
        {
            char ret[1048];

            xQueuePeek(wifi_scan_queue, &ret, 10);
            while (fgets(lineRead, sizeof(lineRead), file))
            {
                if(strstr( lineRead, "@scanlist@"))
                {
                    char *p_content_continue = strrchr(lineRead, '<');
                    char *p_content_start = strtok(lineRead, "@");

                    httpd_resp_sendstr_chunk(req, p_content_start);
                    httpd_resp_sendstr_chunk(req, ret);
                    httpd_resp_sendstr_chunk(req, p_content_continue);
                }
                else
                {
                    httpd_resp_sendstr_chunk(req,lineRead);
                }
            }
            httpd_resp_sendstr_chunk(req, NULL);
        }
        else
        {
            while (fgets(lineRead, sizeof(lineRead), file))
            {
                httpd_resp_sendstr_chunk(req,lineRead);
            }
            httpd_resp_sendstr_chunk(req, NULL);
        }
    }

    esp_vfs_spiffs_unregister(NULL);

    return ESP_OK;
}


esp_err_t get_asset_handler(httpd_req_t* req)
{

    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_vfs_spiffs_register(&config);

    char path[600];

    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    { 
        sprintf(path, "/spiffs%s", req->uri);
    }

    char *ptr = strrchr(path, '.');
    if(strcmp(ptr,".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
    if(strcmp(ptr,".gif") == 0)
    {
        httpd_resp_set_type(req, "image/gif");
    }
    if(strcmp(ptr,".png") == 0)
    {
        httpd_resp_set_type(req, "image/png");
    }
   
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        httpd_resp_send_404(req);
    }
    else
    {
        char lineRead[256];
        while (fgets(lineRead, sizeof(lineRead), file))
        {
            httpd_resp_sendstr_chunk(req,lineRead);
        }
        httpd_resp_sendstr_chunk(req, NULL);
    }

    esp_vfs_spiffs_unregister(NULL);

    return ESP_OK;
}

esp_err_t post_connect_handler(httpd_req_t* req)
{
    char content[512];
    memset(content, 0, 512);
    char *resp;

    size_t recv_size = sizeof(content);
    int ret = httpd_req_recv(req, content, recv_size);
    httpd_resp_set_hdr(req, "Connection", "close");

    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    if(strstr(content, "connect"))
    {
        
        char passwd_str[32];
        char ssid_str[32];
        memset(passwd_str, 0, 32);
        memset(ssid_str, 0, 32);
        char *name_field_start = NULL;
        name_field_start = strstr(content, "ssid");
        if(name_field_start)
        {
            char *ssid = NULL;
            ssid = strstr(name_field_start, "\n\r");
            if(ssid)
            {
                
                int i = 0;
                while(*ssid == '\n' || *ssid == '\r')
                {
                    ssid++;
                }
                while(ssid[i] != '\n')
                {
                    i++;
                }
                memcpy(ssid_str, ssid, i);
                ssid_str[i - 1] = '\0';
            
            }
        }
        char *passwd_field_start = NULL;
        passwd_field_start = strstr(content, "passwd");
        if(passwd_field_start)
        {
            char *passwd = NULL;
            passwd = strstr(passwd_field_start, "\n\r");
            if(passwd)
            {
               
                int i = 0;
                while(*passwd == '\n' || *passwd == '\r')
                {
                    passwd++;
                }
                while(passwd[i] != '\n')
                {
                    i++;
                }
                memcpy(passwd_str, passwd, i);
                passwd_str[i - 1] = '\0';
            }
        }
         esp_wifi_disconnect();
         httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

         if(strlen(passwd_str) > 0)
         {
             wifi_connect(ssid_str, passwd_str);

         }
         else
         {
             wifi_connect(ssid_str, "");
         }
         
         return ESP_OK;      
    }
    else
    {
        resp = "can't connect";
    }
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t* req)
{
    char content[512];
    memset(content, 0, 512);
    char *resp;

    size_t recv_size = sizeof(content);
    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    if(strstr(content, "scan"))
    {
        wifi_scan_aps();
        UBaseType_t is_filled = 0;

        is_filled = uxQueueMessagesWaiting(wifi_scan_queue);
        
        if (is_filled)
        {
            char response[1048];
            xQueuePeek(wifi_scan_queue, &response, 10);
            httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    else
    {
        resp = "Can't scan";
    }
   
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL
    };

httpd_uri_t uri_get_css = {
    .uri = "/app.css",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_bulma = {
    .uri = "/bulma.min.css",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_js = {
    .uri = "/app.js",
    .method = HTTP_GET,
    .handler = get_asset_handler,
    .user_ctx = NULL
};    

httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL
    };
httpd_uri_t uri_post_connect = {
    .uri = "/connect",
    .method = HTTP_POST,
    .handler = post_connect_handler,
    .user_ctx = NULL
};

httpd_handle_t custom_http_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.stack_size = 12098;
    config.lru_purge_enable = true;
    config.max_open_sockets = 4;
    config.send_wait_timeout = 4;
    config.recv_wait_timeout = 4;

    if (httpd_start(&server2, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server2, &uri_get);
        httpd_register_uri_handler(server2, &uri_get_css);
        httpd_register_uri_handler(server2, &uri_get_js);
        httpd_register_uri_handler(server2, &uri_get_bulma);
        httpd_register_uri_handler(server2, &uri_post);
        httpd_register_uri_handler(server2, &uri_post_connect);
/*         httpd_register_uri_handler(server, &bmp);
        httpd_register_uri_handler(server, &handle_rgb_bmp_stream); */
    }

    return server2;
}