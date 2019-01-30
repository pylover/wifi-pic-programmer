#include "bigendian.h"

#include <user_interface.h>


ICACHE_FLASH_ATTR
unsigned char * bigendian_serialize_uint32(unsigned char *buffer, 
		uint32_t value) {
  /*  Write big-endian int value into buffer; 
   *  assumes 32-bit int and 8-bit char. */
  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}


ICACHE_FLASH_ATTR
uint32_t bigendian_deserialize_uint32(const unsigned char *buffer) {
	uint32_t r = buffer[0] << 24;
	r |= buffer[1] << 16;
	r |= buffer[2] << 8;
	r |= buffer[3];
	return r;
}


