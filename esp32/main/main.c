#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "chip8.h"
#include <stdint.h>
#include "esp_wifi.h"
#include "lwip/sockets.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#define SSID "chip8"
extern uint8_t display[2048];
extern uint8_t keyboard[16];
extern uint8_t rom_index;

#define A GPIO_NUM_19
#define B GPIO_NUM_21
#define C GPIO_NUM_23
#define D GPIO_NUM_27
#define OE GPIO_NUM_26
#define CLK GPIO_NUM_25
#define R1 GPIO_NUM_22
#define R2 GPIO_NUM_18
#define STB GPIO_NUM_4

#define WIDTH   64
#define HEIGHT  32

#define MASK 0x00;
void udp_receive();
extern bool active;
esp_err_t event_handler(void *ctx, system_event_t *event) {
//    switch(event->event_id) {
//
//    case SYSTEM_EVENT_AP_STAIPASSIGNED:
//    	xTaskCreate(udp_receive, "udp_receive", 6000, NULL,configMAX_PRIORITIES - 1, NULL);
//    default:
//            break;

//    }
	return ESP_OK;
}
void scan() {
	while (true) {

		for (int row = 0; row < 16; row++) {

			for (int i = 0; i < 64; i++) {

				gpio_set_level(CLK, 0);

				gpio_set_level(R1, display[2047 - ((row * 64) + i)]);

				//es mushaobs
				gpio_set_level(R2, display[1023 - ((row * 64) + i)]);

				gpio_set_level(CLK, 1);

			}

			gpio_set_level(OE, 1);

			gpio_set_level(A, (row & 0x01));
			gpio_set_level(B, (row & 0x02));
			gpio_set_level(C, (row & 0x04));
			gpio_set_level(D, (row & 0x08));

			gpio_set_level(STB, 0);
			gpio_set_level(STB, 1);
			gpio_set_level(STB, 0);

			gpio_set_level(OE, 0);

		}

		// enable display

	}

}

void udp_receive() {

	int socket_fd;
	struct sockaddr_in ra;

	int recv_data;
	char data_buffer[30];
	/* Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
	 * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	 */

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	if (socket_fd < 0) {

		printf("socket call failed");
		exit(0);

	}

	ra.sin_family = AF_INET;

	ra.sin_addr.s_addr = htonl(INADDR_ANY);

	ra.sin_port = htons(1327);

	/* Bind the UDP socket to the port RECEIVER_PORT_NUM and to the current
	 * machines IP address (Its defined by RECEIVER_PORT_NUM).
	 * Once bind is successful for UDP sockets application can operate
	 * on the socket descriptor for sending or receiving data.
	 */

	if (bind(socket_fd, (struct sockaddr *) &ra, sizeof(struct sockaddr_in))
			== -1) {

		close(socket_fd);
		exit(1);
	}

	while (true) {

		recv_data = recvfrom(socket_fd, data_buffer, sizeof(data_buffer), 0,
		NULL, NULL);

		if (recv_data > 0) {

			data_buffer[recv_data] = '\0';
//
//			//if command is load rom
//			char *number=strrchr(data_buffer,':');
//			if(number && strlen(number)>0)
//			{
//				rom_index=(uint8_t)((data_buffer[5])-'0');
//				active=false;
//			}

//if command is key press or release
			char *key = strrchr(data_buffer, 'k');

			if (key && strlen(key) > 0) {

				if (data_buffer[1] == '>' && data_buffer[2] == '1') {
					rom_index = 1;
					active = false;
					continue;
				}
				if (data_buffer[1] == '>' && data_buffer[2] == '2') {
					rom_index = 2;
					active = false;
					continue;
				}
				if (data_buffer[1] == '>' && data_buffer[2] == '3') {
					rom_index = 3;
					active = false;
					continue;
				}
				if (data_buffer[1] == '>' && data_buffer[2] == '4') {
					rom_index = 4;
					active = false;
					continue;
				}
				if (data_buffer[1] == '>' && data_buffer[2] == '5') {
					rom_index = 5;
					active = false;
					continue;
				}
				if (data_buffer[1] == '>' && data_buffer[2] == '6') {
					rom_index = 6;
					active = false;
					continue;
				}


				switch (rom_index) {
				case 1: //brix
				case 2: //filter
					if (data_buffer[1] == '>' && data_buffer[2] == 'a')
						keyboard[4] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'd')
						keyboard[6] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'a')
						keyboard[4] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'd')
						keyboard[6] = 0;
					if (data_buffer[1] == '>' && data_buffer[2] == 'r')
						active = false;
					break;
				case 3: //pong ai
					if (data_buffer[1] == '>' && data_buffer[2] == 'w')
						keyboard[1] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 's')
						keyboard[4] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'w')
						keyboard[1] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 's')
						keyboard[4] = 0;
					if (data_buffer[1] == '>' && data_buffer[2] == 'r')
						active = false;

					break;
				case 4: //pong2
						//player1
					if (data_buffer[1] == '>' && data_buffer[2] == 'w')
						keyboard[1] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 's')
						keyboard[4] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'w')
						keyboard[1] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 's')
						keyboard[4] = 0;
					//player2
					if (data_buffer[1] == '>' && data_buffer[2] == 'v')
						keyboard[12] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'm')
						keyboard[13] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'v')
						keyboard[12] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'm')
						keyboard[13] = 0;

					if (data_buffer[1] == '>' && data_buffer[2] == 'r')
						active = false;
					break;
				case 5:					//space invaders
					if (data_buffer[1] == '>' && data_buffer[2] == 'a')
						keyboard[4] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'd')
						keyboard[6] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'w')
						keyboard[5] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'a')
						keyboard[4] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'd')
						keyboard[6] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'w')
						keyboard[5] = 0;
					if (data_buffer[1] == '>' && data_buffer[2] == 'r')
						active = false;
					break;

				case 6:					//tetris
					if (data_buffer[1] == '>' && data_buffer[2] == 'w')
						keyboard[4] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 's')
						keyboard[7] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'd')
						keyboard[6] = 1;
					if (data_buffer[1] == '>' && data_buffer[2] == 'd')
						keyboard[5] = 1;
					if (data_buffer[1] == '<' && data_buffer[2] == 'w')
						keyboard[4] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 's')
						keyboard[7] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'd')
						keyboard[6] = 0;
					if (data_buffer[1] == '<' && data_buffer[2] == 'd')
						keyboard[5] = 0;
					if (data_buffer[1] == '>' && data_buffer[2] == 'r')
						active = false;
					break;

				default:
					break;

				}
			}
			bzero(data_buffer, 30);

		} else {

			close(socket_fd);
			break;
		}

	}

}

void app_main() {
	nvs_flash_init();

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);

	wifi_init_config_t wifiInitializationConfig = WIFI_INIT_CONFIG_DEFAULT()
	;

	esp_wifi_init(&wifiInitializationConfig);

	esp_wifi_set_storage(WIFI_STORAGE_RAM);

	esp_wifi_set_mode(WIFI_MODE_AP);

	wifi_config_t ap_config = { .ap = { .ssid = SSID, .password = "sgsgsgsg",
			.channel = 0, .authmode = WIFI_AUTH_WPA2_PSK, .ssid_hidden = 0,
			.max_connection = 1, .beacon_interval = 100 } };

	esp_wifi_set_config(WIFI_IF_AP, &ap_config);

	esp_wifi_start();

	gpio_set_direction(A, GPIO_MODE_OUTPUT);
	gpio_set_direction(B, GPIO_MODE_OUTPUT);
	gpio_set_direction(C, GPIO_MODE_OUTPUT);
	gpio_set_direction(D, GPIO_MODE_OUTPUT);
	gpio_set_direction(OE, GPIO_MODE_OUTPUT);
	gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
	gpio_set_direction(R1, GPIO_MODE_OUTPUT);
	gpio_set_direction(R2, GPIO_MODE_OUTPUT);
	gpio_set_direction(STB, GPIO_MODE_OUTPUT);

	xTaskCreate(udp_receive, "udp_receive", 6000, NULL,
	configMAX_PRIORITIES - 1, NULL);

	xTaskCreate(run_cycle, "run_cycle", 6000, NULL,
	configMAX_PRIORITIES - 1, NULL);

	xTaskCreate(scan, "scan", 10000, NULL,
	configMAX_PRIORITIES - 2, NULL);

}

