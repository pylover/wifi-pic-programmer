#ifndef _SP_PIC_IO_H__
#define _SP_PIC_IO_H__

#include <osapi.h>
#include <eagle_soc.h>
#include <gpio.h>


#define DATA_MUX			PERIPHS_IO_MUX_GPIO4_U
#define DATA_NUM			4
#define DATA_FUNC			FUNC_GPIO4

#define CLOCK_MUX			PERIPHS_IO_MUX_GPIO5_U
#define CLOCK_NUM			5
#define CLOCK_FUNC			FUNC_GPIO5

#define MCLR_MUX			PERIPHS_IO_MUX_GPIO6_U
#define MCLR_NUM			6
#define MCLR_FUNC			FUNC_GPIO6

#define VPP_MUX				PERIPHS_IO_MUX_GPIO7_U
#define VPP_NUM				7
#define VPP_FUNC			FUNC_GPIO7

#define VDD_MUX				PERIPHS_IO_MUX_GPIO7_U
#define VDD_NUM				7
#define VDD_FUNC			FUNC_GPIO7


#define LOW 0
#define HIGH 1
#define GPIO_SET(n, v) GPIO_OUTPUT_SET(GPIO_ID_PIN(n), v)
#define GPIO_GET(n) GPIO_INPUT_GET(GPIO_ID_PIN(n))
#define GPIO_INPUT(n) GPIO_DIS_OUTPUT(GPIO_ID_PIN(n))
#define GPIO_OUTPUT(n) GPIO_SET(n, LOW)


#define MCLR_RESET      HIGH    // PIN_MCLR state to reset the PIC
#define MCLR_VPP        LOW     // PIN_MCLR state to apply 13v to MCLR/VPP pin


// All delays are in microseconds.
#define DELAY_SETTLE    50      // Delay for lines to settle for reset
#define DELAY_TPPDP     5       // Hold time after raising MCLR
#define DELAY_THLD0     5       // Hold time after raising VDD
#define DELAY_TSET1     1       // Data in setup time before lowering clock
#define DELAY_THLD1     1       // Data in hold time after lowering clock
#define DELAY_TDLY2     1       // Delay between commands or data
#define DELAY_TDLY3     1       // Delay until data bit read will be valid
#define DELAY_TPROG     4000    // Time for a program memory write to complete
#define DELAY_TDPROG    6000    // Time for a data memory write to complete
#define DELAY_TERA      6000    // Time for a word erase to complete
#define DELAY_TPROG5    1000    // Time for program write on FLASH5 systems
#define DELAY_TFULLERA  50000   // Time for a full chip erase
#define DELAY_TFULL84   20000   // Intermediate wait for PIC16F84/PIC16F84A


// States this application may be in.
#define STATE_IDLE      0       // Idle, device is held in the reset state
#define STATE_PROGRAM   1       // Active, reading and writing program memory
#define STATE_CONFIG    2       // Active, reading and writing config memory


// Commands that may be sent to the device.
#define CMD_LOAD_CONFIG         0x00    // Load (write) to config memory
#define CMD_LOAD_PROGRAM_MEMORY 0x02    // Load to program memory
#define CMD_LOAD_DATA_MEMORY    0x03    // Load to data memory
#define CMD_INCREMENT_ADDRESS   0x06    // Increment the PC
#define CMD_READ_PROGRAM_MEMORY 0x04    // Read from program memory
#define CMD_READ_DATA_MEMORY    0x05    // Read from data memory
#define CMD_BEGIN_PROGRAM       0x08    // Begin programming with erase cycle
#define CMD_BEGIN_PROGRAM_ONLY  0x18    // Begin programming only cycle
#define CMD_END_PROGRAM_ONLY    0x17    // End programming only cycle
#define CMD_BULK_ERASE_PROGRAM  0x09    // Bulk erase program memory
#define CMD_BULK_ERASE_DATA     0x0B    // Bulk erase data memory
#define CMD_CHIP_ERASE


#endif
