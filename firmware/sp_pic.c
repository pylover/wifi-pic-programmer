#include "sp_pic_io.h"
#include "sp_pic_devices.h"

#include <c_types.h>


static int _state;
static uint64_t _program_counter; 


// Flat address ranges for the various memory spaces.  Defaults to the values
// for the PIC16F628A.  "DEVICE" command updates to the correct values later.
static uint64_t programEnd			 = 0x07FF;
static uint64_t configStart  		 = 0x2000;
static uint64_t configEnd    		 = 0x2007;
static uint64_t dataStart    		 = 0x2100;
static uint64_t dataEnd      		 = 0x217F;
static uint64_t reservedStart		 = 0x0800;
static uint64_t reservedEnd  		 = 0x07FF;
static uint32_t configSave   		 = 0x0000;
static uint8_t progFlashType		= FLASH4;
static uint8_t dataFlashType		= EEPROM;


static ICACHE_FLASH_ATTR
void printHex1(unsigned int value) {
    if (value >= 10)
        os_printf("%s", (char)('A' + value - 10));
    else
        os_printf("%s", (char)('0' + value));
}


static ICACHE_FLASH_ATTR
void printHex4(unsigned int word) {
    printHex1((word >> 12) & 0x0F);
    printHex1((word >> 8) & 0x0F);
    printHex1((word >> 4) & 0x0F);
    printHex1(word & 0x0F);
}


static ICACHE_FLASH_ATTR
void printHex8(unsigned long word) {
    unsigned int upper = (unsigned int)(word >> 16);
    if (upper)
        printHex4(upper);
    printHex4((unsigned int)word);
}

// Enter high voltage programming mode.
static ICACHE_FLASH_ATTR
void _enter_program_mode() {
    // Bail out if already in programming mode.
    if (_state != STATE_IDLE)
        return;
    // Lower MCLR, VDD, DATA, and CLOCK initially.  This will put the
    // PIC into the powered-off, reset state just in case.
    GPIO_SET(MCLR_NUM, MCLR_RESET);
    GPIO_SET(VDD_NUM, LOW);
    GPIO_SET(DATA_NUM, LOW);
    GPIO_SET(CLOCK_NUM, LOW);
    // Wait for the lines to settle.
    os_delay_us(DELAY_SETTLE);
    // Switch DATA and CLOCK into outputs.
    GPIO_OUTPUT(DATA_NUM);
    GPIO_OUTPUT(CLOCK_NUM);
    // Raise MCLR, then VDD.
    GPIO_SET(MCLR_NUM, MCLR_VPP);
    os_delay_us(DELAY_TPPDP);
    GPIO_SET(VDD_NUM, HIGH);
    os_delay_us(DELAY_THLD0);
    // Now in program mode, starting at the first word of program memory.
    _state = STATE_PROGRAM;
    _program_counter = 0;
}


// Exit programming mode and reset the device.
static ICACHE_FLASH_ATTR
void _exit_program_mode() {
    // Nothing to do if already out of programming mode.
    if (_state == STATE_IDLE)
        return;
    // Lower MCLR, VDD, DATA, and CLOCK.
    GPIO_SET(MCLR_NUM, MCLR_RESET);
    GPIO_SET(VDD_NUM, LOW);
    GPIO_SET(DATA_NUM, LOW);
    GPIO_SET(CLOCK_NUM, LOW);
    // Float the DATA and CLOCK pins.
    GPIO_INPUT(DATA_NUM);
    GPIO_INPUT(CLOCK_NUM);
    // Now in the idle state with the PIC powered off.
    _state = STATE_IDLE;
    _program_counter = 0;
}


// Send a command to the PIC.
static ICACHE_FLASH_ATTR
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
static ICACHE_FLASH_ATTR
void _send_simple_command(uint8_t cmd) {
    _send_command(cmd);
    os_delay_us(DELAY_TDLY2);
}


// Send a command to the PIC that writes a data argument.
static ICACHE_FLASH_ATTR
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
static ICACHE_FLASH_ATTR
uint32_t _send_read_command(uint8_t cmd) {
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


// Set the program counter to a specific "flat" address.
static ICACHE_FLASH_ATTR
void _set_program_counter(unsigned long addr)
{
    if (addr >= dataStart && addr <= dataEnd) {
        // Data memory.
        addr -= dataStart;
        if (_state != STATE_PROGRAM || addr < _program_counter) {
            // Device is off, currently looking at configuration memory,
            // or the address is further back.  Reset the device.
            _exit_program_mode();
            _enter_program_mode();
        }
    } else if (addr >= configStart && addr <= configEnd) {
        // Configuration memory.
        addr -= configStart;
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
    } else {
        // Program memory.
        if (_state != STATE_PROGRAM || addr < _program_counter) {
            // Device is off, currently looking at configuration memory,
            // or the address is further back.  Reset the device.
            _exit_program_mode();
            _enter_program_mode();
        }
    }
    while (_program_counter < addr) {
        _send_simple_command(CMD_INCREMENT_ADDRESS);
        ++_program_counter;
    }
}
// Sets the PC for "erase mode", which is activated by loading the
// data value 0x3FFF into location 0 of configuration memory.
static ICACHE_FLASH_ATTR
void _set_erase_program_counter()
{
    // Forcibly reset the device so we know what state it is in.
    _exit_program_mode();
    _enter_program_mode();
    // Load 0x3FFF for the configuration.
    _send_write_command(CMD_LOAD_CONFIG, 0x3FFF);
    _state = STATE_CONFIG;
}


// Read a word from memory (program, config, or data depending upon addr).
// The start and stop bits will be stripped from the raw value from the PIC.
static ICACHE_FLASH_ATTR
unsigned int _read_word(unsigned long addr)
{
    _set_program_counter(addr);
    if (addr >= dataStart && addr <= dataEnd)
        return (_send_read_command(CMD_READ_DATA_MEMORY) >> 1) & 0x00FF;
    else
        return (_send_read_command(CMD_READ_PROGRAM_MEMORY) >> 1) & 0x3FFF;
}


/*
 * Read a word from config memory using relative, non-flat, addressing.
 * Used by the "DEVICE" command to fetch information about devices whose
 * flat address ranges are presently unknown.
 */
static ICACHE_FLASH_ATTR
uint32_t _read_config_word(uint64_t addr)
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

// Initialize device properties from the "devices" list and
// print them to the serial port.  Note: "dev" is in PROGMEM.
ICACHE_FLASH_ATTR
void _init_device(const struct deviceInfo *dev)
{
    // Update the global device details.
    programEnd = dev->programSize - 1;
    configStart = dev->configStart;
    configEnd = configStart + dev->configSize - 1;
    dataStart = dev->dataStart;
    dataEnd = dataStart + dev->dataSize - 1;
    reservedStart = programEnd - dev->reservedWords + 1;
    reservedEnd = programEnd;
    configSave = dev->configSave;
    progFlashType = dev->progFlashType;
    dataFlashType = dev->dataFlashType;
    // Print the extra device information.
	os_printf("DeviceName: %s", dev->name);
    os_printf("\r\n");
    os_printf("ProgramRange: 0000-");
    printHex8(programEnd);
    os_printf("\r\n");
    os_printf("ConfigRange: ");
    printHex8(configStart);
    os_printf("-");
    printHex8(configEnd);
    os_printf("\r\n");
    if (configSave != 0) {
        os_printf("ConfigSave: ");
        printHex4(configSave);
        os_printf("\r\n");
    }
    os_printf("DataRange: ");
    printHex8(dataStart);
    os_printf("-");
    printHex8(dataEnd);
    os_printf("\r\n");
    if (reservedStart <= reservedEnd) {
        os_printf("ReservedRange: ");
        printHex8(reservedStart);
		os_printf("-");
        printHex8(reservedEnd);
        os_printf("\r\n");
    }
}

// DEVICE command.
ICACHE_FLASH_ATTR
void sp_pic_cmd_device(const char *args)
{
    // Make sure the device is reset before we start.
    _exit_program_mode();
    // Read identifiers and configuration words from config memory.
    unsigned int userid0 = _read_config_word(DEV_USERID0);
    unsigned int userid1 = _read_config_word(DEV_USERID1);
    unsigned int userid2 = _read_config_word(DEV_USERID2);
    unsigned int userid3 = _read_config_word(DEV_USERID3);
    unsigned int deviceId = _read_config_word(DEV_ID);
    unsigned int configWord = _read_config_word(DEV_CONFIG_WORD);
    // If the device ID is all-zeroes or all-ones, then it could mean
    // one of the following:
    //
    // 1. There is no PIC in the programming socket.
    // 2. The VPP programming voltage is not available.
    // 3. Code protection is enabled and the PIC is unreadable.
    // 4. The PIC is an older model with no device identifier.
    //
    // Case 4 is the interesting one.  We look for any word in configuration
    // memory or the first 16 words of program memory that is non-zero.
    // If we find a non-zero word, we assume that we have a PIC but we
    // cannot detect what type it is.
    if (deviceId == 0 || deviceId == 0x3FFF) {
        unsigned int word = userid0 | userid1 | userid2 | userid3 | configWord;
        unsigned int addr = 0;
        while (!word && addr < 16) {
            word |= _read_word(addr);
            ++addr;
        }
        if (!word) {
            os_printf("ERROR\r\n");
            _exit_program_mode();
            return;
        }
        deviceId = 0;
    }
    os_printf("OK\r\n");
    os_printf("DeviceID: ");
    printHex4(deviceId);
    os_printf("\r\n");
    // Find the device in the built-in list if we have details for it.
    int index = 0;
    for (;;) {
        const char *name = (const char *)
            (devices[index].name);
        if (!name) {
            index = -1;
            break;
        }
        int id = devices[index].deviceId;
        if (id == (deviceId & 0xFFE0))
            break;
        ++index;
    }
    if (index >= 0) {
        _init_device(&(devices[index]));
    } else {
        // Reset the global parameters to their defaults.  A separate
        // "SETDEVICE" command will be needed to set the correct values.
        programEnd    = 0x07FF;
        configStart   = 0x2000;
        configEnd     = 0x2007;
        dataStart     = 0x2100;
        dataEnd       = 0x217F;
        reservedStart = 0x0800;
        reservedEnd   = 0x07FF;
        configSave    = 0x0000;
        progFlashType = FLASH4;
        dataFlashType = EEPROM;
    }
    os_printf("ConfigWord: ");
    printHex4(configWord);
    os_printf("\r\n");
    os_printf(".\r\n");
    // Don't need programming mode once the details have been read.
    _exit_program_mode();
}


ICACHE_FLASH_ATTR
void sp_pic_initialize() {
	PIN_FUNC_SELECT(DATA_MUX, DATA_FUNC);
	PIN_PULLUP_EN(DATA_MUX);
	GPIO_OUTPUT(DATA_NUM);
}


ICACHE_FLASH_ATTR
void sp_pic_shutdown() {
	// Do nothing here for now
}


