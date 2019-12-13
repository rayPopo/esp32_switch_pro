#define __BTSTACK_FILE__ "main.c"

#include <inttypes.h>
#include <stdio.h>

#include "btstack_config.h"
#include "btstack.h"
#include "btstack_stdin.h"
#include "esp_log.h"
#include "sdp_switch_pro.h"
#include "FreeRTOS.h"
#include <M5Stack.h>

static uint16_t sp_channel = 0;
static uint8_t switch_pro_timer = 0;
static bool start_send = false;
static bd_addr_t fc30_bdaddr = {0x98, 0xb6, 0xe9, 0x42, 0x83, 0xFD};
//static uint8_t hid_packet[13] = {0xa1, 0x3f, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80};
//static uint8_t hid_packet[13] = {0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t hid_packet[13] = {0xA1, 0x3F, 0x00, 0x00, 0x08, 0x10, 0x00, 0xEF, 0xFF, 0x10, 0x00, 0xFF, 0xFF};

static void stdin_process(char character)
{
	static const char *TAG = "stdin_process";

	ESP_LOGI(TAG, "%c", character);

	switch (character)
	{
		case 'a':
		{
			ESP_LOGI("Wii","Connecting Wii");
			break;
		}
		case 'p':
		{
			ESP_LOGI(TAG, "gap_delete_all_link_keys");
			gap_delete_all_link_keys();
			break;
		}
	}
}

static void send_test()
{
	if(!start_send)
		return;

	if(!sp_channel)
		return;

	if(!l2cap_reserve_packet_buffer())
		return;

	uint8_t * packet_new = l2cap_get_outgoing_buffer();

/*
	hid_packet[2] = switch_pro_timer;

	switch_pro_timer++;
	
	if (switch_pro_timer == 255)
		switch_pro_timer = 0;
*/

	memcpy(packet_new, hid_packet, 13);
	l2cap_send_prepared(sp_channel, 13);
}

static void send_task(void *pvParameters)
{
	for(;;)
	{
		send_test();
  	vTaskDelay(15);
	}
	vTaskDelete(NULL);
}

static void hid_int_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size)
{
	UNUSED(packet_type);
	UNUSED(channel);
	UNUSED(packet);
	UNUSED(packet_size);

	switch (packet_type)
	{
		case L2CAP_DATA_PACKET:
		{
ESP_LOGI("INT", "DATA");
printf("INT <== ");
for(uint16_t i=0;i<packet_size;i++)
	printf("%02x ",packet[i]);
printf("\n");
			break;
		}

		case HCI_EVENT_PACKET:
		{
			switch (packet[0])
			{
				case L2CAP_EVENT_INCOMING_CONNECTION:
					ESP_LOGI("INT", "Incoming");
					l2cap_accept_connection(channel);
					//gap_delete_all_link_keys();
					start_send = true;
					break;

				case L2CAP_EVENT_CHANNEL_OPENED:
				{
					if(l2cap_event_channel_opened_get_status(packet))
					{
						ESP_LOGW("INT", "L2CAP Connection failed");
						break;
					}
					ESP_LOGI("INT", "Connected");
					sp_channel = channel;
					//l2cap_request_can_send_now_event(channel);
					break;
				}

				case L2CAP_EVENT_CHANNEL_CLOSED:
					sp_channel = 0;
					start_send = false;
					ESP_LOGI("INT", "Closed");
					//gap_delete_all_link_keys();
					switch_pro_timer = 0;
					gap_discoverable_control(0);
					break;

				case L2CAP_EVENT_CAN_SEND_NOW:
					//l2cap_send(channel, hid_packet, 13);
					//l2cap_request_can_send_now_event(channel);
					break;

				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

static void hid_cmd_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size)
{
	UNUSED(packet_type);
	UNUSED(channel);
	UNUSED(packet);
	UNUSED(packet_size);

	switch (packet_type)
	{
		case HCI_EVENT_PACKET:
		{
			switch (packet[0])
			{
				case L2CAP_EVENT_INCOMING_CONNECTION:
					//ESP_LOGI("CMD", "Incoming");
					l2cap_accept_connection(channel);
					break;

				case L2CAP_EVENT_CHANNEL_OPENED:
					if(l2cap_event_channel_opened_get_status(packet))
					{
						ESP_LOGW("CMD", "L2CAP Connection failed");
						break;
					}
					//ESP_LOGI("CMD", "Connected");
					break;

				case L2CAP_EVENT_CHANNEL_CLOSED:
					//ESP_LOGI("CMD", "Closed");
					sp_channel = 0;
					start_send = false;
					break;

				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

extern "C" {
	int btstack_main(int argc, const char * argv[]);
}

int btstack_main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;

	M5.begin(true, false, false, false);

	M5.Lcd.setTextColor(WHITE, BLACK);
	M5.Lcd.setTextSize(2);
	M5.Lcd.setCursor(0, 0);
	M5.Lcd.print("esp32_switch_pro");

	//gap_delete_all_link_keys();

	//fc30_bdaddr[5] = fc30_bdaddr[5] - 2;
	//esp_base_mac_addr_set(fc30_bdaddr);

	gap_set_class_of_device(0x002508);
	gap_set_local_name("Pro Controller");
	gap_connectable_control(1);
	gap_discoverable_control(1);
	//hci_set_master_slave_policy(0);
	//gap_set_default_link_policy_settings(2);
	//gap_set_bondable_mode(1);
	//gap_ssp_set_io_capability(0);
	//gap_ssp_set_authentication_requirement(SSP_IO_AUTHREQ_MITM_PROTECTION_NOT_REQUIRED_DEDICATED_BONDING);

	l2cap_init();

	sdp_switch_pro();

	l2cap_register_service(hid_int_packet_handler, PSM_HID_INTERRUPT, 700, LEVEL_0);
	l2cap_register_service(hid_cmd_packet_handler, PSM_HID_CONTROL,   700, LEVEL_0);

	hci_dump_open(NULL, HCI_DUMP_STDOUT);

	//xTaskCreatePinnedToCore(send_task, "send_task", 10240, NULL, 70, NULL, 1);

	btstack_stdin_setup(stdin_process);

	hci_power_control(HCI_POWER_ON);

	return 0;
}