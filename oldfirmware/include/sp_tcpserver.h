/* Serial Programmer main module's header */

#ifndef _SP_TCPSERVER_H__
#define _SP_TCPSERVER_H__

#include "sp.h"


#ifndef SP_TCPSERVER_PORT
#define SP_TCPSERVER_PORT	8585
#endif


typedef SPError (*SPRequestCallback)(SPPacket*);

void ICACHE_FLASH_ATTR
sp_tcpserver_cleanup_request();
	

void ICACHE_FLASH_ATTR
sp_tcpserver_response(int8_t, const char*, uint32_t);

#endif
