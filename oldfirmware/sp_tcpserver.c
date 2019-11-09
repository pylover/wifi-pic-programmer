#include "sp_tcpserver.h"
#include "bigendian.h"

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
static SPPacket sp_current_request;
static SPRequestCallback sp_request_callback = NULL;



static SPError ICACHE_FLASH_ATTR
_process_request(SPPacket *req) {
	if (sp_request_callback != NULL) {
		return sp_request_callback(req);
	}
}


static SPError ICACHE_FLASH_ATTR
_read_request_body(const unsigned char *data, uint16_t length, SPPacket *req) {
	uint32_t remaining_bytes = req->head.body_length - sp_reading_bytes;
	
	if (remaining_bytes < length) {
		return SP_ERR_REQ_LEN;
	}

	os_memcpy(&(req->body[sp_reading_bytes]), data, length);  
	remaining_bytes -= length;
	
	if (remaining_bytes == 0) {
		sp_reading_bytes = 0;
		return _process_request(req);
	}
	sp_reading_bytes += length;
	return SP_OK;
}


static SPError ICACHE_FLASH_ATTR
_parse_request(const unsigned char *data, uint16_t length) {
	SPPacket *req = &sp_current_request; 
	
	// Append to read buffer if status is reading
	if (sp_reading_bytes) {
		return _read_request_body(data, length, req);	
	}

	// Process a new request
	if (length < 5) {
		return SP_ERR_REQ_LEN;
	}
	sp_tcpserver_cleanup_request();
	req->head.command = data[0];
	req->head.body_length = bigendian_deserialize_uint32(data+1);
	//os_memcpy(&(req->head), data, 5);

	// Body, if available
	if (req->head.body_length) {
		req->body = (char*) os_zalloc(req->head.body_length);
		return _read_request_body(&data[5], length-5, req);
	}

	// Process request without body
	return _process_request(req);
}



static void ICACHE_FLASH_ATTR
_receive(void *arg, char *data, uint16_t length) {
	SPError err = _parse_request((unsigned char*)data, length);
	if(SP_OK != err) {
		os_printf("Cannot parse request\r\n");
		sp_tcpserver_cleanup_request();
		sp_tcpserver_response(err, NULL, 0);
		return;
	}
}


static ICACHE_FLASH_ATTR
void _client_reconnect(void *arg, sint8 err)
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
void _client_disconnected(void *arg)
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
void _client_connected(void *arg) {
    struct espconn *pesp_conn = arg;

    os_printf("SP TCPSERVER: %d.%d.%d.%d:%d connected\n", 
			pesp_conn->proto.tcp->remote_ip[0],
        	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
        	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port
	);

    espconn_regist_recvcb(pesp_conn, _receive);
    espconn_regist_reconcb(pesp_conn, _client_reconnect);
    espconn_regist_disconcb(pesp_conn, _client_disconnected);
}


void ICACHE_FLASH_ATTR
sp_tcpserver_cleanup_request() {
	// Cleaning up
	if (sp_current_request.body) {
		os_free(sp_current_request.body);
	}
	os_memset(&sp_current_request, 0, sizeof(SPPacket));
}


void ICACHE_FLASH_ATTR
sp_tcpserver_response(int8_t status, const char *buffer, uint32_t length) {
	
	uint32_t total_length = 5 + length;

	// Allocate memory for send buffer	
    unsigned char *tcpbuffer = (unsigned char *)os_zalloc(total_length);
	
	// Copy 5 bytes of head 
	tcpbuffer[0] = status;
	bigendian_serialize_uint32(tcpbuffer + 1, length);

	// Body, if provided
    if (length > 0) {
        os_memcpy(tcpbuffer + 5, buffer, length);
    }

	// Finally, send the buffer
	espconn_sent(esp_conn, tcpbuffer, total_length);
	
	// Free allocated memory
    os_free(tcpbuffer);
}


ICACHE_FLASH_ATTR
void sp_tcpserver_initialize(SPRequestCallback request_callback) {
	esp_conn = (struct espconn*) os_zalloc(sizeof(struct espconn));
    esp_conn->type = ESPCONN_TCP;
    esp_conn->state = ESPCONN_NONE;
    esp_conn->proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
    esp_conn->proto.tcp->local_port = SP_TCPSERVER_PORT;
    espconn_regist_connectcb(esp_conn, _client_connected);
	sp_request_callback = request_callback;
    espconn_accept(esp_conn);
}


void ICACHE_FLASH_ATTR
sp_tcpserver_shutdown() {
	sp_tcpserver_cleanup_request();
	if (esp_conn) {
		espconn_abort(esp_conn);
		espconn_delete(esp_conn);
		os_free(esp_conn->proto.tcp);
		os_free(esp_conn);
		esp_conn = NULL;
	}
}
