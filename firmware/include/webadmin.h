#ifndef _WEBADMIN_H__
#define _WEBADMIN_H__

#include <espconn.h>


#define WA_HTTPSERVER_PORT 80
#define OK 0


typedef enum httpverb {
	GET,
	POST
} HTTPVerb;

typedef struct request {
	HTTPVerb verb;
	char *body;
	uint16_t body_length;
} Request;

typedef err_t Error;


void webadmin_initialize();

#endif
