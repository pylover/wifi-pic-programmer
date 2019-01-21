
#include "programmer.h"

#include <mem.h>
#include <c_types.h>
#include <ets_sys.h>
#include <ip_addr.h>
#include <espconn.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>


static struct espconn * esp_conn;
static uint32_t sp_reading_bytes = 0;
static SPRequest sp_current_request;
static struct mdns_info mdns; 


static void ICACHE_FLASH_ATTR
sp_setup_mdns() {
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



static void ICACHE_FLASH_ATTR
sp_send_response(int8_t status, const char *response_buffer, 
		uint32_t response_length) {
	
	uint32_t total_length = 5 + response_length;

	// Allocate memory for send buffer	
    char *buffer = (char *)os_zalloc(total_length);
	
	// Status
	buffer[0] = status;

	// Copy 4 bytes for length 
	os_memcpy(&buffer[1], &response_length, 4);

	// Body, if provided
    if (response_length > 0) {
        os_memcpy(buffer + 5, response_buffer, response_length);
    }

	// Finally, send the buffer
	espconn_sent(esp_conn, buffer, total_length);
	
	// Free allocated memory
    os_free(buffer);
}


static SPError ICACHE_FLASH_ATTR
sp_command_echo(SPRequest *req) {
	sp_send_response(SP_OK, req->body, req->head.body_length);
	return SP_OK;
}


static void ICACHE_FLASH_ATTR
sp_cleanup_request() {
	// Cleaning up
	if (sp_current_request.body) {
		os_free(sp_current_request.body);
	}
	os_memset(&sp_current_request, 0, sizeof(SPRequest));
}


static SPError ICACHE_FLASH_ATTR
sp_process_request(SPRequest *req) {

#if SP_VERBOSE
	os_printf("Command: %d", req->head.command);
	if (req->head.body_length) {
		char body[req->head.body_length+1]; 
		os_strcpy(body, req->body);
		os_printf(" %s", body);
	}
	os_printf("\r\n");
#endif
	
	switch (req->head.command) {
		case SP_CMD_ECHO:
			return sp_command_echo(req);

		default:
			return SP_ERR_INVALID_COMMAND;
	}

	sp_cleanup_request(req);
	return SP_OK;
}


static SPError ICACHE_FLASH_ATTR
sp_read_request_body(const char *data, uint16_t length, SPRequest *req) {
	uint32_t remaining_bytes = req->head.body_length - sp_reading_bytes;
	
	if (remaining_bytes < length) {
		return SP_ERR_REQ_LEN;
	}

	os_memcpy(&(req->body[sp_reading_bytes]), data, length);  
	remaining_bytes -= length;
	
	if (remaining_bytes == 0) {
		sp_reading_bytes = 0;
		return sp_process_request(req);
	}
	sp_reading_bytes += length;
	return SP_OK;
}


static SPError ICACHE_FLASH_ATTR
sp_parse_request(const char *data, uint16_t length) {
	SPRequest *req = &sp_current_request; 
	
	// Append to read buffer if status is reading
	if (sp_reading_bytes) {
		return sp_read_request_body(data, length, req);	
	}


	// Process a new request
	if (length < 5) {
		return SP_ERR_REQ_LEN;
	}
	sp_cleanup_request();
	os_memcpy(&(req->head), data, 5);

	// Body, if available
	if (req->head.body_length) {
		req->body = (char*) os_zalloc(req->head.body_length);
		return sp_read_request_body(&data[5], length-5, req);
	}

	// Process request without body
	return sp_process_request(req);
}


static void ICACHE_FLASH_ATTR
sp_tcpserver_recv(void *arg, char *data, uint16_t length) {
	SPError err = sp_parse_request(data, length);
	if(SP_OK != err) {
		os_printf("Cannot parse request\r\n");
		sp_cleanup_request();
		sp_send_response(err, NULL, 0);
		return;
	}
}


static ICACHE_FLASH_ATTR
void sp_tcpserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = (struct espconn*) arg;
	// TODO: use macro for IPs
    os_printf("SP TCPSERVER: %d.%d.%d.%d:%d err %d reconnect\n", 
			pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port, 
			err
	);
}

static ICACHE_FLASH_ATTR
void sp_tcpserver_conn(void *arg)
{
    struct espconn *pesp_conn = (struct espconn*) arg;

    os_printf("SP TCPSERVER: %d.%d.%d.%d:%d connected\n", 
			pesp_conn->proto.tcp->remote_ip[0],
        	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
        	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port
	);
}


static ICACHE_FLASH_ATTR
void sp_tcpserver_disconnected(void *arg)
{
    struct espconn *pesp_conn = (struct espconn*) arg;

    os_printf("SP TCPSERVER: %d.%d.%d.%d:%d disconnect\n", 
			pesp_conn->proto.tcp->remote_ip[0],
        	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
        	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port
	);
}


static ICACHE_FLASH_ATTR
void sp_tcpserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;
    espconn_regist_connectcb(pesp_conn, sp_tcpserver_conn);
    espconn_regist_recvcb(pesp_conn, sp_tcpserver_recv);
    espconn_regist_reconcb(pesp_conn, sp_tcpserver_recon);
    espconn_regist_disconcb(pesp_conn, sp_tcpserver_disconnected);
}


void ICACHE_FLASH_ATTR
sp_initialize() {
	sp_setup_mdns();
	esp_conn = (struct espconn*) os_zalloc(sizeof(struct espconn));
    esp_conn->type = ESPCONN_TCP;
    esp_conn->state = ESPCONN_NONE;
    esp_conn->proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
    esp_conn->proto.tcp->local_port = SP_TCPSERVER_PORT;
    espconn_regist_connectcb(esp_conn, sp_tcpserver_listen);
    espconn_accept(esp_conn);
}


void ICACHE_FLASH_ATTR
sp_shutdown() {
	espconn_mdns_close();
	if (esp_conn) {
		espconn_abort(esp_conn);
		espconn_delete(esp_conn);
		os_free(esp_conn->proto.tcp);
		os_free(esp_conn);
		esp_conn = NULL;
	}
}
