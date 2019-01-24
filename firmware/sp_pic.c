
// Initialize device properties from the "devices" list and
// print them to the serial port.  Note: "dev" is in PROGMEM.
void initDevice(const struct deviceInfo *dev)
{
    // Update the global device details.
    programEnd = pgm_read_dword(&(dev->programSize)) - 1;
    configStart = pgm_read_dword(&(dev->configStart));
    configEnd = configStart + pgm_read_word(&(dev->configSize)) - 1;
    dataStart = pgm_read_dword(&(dev->dataStart));
    dataEnd = dataStart + pgm_read_word(&(dev->dataSize)) - 1;
    reservedStart = programEnd - pgm_read_word(&(dev->reservedWords)) + 1;
    reservedEnd = programEnd;
    configSave = pgm_read_word(&(dev->configSave));
    progFlashType = pgm_read_byte(&(dev->progFlashType));
    dataFlashType = pgm_read_byte(&(dev->dataFlashType));
    // Print the extra device information.
    Serial.print("DeviceName: ");
    printProgString((const prog_char *)(pgm_read_word(&(dev->name))));
    Serial.println();
    Serial.print("ProgramRange: 0000-");
    printHex8(programEnd);
    Serial.println();
    Serial.print("ConfigRange: ");
    printHex8(configStart);
    Serial.print('-');
    printHex8(configEnd);
    Serial.println();
    if (configSave != 0) {
        Serial.print("ConfigSave: ");
        printHex4(configSave);
        Serial.println();
    }
    Serial.print("DataRange: ");
    printHex8(dataStart);
    Serial.print('-');
    printHex8(dataEnd);
    Serial.println();
    if (reservedStart <= reservedEnd) {
        Serial.print("ReservedRange: ");
        printHex8(reservedStart);
        Serial.print('-');
        printHex8(reservedEnd);
        Serial.println();
    }
}
