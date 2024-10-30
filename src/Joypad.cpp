#include <Joypad.hpp>

Joypad::Joypad() {
    joypadState = 0;
    buttonIndex = 0;
    strobe = false;
}

Joypad::~Joypad() {
}

void Joypad::write(uint8_t data) {
    strobe = data & 0x01;
    if (strobe) {
        buttonIndex = 0;
    }
}

uint8_t Joypad::read() {
    if (buttonIndex > 7) {
        return 1;
    }
    uint8_t result = (joypadState & (1 << buttonIndex)) >> buttonIndex;
    if (!strobe) {
        buttonIndex++;
    }
    return result;
}

void Joypad::setButtonState(uint8_t button, bool pressed) {
    if (pressed) {
        joypadState |= button;
    } else {
        joypadState &= ~button;
    }
}
