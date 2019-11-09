# wifi-pic-programmer

An ESP8266 powered Microchip PIC programmer.


## Firmware

The firmware is implemented at top of the [ESP8266 NON-OS SDK V3](
https://github.com/espressif/ESP8266_NONOS_SDK).

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
make flash_map4 
```

For other information about ESP8266 flash maps check
[here](https://github.com/espressif/esptool#flash-modes)

### Firmware first boot

After flash and restart, you may found a wifi ssid: `WifiPicProg_xxxxxx`.

Connect to it with password `esp-8266`.

Browse the `http://192.168.43.1` and configure the device to connet to your
Wifi router.

### Test Wifi connection

use:

```bash
avahi-browse -var
```

to discover your new device, you may see an output like this:

```bash
Server version: avahi 0.7; Host name: XXXXXXX.local
E Ifce Prot Name                                          Type                 Domain
: Cache exhausted
+ wlp108s0 IPv4 WifiPicProg                                   _WifiPicProgServer._tcp local
= wlp108s0 IPv4 WifiPicProg                                   _WifiPicProgServer._tcp local
   hostname = [WifiPicProg.local]
   address = [192.168.5.107]
   port = [80]
   txt = ["version = 1.0.1" "vendor = Espressif"]
: All for now
```

That's it, the try the python tool to program your PIC.


#### ESP8266 GPIO

VPP(+12v):  D0   GPIO16 
VDD:        D4   GPIO02
data:       D2   GPIO04
clock:      D1   GPIO05

