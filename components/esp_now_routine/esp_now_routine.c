#include "ESP_NOW_ROUTINE.h"


static uint8_t satellite_1_mac[ESP_NOW_ETH_ALEN] = { 0x7c, 0x9e, 0xbd, 0xe3, 0xa3, 0x2c };


static void espnow_send_cb()
{
	printf("%s\n", "data was sent?");

}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{

	for(int i = 0; i < data_len; i++)
	{	
		printf("%d\n", data[i]);
	}

	printf("%s\n", "received");
}

esp_err_t esp_now_install()
{
    printf("%s\n", "esp now  init");

	esp_err_t err;

	err = esp_now_init();
	if(err != ESP_OK)
	{
		printf("%s\n", esp_err_to_name(err));
	}

	struct esp_now_peer_info peer = {
		.ifidx = ESP_IF_WIFI_STA,
		.encrypt = false,

	};

	memcpy(peer.peer_addr, satellite_1_mac, ESP_NOW_ETH_ALEN);

    ESP_ERROR_CHECK( esp_now_add_peer(&peer) );

	int is_on = 0;

	is_on = esp_now_is_peer_exist(peer.peer_addr);

	printf("is exist %d\n", is_on);

	ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );

	uint8_t data_to_send[42];
	memset(data_to_send, 0, 42);

	for (int i = 0; i < 42; i++)
	{
		data_to_send[i] = i;
	}

	err = esp_now_send(peer.peer_addr, data_to_send, 42);
	
	printf("send %s\n", esp_err_to_name(err));

    return (ESP_OK);
}

esp_err_t hc_sr501_read()
{
  
    return (ESP_OK);
}

