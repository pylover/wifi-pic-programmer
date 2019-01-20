# wifi-pic-programmer

An ESP8266 powered Microchip PIC programmer.


## Firmware

The firmware is implemented at top of the [ESP8266 NON-OS SDK V3]
(https://github.com/espressif/ESP8266_NONOS_SDK).

### Build firmware

Follow [this](https://github.com/easyqiot/esp-env) instruction to set up your 
environment.

Then:

```bash
cd path/to/wifi-pic-programmer
make flash_map2 
```

Other available flash maps:

```bash
make flash_map3 
make flash_map5 
```

For other information about ESP8266 flash maps check
[here](https://github.com/espressif/esptool#flash-modes)


