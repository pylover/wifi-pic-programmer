#ifndef _PIC_DEVICES_H__
#define _PIC_DEVICES_H__


// Offsets of interesting config locations that contain device information.
#define DEV_USERID0         0
#define DEV_USERID1         1
#define DEV_USERID2         2
#define DEV_USERID3         3
#define DEV_ID              6
#define DEV_CONFIG_WORD     7


// List of devices that are currently supported and their properties.
// Note: most of these are based on published information and have not
// been tested by the author.  Patches welcome to improve the list.
struct deviceInfo
{
    const char *name;      // User-readable name of the device.
    int16_t deviceId;      // Device ID for the PIC (-1 if no id).
    uint32_t programSize;  // Size of program memory (words).
    uint32_t configStart;  // Flat address start of configuration memory.
    uint32_t dataStart;    // Flat address start of EEPROM data memory.
    uint16_t configSize;   // Number of configuration words.
    uint16_t dataSize;     // Size of EEPROM data memory (bytes).
    uint16_t reservedWords;// Reserved program words (e.g. for OSCCAL).
    uint16_t configSave;   // Bits in config word to be saved.
    uint8_t progFlashType; // Type of flash for program memory.
    uint8_t dataFlashType; // Type of flash for data memory.
};


// Flash types.  Uses a similar naming system to picprog.
#define EEPROM          0
#define FLASH           1
#define FLASH4          4
#define FLASH5          5



// Device names, forced out into PROGMEM.
#define DEV_PIC12F629 	"pic12f629"
#define DEV_PIC12F675  	"pic12f675"
#define DEV_PIC16F630  	"pic16f630"
#define DEV_PIC16F676  	"pic16f676"
#define DEV_PIC16F84	"pic16f84"
#define DEV_PIC16F84A 	"pic16f84a"
#define DEV_PIC16F87	"pic16f87"
#define DEV_PIC16F88	"pic16f88"
#define DEV_PIC16F627 	"pic16f627"
#define DEV_PIC16F627A	"pic16f627a"
#define DEV_PIC16F628 	"pic16f628"
#define DEV_PIC16F628A	"pic16f628a"
#define DEV_PIC16F648A	"pic16f648a"
#define DEV_PIC16F882 	"pic16f882"
#define DEV_PIC16F883  	"pic16f883"
#define DEV_PIC16F884  	"pic16f884"
#define DEV_PIC16F886  	"pic16f886"
#define DEV_PIC16F887  	"pic16f887"


static struct deviceInfo devices[] = {
    // http://ww1.microchip.com/downloads/en/DeviceDoc/41191D.pdf
    {DEV_PIC12F629,  0x0F80, 1024, 0x2000, 0x2100, 8, 128, 1, 0x3000, FLASH4, EEPROM},
    {DEV_PIC12F675,  0x0FC0, 1024, 0x2000, 0x2100, 8, 128, 1, 0x3000, FLASH4, EEPROM},
    {DEV_PIC16F630,  0x10C0, 1024, 0x2000, 0x2100, 8, 128, 1, 0x3000, FLASH4, EEPROM},
    {DEV_PIC16F676,  0x10E0, 1024, 0x2000, 0x2100, 8, 128, 1, 0x3000, FLASH4, EEPROM},
    // http://ww1.microchip.com/downloads/en/DeviceDoc/30262e.pdf
    {DEV_PIC16F84,   -1,     1024, 0x2000, 0x2100, 8,  64, 0, 0, FLASH,  EEPROM},
    {DEV_PIC16F84A,  0x0560, 1024, 0x2000, 0x2100, 8,  64, 0, 0, FLASH,  EEPROM},
    // http://ww1.microchip.com/downloads/en/DeviceDoc/39607c.pdf
    {DEV_PIC16F87,   0x0720, 4096, 0x2000, 0x2100, 9, 256, 0, 0, FLASH5, EEPROM},
    {DEV_PIC16F88,   0x0760, 4096, 0x2000, 0x2100, 9, 256, 0, 0, FLASH5, EEPROM},
    // 627/628:  http://ww1.microchip.com/downloads/en/DeviceDoc/30034d.pdf
    // A series: http://ww1.microchip.com/downloads/en/DeviceDoc/41196g.pdf
    {DEV_PIC16F627,  0x07A0, 1024, 0x2000, 0x2100, 8, 128, 0, 0, FLASH,  EEPROM},
    {DEV_PIC16F627A, 0x1040, 1024, 0x2000, 0x2100, 8, 128, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F628,  0x07C0, 2048, 0x2000, 0x2100, 8, 128, 0, 0, FLASH,  EEPROM},
    {DEV_PIC16F628A, 0x1060, 2048, 0x2000, 0x2100, 8, 128, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F648A, 0x1100, 4096, 0x2000, 0x2100, 8, 256, 0, 0, FLASH4, EEPROM},
    // http://ww1.microchip.com/downloads/en/DeviceDoc/41287D.pdf
    {DEV_PIC16F882,  0x2000, 2048, 0x2000, 0x2100, 9, 128, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F883,  0x2020, 4096, 0x2000, 0x2100, 9, 256, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F884,  0x2040, 4096, 0x2000, 0x2100, 9, 256, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F886,  0x2060, 8192, 0x2000, 0x2100, 9, 256, 0, 0, FLASH4, EEPROM},
    {DEV_PIC16F887,  0x2080, 8192, 0x2000, 0x2100, 9, 256, 0, 0, FLASH4, EEPROM},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif
