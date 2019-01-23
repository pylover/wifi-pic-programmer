
static int _state;
static uint64_t _program_counter; 




// Send a command to the PIC.
void sendCommand(byte cmd)
{
    for (byte bit = 0; bit < 6; ++bit) {
        digitalWrite(PIN_CLOCK, HIGH);
        if (cmd & 1)
            digitalWrite(PIN_DATA, HIGH);
        else
            digitalWrite(PIN_DATA, LOW);
        os_delay_us(DELAY_TSET1);
        digitalWrite(PIN_CLOCK, LOW);
        os_delay_us(DELAY_THLD1);
        cmd >>= 1;
    }
}


// Send a command to the PIC that has no arguments.
void sendSimpleCommand(byte cmd) {
    sendCommand(cmd);
    os_delay_us(DELAY_TDLY2);
}


// Send a command to the PIC that writes a data argument.
void sendWriteCommand(byte cmd, unsigned int data) {
    sendCommand(cmd);
    os_delay_us(DELAY_TDLY2);
    for (byte bit = 0; bit < 16; ++bit) {
        digitalWrite(PIN_CLOCK, HIGH);
        if (data & 1)
            digitalWrite(PIN_DATA, HIGH);
        else
            digitalWrite(PIN_DATA, LOW);
        os_delay_us(DELAY_TSET1);
        digitalWrite(PIN_CLOCK, LOW);
        os_delay_us(DELAY_THLD1);
        data >>= 1;
    }
    os_delay_us(DELAY_TDLY2);
}


// Send a command to the PIC that reads back a data value.
uint32_t sendReadCommand(byte cmd) {
    unsigned int data = 0;
    sendCommand(cmd);
    digitalWrite(PIN_DATA, LOW);
    GPIO_INPUT(DATA_NUM);
    os_delay_us(DELAY_TDLY2);
    for (byte bit = 0; bit < 16; ++bit) {
        data >>= 1;
        GPIO_SET(CLOCK_NUM, HIGH);
        os_delay_us(DELAY_TDLY3);
        if (GPIO_GET(DATA_NUM))
            data |= 0x8000;

        GPIO_SET(CLOCK_NUM, LOW);
        os_delay_us(DELAY_THLD1);
    }
    GPIO_OUTPUT(DATA_NUM, 1);
    os_delay_us(DELAY_TDLY2);
    return data;
}


/*
 * Read a word from config memory using relative, non-flat, addressing.
 * Used by the "DEVICE" command to fetch information about devices whose
 * flat address ranges are presently unknown.
 */
uint32_t sp_pic_read_config_word(uint64_t addr)
{
    if (_state == STATE_IDLE) {
        // Enter programming mode and switch to config memory.
        _enter_program_mode();
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
    } else if (state == STATE_PROGRAM) {
        // Switch from program memory to config memory.
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
        _program_counter = 0;
    } else if (addr < pc) {
        // Need to go backwards in config memory, so reset the device.
        _exit_program_mode();
        _enter_program_mode();
        _send_write_command(CMD_LOAD_CONFIG, 0);
        _state = STATE_CONFIG;
    }
    while (pc < addr) {
        _send_simple_command(CMD_INCREMENT_ADDRESS);
        _program_counter++;
    }
    return (_send_read_command(CMD_READ_PROGRAM_MEMORY) >> 1) & 0x3FFF;
}

