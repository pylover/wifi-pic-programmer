#include "wifi.h"
#include "webadmin.h"


#include <user_interface.h>
#include <osapi.h>
#include <espconn.h>
#include <mem.h>
#include <ets_sys.h>
#include <c_types.h>
#include <os_type.h>


#define FB_RESPONSE_HEADER_FORMAT \
	"HTTP/1.0 200 OK\r\n"\
	"Content-Length: %d\r\n"\
	"Server: lwIP/1.4.0\r\n"\
	"Content-type: text/html\r\n"\
	"Expires: Fri, 10 Apr 2008 14:00:00 GMT\r\n"\
	"Pragma: no-cache\r\n\r\n"

#define FB_BAD_REQUEST_FORMAT \
	"HTTP/1.0 400 BadRequest\r\n"\
	"Content-Length: 0\r\n"\
	"Server: lwIP/1.4.0\r\n"

#define HTML_HEADER \
	"<!DOCTYPE html><html>" \
	"<head><title>ESP8266 Firstboot config</title></head><body>\r\n" 
#define HTML_FOOTER "\r\n</body></html>\r\n"
#define HTML_FORM \
	HTML_HEADER \
	"<form method=\"post\">" \
	"SSID: <input name=\"ssid\" value=\"%s\"/><br/>" \
	"PSK: <input name=\"psk\" value=\"%s\"/><br/>" \
	"<input type=\"submit\" value=\"Connect\" />" \
	"</form>" \
	HTML_FOOTER


static struct espconn esp_conn;
static esp_tcp esptcp;


static void ICACHE_FLASH_ATTR
webadmin_send_response(bool ok, const char *response_buffer) {
	uint16_t total_length = 0;
	uint16_t head_length = 0;
    char *send_buffer = NULL;
    char httphead[256];
    os_memset(httphead, 0, 256);
	uint16_t response_length = (ok && response_buffer != NULL) ? \
		os_strlen(response_buffer): 0;

	os_sprintf(
			httphead, 
			ok? FB_RESPONSE_HEADER_FORMAT: FB_BAD_REQUEST_FORMAT, 
			response_length
		);
	head_length = os_strlen(httphead);	
    total_length = head_length + response_length;
    send_buffer = (char *)os_zalloc(total_length + 1);
	// Write head
    os_memcpy(send_buffer, httphead, head_length);

	// Body
    if (response_length > 0) {
        os_memcpy(send_buffer+head_length, response_buffer, response_length);
    }

	espconn_sent(&esp_conn, send_buffer, total_length);
    os_free(send_buffer);
}


static void ICACHE_FLASH_ATTR
webadmin_serve_form() {
	struct station_config station_conf;
	char *buffer = (char*) os_zalloc(1024);
	char *ssid;
	char *psk;

	bool ok = wifi_station_get_config(&station_conf);
	if (ok) {
		os_sprintf(
				buffer, 
				HTML_FORM, 
				(char *)station_conf.ssid,
				(char *)station_conf.password
			);
	}
	else {
		os_sprintf(buffer, HTML_FORM, "", "");
	}

	webadmin_send_response(true, buffer);
    os_free(buffer);
}


static void ICACHE_FLASH_ATTR
webadmin_update_params_field(struct station_config *wifi_config, 
		const char *field, const char *value) {
	os_printf("Updating Field: %s with value: %s\r\n", field, value);
	char *target;
	if (os_strcmp(field, "ssid") == 0) {
		target = (char*)&wifi_config->ssid;
	}
	else if (os_strcmp(field, "psk") == 0) {
		target = (char*)&wifi_config->password;
	}
	else return;
	os_sprintf(target, "%s", value);
}


static void ICACHE_FLASH_ATTR
webadmin_parse_form(const char *form, struct station_config *wifi_config) {
	char *field = (char*)&form[0];
	char *value;
	char *tmp;

	while (true) {
		value = os_strstr(field, "=") + 1;
		(value-1)[0] = 0;
		tmp  = os_strstr(value, "&");
		if (tmp != NULL) {
			tmp[0] = 0;
		}
		webadmin_update_params_field(wifi_config, field, value);
		if (tmp == NULL) {
			return;
		}
		field = tmp + 1;
	}
}


static Error ICACHE_FLASH_ATTR
webadmin_parse_request(char *data, uint16_t length, Request *req) {
	char *cursor;
	if (os_strncmp(data, "GET", 3) == 0) {
		req->verb = GET;
		req->body_length = 0;
		req->body =  NULL; 	
		return OK;
	}
	
	if (os_strncmp(data, "POST", 4) == 0) {
		req->verb = POST;
		req->body = (char*)os_strstr(data, "\r\n\r\n");
		if (req->body == NULL) {
			goto error;
		}
		req->body += 4;
		req->body_length = length - (req->body - data);	
		return OK;
	}

error:
	req->body_length = 0;
	webadmin_send_response(false, NULL);
	return 1;

}


static void ICACHE_FLASH_ATTR
webadmin_webserver_recv(void *arg, char *data, uint16_t length) {
	Request req;
	if(OK != webadmin_parse_request(data, length, &req)) {
		return;
	}

	os_printf("--> Verb: %s Length: %d Body: %s\r\n", 
			req.verb == 0 ? "GET" : "POST", 
			req.body_length, 
			req.body
	);
	if (req.verb == GET) {
		webadmin_serve_form();
	}
	else {
		struct station_config station_conf;
		os_memset(&station_conf, 0, sizeof(struct station_config));
		webadmin_parse_form(req.body, &station_conf);
		if(!wifi_station_set_config(&station_conf)) {
			os_printf("Cannot save WIFI params\r\n");
			webadmin_send_response(false, "Error Saving WIFI parameters");
			return;
		}
		
		os_printf("WIFI params updated sucessfully.\r\n");
		webadmin_send_response(true, "Update Successfull.");
		wifi_station_connect();
		//system_restart();
	}
}


static ICACHE_FLASH_ATTR
void webadmin_webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;
	// TODO: use macro for IPs
    os_printf("webserver's %d.%d.%d.%d:%d err %d reconnect\n", 
			pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port, 
			err
	);
}


static ICACHE_FLASH_ATTR
void webadmin_webserver_disconnected(void *arg)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d disconnect\n", 
			pesp_conn->proto.tcp->remote_ip[0],
        	pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
        	pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port
	);
}


static ICACHE_FLASH_ATTR
void webadmin_webserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;
    espconn_regist_recvcb(pesp_conn, webadmin_webserver_recv);
    espconn_regist_reconcb(pesp_conn, webadmin_webserver_recon);
    espconn_regist_disconcb(pesp_conn, webadmin_webserver_disconnected);
}


void ICACHE_FLASH_ATTR
webadmin_init() {
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = 80;
    espconn_regist_connectcb(&esp_conn, webadmin_webserver_listen);
    espconn_accept(&esp_conn);
}

