/* Serial Programmer main module */

#include "sp.h"
#include "sp_mdns.h"
#include "sp_tcpserver.h"

#include <osapi.h>


static SPError ICACHE_FLASH_ATTR
sp_command_echo(SPPacket *req) {
	sp_tcpserver_response(SP_OK, req->body, req->head.body_length);
	return SP_OK;
}


static SPError ICACHE_FLASH_ATTR
sp_process_request(SPPacket *req) {

#if SP_VERBOSE
	os_printf("Command: %d", req->head.command); 
	if (req->head.body_length) {
		char body[req->head.body_length+1]; 
		os_strcpy(body, req->body);
		os_printf(" len: %d body: %s", req->head.body_length, body);
	}
	os_printf("\r\n");
#endif
	
	switch (req->head.command) {
		case SP_CMD_ECHO:
			return sp_command_echo(req);

		default:
			return SP_ERR_INVALID_COMMAND;
	}

	sp_tcpserver_cleanup_request(req);
	return SP_OK;
}


void ICACHE_FLASH_ATTR
sp_initialize() {
	sp_mdns_setup();
	sp_tcpserver_initialize(sp_process_request);
}


void ICACHE_FLASH_ATTR
sp_shutdown() {
	espconn_mdns_close();
	sp_tcpserver_shutdown();
}
