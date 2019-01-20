#include "wifi.h"
#include "user_config.h"

#include <user_interface.h>
#include <osapi.h>
#include <espconn.h>
#include <mem.h>
#include <ets_sys.h>
#include <c_types.h>
#include <os_type.h>


static ETSTimer wifi_timer;
static uint8_t wifiStatus = STATION_IDLE;
static uint8_t lastWifiStatus = STATION_IDLE;
WifiCallback wifi_cb = NULL;
TickCallback tick_cb = NULL;
static uint32_t ticks = 0;


static void ICACHE_FLASH_ATTR 
wifi_check_ip(void *arg) {
	struct ip_info ipConfig;

	os_timer_disarm(&wifi_timer);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0) {
		if (tick_cb) 
			tick_cb(ticks++);

		os_timer_setfn(&wifi_timer, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&wifi_timer, 2000, 0);
	}
	else {
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD) {
			os_printf("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND) {
			os_printf("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL) {
			os_printf("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else {
#if WIFI_VERBOSE
			os_printf("STATION_IDLE\r\n");
#endif
		}

		os_timer_setfn(&wifi_timer, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&wifi_timer, 1000, 0);
	}

	if(wifiStatus != lastWifiStatus){
		lastWifiStatus = wifiStatus;
		if(wifi_cb)
			wifi_cb(wifiStatus);
	}
}


static void ICACHE_FLASH_ATTR 
wifi_init_softap(const char *device_name) {
	uint8_t mac[6];

	/***add by tzx for saving ip_info to avoid dhcp_client start****/
    struct dhcp_client_info dhcp_info;
    struct ip_info sta_info;
    system_rtc_mem_read(64, &dhcp_info, sizeof(struct dhcp_client_info));
	if(dhcp_info.flag == 0x01 ) {
		if (true == wifi_station_dhcpc_status()) {
			wifi_station_dhcpc_stop();
		}
		sta_info.ip = dhcp_info.ip_addr;
		sta_info.gw = dhcp_info.gw;
		sta_info.netmask = dhcp_info.netmask;
		if ( true != wifi_set_ip_info(STATION_IF, &sta_info)) {
			os_printf("set default ip wrong\n");
		}
	}
    os_memset(&dhcp_info, 0, sizeof(struct dhcp_client_info));
    system_rtc_mem_write(64, &dhcp_info, sizeof(struct rst_info));


	// Get the device mac address
	bool ok = wifi_get_macaddr(SOFTAP_IF, &mac[0]);
	if (!ok) {
		os_printf("Cannot get softap macaddr\r\n");
	}

	// initialization
	// TODO: free ?
    struct softap_config *config = (struct softap_config *) \
			os_zalloc(sizeof(struct softap_config));

	// Get soft-AP config first.
	wifi_softap_get_config(config);     

	// Updating ssid and password
	os_sprintf(config->ssid, "%s_%02x%02x%02x%02x%02x%02x", 
			device_name,
			MAC2STR(mac));
	os_printf("SSID: %s\r\n", config->ssid);
    config->ssid_len = 0; 
    os_sprintf(config->password, WIFI_SOFTAP_PSK);
    config->authmode = AUTH_WPA_WPA2_PSK;
    config->max_connection = 4;
	config->channel = 5;	
	config->beacon_interval = 120;

	// Set ESP8266 soft-AP config
    ok = wifi_softap_set_config(config); 
    os_free(config);
	if (!ok) {
		os_printf("Cannot set softap config\r\n");
		return;
	}

    struct station_info * station = wifi_softap_get_station_info();
    while (station) {
        os_printf("bssid : MACSTR, ip : IPSTR/n", MAC2STR(station->bssid), 
				IP2STR(&station->ip));
        station = STAILQ_NEXT(station, next);
    }

	// Free it by calling functionss
    wifi_softap_free_station_info(); 
    wifi_softap_dhcps_stop(); // disable soft-AP DHCP server
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 43, 1); // set IP
    IP4_ADDR(&info.gw, 192, 168, 43, 1); // set gateway
    IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
    wifi_set_ip_info(SOFTAP_IF, &info);
    struct dhcps_lease dhcp_lease;
    IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 43, 100);
    IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 43, 105);
    wifi_softap_set_dhcps_lease(&dhcp_lease);
    wifi_softap_dhcps_start(); // enable soft-AP DHCP server

}


void ICACHE_FLASH_ATTR 
wifi_initialize(const char *device_name, WifiCallback cb, TickCallback tcb) {
	struct station_config stationConf;
	wifi_init_softap(device_name);
	wifi_set_opmode_current(STATIONAP_MODE);
	wifi_set_sleep_type(NONE_SLEEP_T);
	wifi_cb = cb;
	tick_cb = tcb;

	//os_memset(&stationConf, 0, sizeof(struct station_config));
	//os_sprintf(stationConf.ssid, "%s", ssid);
	//os_sprintf(stationConf.password, "%s", pass);
	//wifi_station_set_config_current(&stationConf);

	os_timer_disarm(&wifi_timer);
	os_timer_setfn(&wifi_timer, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&wifi_timer, 1000, 0);
	wifi_station_set_auto_connect(true);
	wifi_station_connect();
}

