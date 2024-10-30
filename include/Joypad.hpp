#pragma once

#include <cstdint>

#define JOYPAD_A 0x01
#define JOYPAD_B 0x02
#define JOYPAD_SELECT 0x04
#define JOYPAD_START 0x08
#define JOYPAD_UP 0x10
#define JOYPAD_DOWN 0x20
#define JOYPAD_LEFT 0x40
#define JOYPAD_RIGHT 0x80

class Joypad {
public:
    Joypad();
    ~Joypad();

    void write(uint8_t data);
    uint8_t read();

    void setButtonState(uint8_t button, bool pressed);
private:
    uint8_t joypadState;
    uint8_t buttonIndex;
    bool strobe;
};