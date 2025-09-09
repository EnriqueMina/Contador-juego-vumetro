#include "TM1638.h"

// Map segment table para dígitos 0-9
static const uint8_t SEGMENT_MAP[16] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0,0,0,0,0,0
};

TM1638::TM1638(PinName dio, PinName clk, PinName stb)
    : _dio(dio), _clk(clk), _stb(stb) {
    _dio.output();
    _clk = 1;
    _stb = 1;
}

void TM1638::init() {
    clearDisplay();
}

void TM1638::sendCommand(uint8_t data) {
    _stb = 0;
    shiftOut(data);
    _stb = 1;
}

void TM1638::sendData(uint8_t address, uint8_t data) {
    sendCommand(0x44); // modo fijo
    _stb = 0;
    shiftOut(0xC0 | address);
    shiftOut(data);
    _stb = 1;
}

void TM1638::shiftOut(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        _dio = (data >> i) & 1;
        _clk = 1;
    }
}

uint8_t TM1638::receive() {
    uint8_t temp = 0;
    _dio.input();
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        if (_dio.read()) temp |= (1 << i);
        _clk = 1;
    }
    _dio.output();
    return temp;
}

void TM1638::setBrightness(uint8_t brightness) {
    if (brightness > 7) brightness = 7;
    sendCommand(0x88 | brightness);
}

void TM1638::clearDisplay() {
    for (int i = 0; i < 16; i++) {
        sendData(i, 0x00);
    }
}

void TM1638::setLED(int pos, bool state) {
    if (pos < 0 || pos > 7) return;
    sendData((pos << 1) + 1, state ? 1 : 0);
}

void TM1638::setDisplayToString(const char *str) {
    for (int i = 0; i < 8; i++) {
        uint8_t seg = encodeChar(str[i]);
        sendData(i << 1, seg);
    }
}

void TM1638::setDisplayToDecNumber(int number) {
    char buf[9];
    sprintf(buf, "%08d", number);
    setDisplayToString(buf);
}

uint8_t TM1638::readButtons() {
    uint8_t buttons = 0;
    _stb = 0;
    shiftOut(0x42);
    for (int i = 0; i < 4; i++) {
        buttons |= (receive() << i);
    }
    _stb = 1;
    return buttons;
}

uint8_t TM1638::encodeChar(char c) {
    if (c >= '0' && c <= '9') {
        return SEGMENT_MAP[c - '0'];
    }
    return 0x00;
}
