#ifndef USER_WIFI_H_
#define USER_WIFI_H_

#include <user_interface.h>
#include <os_type.h>


#define WIFI_SOFTAP_CHANNEL		7
#define WIFI_SOFTAP_PSK			"esp-8266"
#define WIFI_VERBOSE			false

typedef void (*WifiCallback)(uint8_t);
typedef void (*TickCallback)(uint32_t);

void ICACHE_FLASH_ATTR 
wifi_initialize(const char *device_name, WifiCallback cb, TickCallback tcb);

struct dhcp_client_info {
	ip_addr_t ip_addr;
	ip_addr_t netmask;
	ip_addr_t gw;
	uint8 flag;
	uint8 pad[3];
};



#endif /* USER_WIFI_H_ */
