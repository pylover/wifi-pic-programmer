
#include "fotabtn.h"

#include <c_types.h>
#include <eagle_soc.h>
#include <gpio.h>
#include <upgrade.h>
#include <osapi.h>


static bool btn = false;


void gpio_intr(void *arg) {
	unsigned int gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	bool pressed = !GPIO_INPUT_GET(GPIO_ID_PIN(FOTABTN_NUM)); 
	if (pressed ^ btn) {
		btn = pressed;
		if (pressed) {
			os_printf("FOTA Button pressed, REBOOTING...\r\n");
			system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			system_upgrade_reboot();
		}
	}
	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
}


ICACHE_FLASH_ATTR
void fotabtn_init() {
	// GPIO
	ETS_GPIO_INTR_DISABLE();

	PIN_PULLUP_EN(FOTABTN_MUX);
	PIN_FUNC_SELECT(FOTABTN_MUX, FOTABTN_FUNC);
	GPIO_DIS_OUTPUT(FOTABTN_NUM);
	ETS_GPIO_INTR_ATTACH(gpio_intr, NULL);
	gpio_pin_intr_state_set(GPIO_ID_PIN(FOTABTN_NUM), GPIO_PIN_INTR_ANYEDGE);
	ETS_GPIO_INTR_ENABLE();
}

