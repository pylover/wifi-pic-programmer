#include "sp_pic_io.h"

#include <c_types.h>


static int _state;
static uint64_t _program_counter; 


// Send a command to the PIC.
ICACHE_FLASH_ATTR
void _send_command(uint8_t cmd) {
	uint8_t bit;
    for (bit = 0; bit < 6; ++bit) {
        GPIO_SET(CLOCK_NUM, HIGH);
        if (cmd & 1)
            GPIO_SET(DATA_NUM, HIGH);
        else
            GPIO_SET(DATA_NUM, LOW);
        os_delay_us(DELAY_TSET1);
        GPIO_SET(CLOCK_NUM, LOW);
        os_delay_us(DELAY_THLD1);
        cmd >>= 1;
    }
}


// Send a command to the PIC that has no arguments.
ICACHE_FLASH_ATTR
void _send_simple_command(uint8_t cmd) {
    _send_command(cmd);
    os_delay_us(DELAY_TDLY2);
}


// Send a command to the PIC that writes a data argument.
ICACHE_FLASH_ATTR
void _send_write_command(uint8_t cmd, uint32_t data) {
	uint8_t bit;
    _send_command(cmd);
    os_delay_us(DELAY_TDLY2);
    for (bit = 0; bit < 16; ++bit) {
        GPIO_SET(CLOCK_NUM, HIGH);
        if (data & 1)
            GPIO_SET(DATA_NUM, HIGH);
        else
            GPIO_SET(DATA_NUM, LOW);
        os_delay_us(DELAY_TSET1);
        GPIO_SET(CLOCK_NUM, LOW);
        os_delay_us(DELAY_THLD1);
        data >>= 1;
    }
    os_delay_us(DELAY_TDLY2);
}


// Send a command to the PIC that reads back a data value.
ICACHE_FLASH_ATTR
uint32_t sendReadCommand(uint8_t cmd) {
	uint8_t bit;
    uint32_t data = 0;
    _send_command(cmd);
    GPIO_SET(DATA_NUM, LOW);
    GPIO_INPUT(DATA_NUM);
    os_delay_us(DELAY_TDLY2);
    for (bit = 0; bit < 16; ++bit) {
        data >>= 1;
        GPIO_SET(CLOCK_NUM, HIGH);
        os_delay_us(DELAY_TDLY3);
        if (GPIO_GET(DATA_NUM)) {
            data |= 0x8000;
		}

        GPIO_SET(CLOCK_NUM, LOW);
        os_delay_us(DELAY_THLD1);
    }
    GPIO_OUTPUT(DATA_NUM);
    os_delay_us(DELAY_TDLY2);
    return data;
}


/*
 * Read a word from config memory using relative, non-flat, addressing.
 * Used by the "DEVICE" command to fetch information about devices whose
 * flat address ranges are presently unknown.
 */
ICACHE_FLASH_ATTR
uint32_t sp_pic_read_config_word(uint64_t addr)
{
    if (_state == STATE_IDLE) {
        // Enter programming mode and switch to config memory.
        _enter_program_mode();
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
    } else if (_state == STATE_PROGRAM) {
        // Switch from program memory to config memory.
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
        _program_counter = 0;
    } else if (addr < _program_counter) {
        // Need to go backwards in config memory, so reset the device.
        _exit_program_mode();
        _enter_program_mode();
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
    }
    while (_program_counter < addr) {
        _send_simple_command(CMD_INCREMENT_ADDRESS);
        _program_counter++;
    }
    return (_send_read_command(CMD_READ_PROGRAM_MEMORY) >> 1) & 0x3FFF;
}


ICACHE_FLASH_ATTR
void sp_pic_inititialize() {
	PIN_FUNC_SELECT(DATA_MUX, DATA_FUNC);
	PIN_PULLUP_EN(DATA_MUX);
	GPIO_OUTPUT(DATA_NUM);
}


ICACHE_FLASH_ATTR
void sp_pic_shutdown() {
	// Do nothing here for now
}
