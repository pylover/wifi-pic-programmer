#ifndef _WEBADMIN_H__
#define _WEBADMIN_H__


#define WA_HTTPSERVER_PORT 80
#define WA_OK 0


typedef enum httpverb {
	GET,
	POST
} HTTPVerb;

typedef struct request {
	HTTPVerb verb;
	char *body;
	uint16_t body_length;
} Request;


typedef uint8_t Error;


void webadmin_initialize();
void webadmin_shutdown();

#endif
