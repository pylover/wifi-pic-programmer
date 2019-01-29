/* Serial Programmer main module's header */

#ifndef _SP_H__
#define _SP_H__

#include <c_types.h>

#ifndef SP_VERBOSE
#define SP_VERBOSE	true
#endif

#define SP_VERSION	"0.1.0"


// List of all commands that are understood by the programmer.
typedef enum {
	SP_CMD_ECHO = 1,
	SP_CMD_PROGRAMMER_VERSION,
	// Detect Device
	SP_CMD_DEVICE,
	// Reads program and data words from device memory (text)
	SP_CMD_READ, 
	// Reads program and data words from device memory (binary)
	SP_CMD_READBIN, 
	// Writes program and data words to device memory (text)
	SP_CMD_WRITE, 
	// Writes program and data words to device memory (binary)
	SP_CMD_WRITEBIN, 
	// Erases the contents of program, configuration, and data memory
	SP_CMD_ERASE, 
	// Returns a list of all supported device type
	SP_CMD_DEVICES, 
	// Sets a specific device type manually
	SP_CMD_SETDEVICE, 
	// Powers off the device in the programming socke
	SP_CMD_PWROFF
} SPCommand;


typedef struct {
	uint8_t command;
	uint32_t body_length;
} SPPacketHead;


typedef struct {
	SPPacketHead head;
	char *body;
} SPPacket;


typedef enum {
	SP_OK = 0,
	SP_ERR_INVALID_COMMAND,
	SP_ERR_REQ_LEN,
	SP_ERR_DEVICE_NOT_DETECTED,
} SPError;

#endif
