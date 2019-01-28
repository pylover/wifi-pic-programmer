#include "sp_pic_io.h"
#include "pic_devices.h"
#include "pic.h"

#include <c_types.h>


// DEVICE command.
ICACHE_FLASH_ATTR
void sp_pic_command_detect(const char *args) {
    // Make sure the device is reset before we start.
    pic_exit_program_mode();
	
	os_printf("Reading configuration...");

    // Read identifiers and configuration words from config memory.
    uint32_t userid0 = pic_read_config_word(DEV_USERID0);
    uint32_t userid1 = pic_read_config_word(DEV_USERID1);
    uint32_t userid2 = pic_read_config_word(DEV_USERID2);
    uint32_t userid3 = pic_read_config_word(DEV_USERID3);
    uint32_t deviceId = pic_read_config_word(DEV_ID);
    uint32_t configWord = pic_read_config_word(DEV_CONFIG_WORD);

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
        uint32_t word = userid0 | userid1 | userid2 | userid3 | configWord;
        uint32_t addr = 0;
        while (!word && addr < 16) {
            word |= pic_read_word(addr);
            ++addr;
        }
        if (!word) {
            os_printf("ERROR\r\n");
            pic_exit_program_mode();
            return;
        }
        deviceId = 0;
    }
    os_printf("OK\r\n");
    os_printf("DeviceID: %02X\r\n", deviceId);
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
        pic_init_device(&(devices[index]));
    } else {
		os_printf("No device detected\r\n");
		pic_reset_device();
    }
    os_printf("ConfigWord: %02X\r\n", configWord);
    os_printf(".\r\n");
    // Don't need programming mode once the details have been read.
    pic_exit_program_mode();
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

