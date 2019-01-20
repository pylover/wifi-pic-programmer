
// Internal 
#include "partition.h"
#include "wifi.h"
#include "user_config.h"
#include "webadmin.h"

// SDK
#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include <user_interface.h>
#include <driver/uart.h>
#include <upgrade.h>


static void ICACHE_FLASH_ATTR 
tick_cb(uint32_t ticks) {
	//os_printf("%d\r\n", ticks);
}


static void ICACHE_FLASH_ATTR
setup_mdns() {
	wifi_set_broadcast_if (STATIONAP_MODE);
	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	
	struct mdns_info *info = \
				(struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
	info->host_name = DEVICE_NAME;
	info->ipAddr= ipconfig.ip.addr; //sation ip
	info->server_name = DEVICE_NAME"Server";
	info->server_port = 80;
	info->txt_data[0] = "version = 1.0.1";
	espconn_mdns_init(info);
}


static void ICACHE_FLASH_ATTR 
wifi_connect_cb(uint8_t status) {
    if(status == STATION_GOT_IP) {
		os_printf("Wifi connected\r\n");
		setup_mdns();
    } else {
		os_printf("Wifi disconnected\r\n");
    }
}


void ICACHE_FLASH_ATTR 
user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);
    wifi_initialize(DEVICE_NAME, wifi_connect_cb, tick_cb);
	webadmin_init();
    os_printf("System started ...\r\n");
}


void ICACHE_FLASH_ATTR 
user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, 
				sizeof(at_partition_table)/sizeof(at_partition_table[0]),
				SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

