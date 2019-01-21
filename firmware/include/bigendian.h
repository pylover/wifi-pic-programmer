
#ifndef _BIGENDIAN_H__
#define _BIGENDIAN_H__

#include <c_types.h>


ICACHE_FLASH_ATTR
unsigned char * bigendian_serialize_uint32(unsigned char *buffer, 
		uint32_t value);

ICACHE_FLASH_ATTR
uint32_t bigendian_deserialize_uint32(const unsigned char *buffer);

#endif
