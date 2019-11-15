#include "programmer.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>
#include <osapi.h>


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
#define CMD_CHIP_ERASE          0x1F    // Erase the entire chip


#define vppon()    GPIO_OUTPUT_SET(VPP_NUM, 0)
#define vppoff()    GPIO_OUTPUT_SET(VPP_NUM, 1)

#define vddon()    GPIO_OUTPUT_SET(VDD_NUM, 1)
#define vddoff()    GPIO_OUTPUT_SET(VDD_NUM, 0)

#define clockhigh()  GPIO_OUTPUT_SET(CLOCK_NUM, 1)
#define clocklow()   GPIO_OUTPUT_SET(CLOCK_NUM, 0)

#define datahigh()   GPIO_OUTPUT_SET(DATA_NUM, 1)
#define datalow()    GPIO_OUTPUT_SET(DATA_NUM, 0)
#define dataset(v)    GPIO_OUTPUT_SET(DATA_NUM, v)

#define DELAYUNIT   1

#define delaysettle()    os_delay_us(DELAYUNIT * 50)      // Delay for lines to settle for reset
#define delaytppdp()     os_delay_us(DELAYUNIT * 5)       // Hold time after raising MCLR
#define delaythld0()     os_delay_us(DELAYUNIT * 5)       // Hold time after raising VDD
#define delaytset1()     os_delay_us(DELAYUNIT * 1)       // Data in setup time before lowering clock
#define delaythld1()     os_delay_us(DELAYUNIT * 1)       // Data in hold time after lowering clock
#define delaytdly2()     os_delay_us(DELAYUNIT * 1)       // Delay between commands or data
#define delaytdly3()     os_delay_us(DELAYUNIT * 1)       // Delay until data bit read will be valid
//#define delaytprog()     os_delay_us(4000)    // Time for a program memory write to complete
//#define delaytdprog()    os_delay_us(6000)    // Time for a data memory write to complete
//#define delaytera()      os_delay_us(6000)    // Time for a word erase to complete
//#define delaytprog5()    os_delay_us(1000)    // Time for program write on FLASH5 systems
//#define delaytfullera()  os_delay_us(50000)   // Time for a full chip erase
//#define delaytfull84()   os_delay_us(20000)   // Intermediate wait for PIC16F84/PIC16F84A


static bool programming = false;


ICACHE_FLASH_ATTR
static _begin_programming() {
    if (programming) {
        return;
    }

    vppoff();
    vddoff();
    clocklow();
    datalow();
    
    delaysettle();
    vppon();
    delaytppdp();
    vddon();
    delaythld0();
    programming = true;
}


ICACHE_FLASH_ATTR
static _end_programming() {
    vddoff();
    vppoff();
}


// Send a command to the PIC.
static void _send_command(unsigned char cmd) {
    unsigned char bit;
    for (bit = 0; bit < 6; ++bit) {
        clockhigh();
        dataset(cmd & 1);
        delaytset1();
        clocklow();
        delaythld1();
        cmd >>= 1;
        if (!(cmd & 1)) {
            datalow();
        }
    }
}


static void _send_write_command(unsigned char cmd, unsigned int data) {
    unsigned char bit;
    _send_command(cmd);
    delaytdly2();
    for (bit = 0; bit < 16; ++bit) {
        clockhigh();
        dataset(data & 1);
        delaytset1();
        clocklow();
        delaythld1();
        data >>= 1;
        if (!(data & 1)) {
            datalow();
        }
    }
    delaytdly2();
}



ICACHE_FLASH_ATTR
int programmer_detect(int *chip) {
    _begin_programming();
    _send_write_command(CMD_LOAD_CONFIG, 0); 
    _end_programming();
    return 0; 
}

ICACHE_FLASH_ATTR
void programmer_init() {

	// GPIO Initialization
	ETS_GPIO_INTR_DISABLE();

	PIN_PULLUP_DIS(VPP_MUX);
	PIN_FUNC_SELECT(VPP_MUX, VPP_FUNC);
    vppoff();

	PIN_PULLUP_DIS(VDD_MUX);
	PIN_FUNC_SELECT(VDD_MUX, VDD_FUNC);
    vddoff();

	PIN_PULLUP_DIS(DATA_MUX);
	PIN_FUNC_SELECT(DATA_MUX, DATA_FUNC);
    datalow();

	PIN_PULLUP_DIS(CLOCK_MUX);
	PIN_FUNC_SELECT(CLOCK_MUX, CLOCK_FUNC);
    clocklow();

	ETS_GPIO_INTR_ENABLE();

    /*
	ETS_GPIO_INTR_ATTACH(gpio_intr, NULL);
	gpio_pin_intr_state_set(GPIO_ID_PIN(FOTABTN_NUM), GPIO_PIN_INTR_ANYEDGE);
    */

}

