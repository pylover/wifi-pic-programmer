#include "sp_mdns.h"
#include "sp_tcpserver.h"

#include <mem.h>
#include <c_types.h>
#include <ets_sys.h>
#include <ip_addr.h>
#include <espconn.h>
#include <user_interface.h>
#include <osapi.h>


static struct mdns_info mdns; 


ICACHE_FLASH_ATTR
void sp_mdns_setup() {
	wifi_set_broadcast_if(STATIONAP_MODE);

	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	
	os_memset(&mdns, 0, sizeof(struct mdns_info));
	mdns.host_name = DEVICE_NAME;
	mdns.ipAddr= ipconfig.ip.addr; //sation ip
	mdns.server_name = DEVICE_NAME"S";
	mdns.server_port = SP_TCPSERVER_PORT;
	mdns.txt_data[0] = "version = "SP_VERSION;
	espconn_mdns_init(&mdns);
}


