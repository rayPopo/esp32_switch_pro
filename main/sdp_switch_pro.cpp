#include "sdp_switch_pro.h"

static uint8_t hid_service_buffer[450];
static uint8_t device_id_sdp_service_buffer[200];
static uint8_t l2cap_sdp_service_buffer[600];
static const char hid_device_name[] = "Wireless Gamepad";

const uint8_t hid_descriptor_pro_controller[] = {
	0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x06, 0x01, 0xff, 0x85, 0x21, 0x09, 0x21, 0x75, 0x08, 0x95,
	0x30, 0x81, 0x02, 0x85, 0x30, 0x09, 0x30, 0x75, 0x08, 0x95, 0x30, 0x81, 0x02, 0x85, 0x31, 0x09,
	0x31, 0x75, 0x08, 0x96, 0x69, 0x01, 0x81, 0x02, 0x85, 0x32, 0x09, 0x32, 0x75, 0x08, 0x96, 0x69,
	0x01, 0x81, 0x02, 0x85, 0x33, 0x09, 0x33, 0x75, 0x08, 0x96, 0x69, 0x01, 0x81, 0x02, 0x85, 0x3f,
	0x05, 0x09, 0x19, 0x01, 0x29, 0x10, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x10, 0x81, 0x02,
	0x05, 0x01, 0x09, 0x39, 0x15, 0x00, 0x25, 0x07, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x05, 0x09,
	0x75, 0x04, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x33, 0x09, 0x34,
	0x16, 0x00, 0x00, 0x27, 0xff, 0xff, 0x00, 0x00, 0x75, 0x10, 0x95, 0x04, 0x81, 0x02, 0x06, 0x01,
	0xff, 0x85, 0x01, 0x09, 0x01, 0x75, 0x08, 0x95, 0x30, 0x91, 0x02, 0x85, 0x10, 0x09, 0x10, 0x75,
	0x08, 0x95, 0x30, 0x91, 0x02, 0x85, 0x11, 0x09, 0x11, 0x75, 0x08, 0x95, 0x30, 0x91, 0x02, 0x85,
	0x12, 0x09, 0x12, 0x75, 0x08, 0x95, 0x30, 0x91, 0x02, 0xc0

};

void device_id_sdp_record(uint8_t *service, uint32_t service_record_handle, uint16_t vendor_id_source, uint16_t vendor_id, uint16_t product_id, uint16_t version){
	//1200 Service Request: PNP info
	
	uint8_t* attribute;
	de_create_sequence(service);
	
	// 0x0000 "Service Record Handle"
	de_add_number(service, DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_RECORD_HANDLE);
	de_add_number(service, DE_UINT, DE_SIZE_32, service_record_handle);
	
	// 0x0001 "Service Class ID List"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_CLASS_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UUID, DE_SIZE_16, BLUETOOTH_SERVICE_CLASS_PNP_INFORMATION );
	}
	de_pop_sequence(service, attribute);
	//0x0004 "Protocol Descriptor List"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * l2cpProtocol = de_push_sequence(attribute);
		{
			de_add_number(l2cpProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_L2CAP);
			de_add_number(l2cpProtocol,  DE_UINT, DE_SIZE_16, 0x01);
		}
		de_pop_sequence(attribute, l2cpProtocol);
		
		uint8_t * hidProtocol = de_push_sequence(attribute);
		{
			de_add_number(hidProtocol,  DE_UUID, DE_SIZE_16, 0x01);
		}
		de_pop_sequence(attribute, hidProtocol);
	}
	de_pop_sequence(service, attribute);
	//0x0006 Lang Base Attribute ID
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_LANGUAGE_BASE_ATTRIBUTE_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x656e);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x006a);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x0100);
	}
	de_pop_sequence(service, attribute);
	//0x0009 Profile Descriptor List
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * hidProfile = de_push_sequence(attribute);
		{
			de_add_number(hidProfile,  DE_UUID, DE_SIZE_16, BLUETOOTH_SERVICE_CLASS_PNP_INFORMATION);
			de_add_number(hidProfile,  DE_UINT, DE_SIZE_16, 0x0100);	// Version 1.1
		}
		de_pop_sequence(attribute, hidProfile);
	}
	de_pop_sequence(service, attribute);

	//0x0100
	const char wgp2[] = "Wireless Gamepad PnP Server";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0100);
	de_add_data(service, DE_STRING, strlen(wgp2), (uint8_t *)wgp2);
	
	//Gamepad
	const char gp2[] = "Gamepad";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0101);
	de_add_data(service, DE_STRING, strlen(gp2), (uint8_t *)gp2);

	
	// 0x0200 "SpecificationID"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SPECIFICATION_ID);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0103);	// v1.3
	
	// 0x0201 "VendorID"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_VENDOR_ID);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x057E);
	
	// 0x0202 "ProductID"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PRODUCT_ID);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x2009);
	
	// 0x0203 "Version"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_VERSION);
	de_add_number(service,  DE_UINT, DE_SIZE_16, version);
	
	// 0x0204 "PrimaryRecord"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PRIMARY_RECORD);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  1);	// yes, this is the primary record - there are no others
	
	// 0x0205 "VendorIDSource"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_VENDOR_ID_SOURCE);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0002);
}

//Custom HID SDP record - taken from hid_device.c
void hid_sdp_record(
						   uint8_t *service,
						   uint32_t service_record_handle,
						   uint16_t hid_device_subclass,
						   uint8_t  hid_country_code,
						   uint8_t  hid_virtual_cable,
						   uint8_t  hid_reconnect_initiate,
						   uint8_t  hid_boot_device,
						   const uint8_t * descriptor, uint16_t descriptor_size,
						   const char *device_name){
	//1124 Service Request: Hunam Interface Device
	uint8_t * attribute;
	de_create_sequence(service);
	
	de_add_number(service, DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_RECORD_HANDLE);
	de_add_number(service, DE_UINT, DE_SIZE_32, service_record_handle);
	
	de_add_number(service, DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_CLASS_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UUID, DE_SIZE_16, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
	}
	de_pop_sequence(service, attribute);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * l2cpProtocol = de_push_sequence(attribute);
		{
			de_add_number(l2cpProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_L2CAP);
			de_add_number(l2cpProtocol,  DE_UINT, DE_SIZE_16, PSM_HID_CONTROL);
		}
		de_pop_sequence(attribute, l2cpProtocol);
		
		uint8_t * hidProtocol = de_push_sequence(attribute);
		{
			de_add_number(hidProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_HIDP);
		}
		de_pop_sequence(attribute, hidProtocol);
	}
	de_pop_sequence(service, attribute);
	
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_BROWSE_GROUP_LIST); // public browse group
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UUID, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PUBLIC_BROWSE_ROOT );
	}
	de_pop_sequence(service, attribute);
	

	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_LANGUAGE_BASE_ATTRIBUTE_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x656e);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x006a);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x0100);
	}
	de_pop_sequence(service, attribute);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * hidProfile = de_push_sequence(attribute);
		{
			de_add_number(hidProfile,  DE_UUID, DE_SIZE_16, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
			de_add_number(hidProfile,  DE_UINT, DE_SIZE_16, 0x0101);	// Version 1.1
		}
		de_pop_sequence(attribute, hidProfile);
	}
	de_pop_sequence(service, attribute);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS);
	attribute = de_push_sequence(service);
	{
		uint8_t * additionalDescriptorAttribute = de_push_sequence(attribute);
		{
			uint8_t * l2cpProtocol = de_push_sequence(additionalDescriptorAttribute);
			{
				de_add_number(l2cpProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_L2CAP);
				de_add_number(l2cpProtocol,  DE_UINT, DE_SIZE_16, PSM_HID_INTERRUPT);
			}
			de_pop_sequence(additionalDescriptorAttribute, l2cpProtocol);
			
			uint8_t * hidProtocol = de_push_sequence(additionalDescriptorAttribute);
			{
				de_add_number(hidProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_HIDP);
			}
			de_pop_sequence(additionalDescriptorAttribute, hidProtocol);
		}
		de_pop_sequence(attribute, additionalDescriptorAttribute);
	}
	de_pop_sequence(service, attribute);
	
	// 0x0100 "ServiceName"
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0100);
	de_add_data(service,  DE_STRING, strlen(device_name), (uint8_t *) device_name);
	
	const char gp[] = "Gamepad";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0101);
	de_add_data(service, DE_STRING, strlen(gp), (uint8_t *)gp);
	
	const char nin[] = "Nintendo";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0102);
	de_add_data(service, DE_STRING, strlen(nin), (uint8_t *)nin);
	
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_PARSER_VERSION);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0111);  // v1.1.1
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_DEVICE_SUBCLASS);
	de_add_number(service,  DE_UINT, DE_SIZE_8,  hid_device_subclass);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_COUNTRY_CODE);
	de_add_number(service,  DE_UINT, DE_SIZE_8,  hid_country_code);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_VIRTUAL_CABLE);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  1);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_RECONNECT_INITIATE);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  1);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t* hidDescriptor = de_push_sequence(attribute);
		{
			de_add_number(hidDescriptor,  DE_UINT, DE_SIZE_8, 0x22);	// Report Descriptor
			de_add_data(hidDescriptor,  DE_STRING, descriptor_size, (uint8_t *) descriptor);
		}
		de_pop_sequence(attribute, hidDescriptor);
	}
	de_pop_sequence(service, attribute);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HIDLANGID_BASE_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t* hig_lang_base = de_push_sequence(attribute);
		{
			// see: http://www.usb.org/developers/docs/USB_LANGIDs.pdf
			de_add_number(hig_lang_base,  DE_UINT, DE_SIZE_16, 0x0409);	// HIDLANGID = English (US)
			de_add_number(hig_lang_base,  DE_UINT, DE_SIZE_16, 0x0100);	// HIDLanguageBase = 0x0100 default
		}
		de_pop_sequence(attribute, hig_lang_base);
	}
	de_pop_sequence(service, attribute);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0209);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  1);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_REMOTE_WAKE);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  1);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x020C);
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0C80);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x20D);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  0);
	
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_HID_BOOT_DEVICE);
	de_add_number(service,  DE_BOOL, DE_SIZE_8,  0);
}

void l2cap_create_sdp_record(uint8_t *service)
{
	//0100 Service Request: L2CAP
	uint8_t * attribute;
	de_create_sequence(service);
	
	de_add_number(service, DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_RECORD_HANDLE);
	de_add_number(service, DE_UINT, DE_SIZE_32, 0x0);
	
	de_add_number(service, DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SERVICE_CLASS_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UUID, DE_SIZE_16, 0x1000);
	}
	de_pop_sequence(service, attribute);

	//0x0004 "Protocol Descriptor List"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * l2cpProtocol = de_push_sequence(attribute);
		{
			de_add_number(l2cpProtocol,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_L2CAP);
			de_add_number(l2cpProtocol,  DE_UINT, DE_SIZE_16, 0x01);
		}
		de_pop_sequence(attribute, l2cpProtocol);
		
		uint8_t * hidProtocol = de_push_sequence(attribute);
		{
			de_add_number(hidProtocol,  DE_UUID, DE_SIZE_16, 0x01);
		}
		de_pop_sequence(attribute, hidProtocol);
	}
	de_pop_sequence(service, attribute);

	// Public browse group
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_BROWSE_GROUP_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UUID, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_PUBLIC_BROWSE_ROOT );
	}
	de_pop_sequence(service, attribute);

	//0x0006 Lang Base Attribute ID
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_LANGUAGE_BASE_ATTRIBUTE_ID_LIST);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x656e);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x006a);
		de_add_number(attribute, DE_UINT, DE_SIZE_16, 0x0100);
	}
	de_pop_sequence(service, attribute);

	//0x0009 Profile Descriptor List
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
	attribute = de_push_sequence(service);
	{
		uint8_t * hidProfile = de_push_sequence(attribute);
		{
			de_add_number(hidProfile,  DE_UUID, DE_SIZE_16, BLUETOOTH_PROTOCOL_L2CAP);
			de_add_number(hidProfile,  DE_UINT, DE_SIZE_16, 0x0100);
		}
		de_pop_sequence(attribute, hidProfile);
	}
	de_pop_sequence(service, attribute);

	//0x0100
	const char wgp2[] = "Wireless Gamepad";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0100);
	de_add_data(service, DE_STRING, strlen(wgp2), (uint8_t *)wgp2);
	
	//Gamepad
	const char gp2[] = "Gamepad";
	de_add_number(service,  DE_UINT, DE_SIZE_16, 0x0101);
	de_add_data(service, DE_STRING, strlen(gp2), (uint8_t *)gp2);
	
	// 0x0200 "SpecificationID"
	de_add_number(service,  DE_UINT, DE_SIZE_16, BLUETOOTH_ATTRIBUTE_SPECIFICATION_ID);
	attribute = de_push_sequence(service);
	{
		de_add_number(attribute,  DE_UINT, DE_SIZE_16, 0x0100);
	}
	de_pop_sequence(service, attribute);
}

void sdp_switch_pro()
{	
	sdp_init();
	memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
	memset(device_id_sdp_service_buffer, 0, sizeof(device_id_sdp_service_buffer));

	//1200 Service Request: PNP info
	device_id_sdp_record(device_id_sdp_service_buffer, 0x10001, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
	sdp_register_service(device_id_sdp_service_buffer);

	//1124 Service Request: Hunam Interface Device - NOTE - comment out lines in sdp_server.c sdp_register_service() to allow for 0x10000 service handles
	hid_sdp_record(hid_service_buffer, 0x10000, 0x2508, 33, 0, 0, 1, hid_descriptor_pro_controller, sizeof(hid_descriptor_pro_controller), hid_device_name);
	sdp_register_service(hid_service_buffer);

	//0100 Service Request: L2CAP
	l2cap_create_sdp_record(l2cap_sdp_service_buffer);
	sdp_register_service(l2cap_sdp_service_buffer);
}