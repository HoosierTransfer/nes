#include <ppu/ControlRegister.hpp>
#include <stdexcept>

ControlRegister::ControlRegister() {
    flags = 0;
}

void ControlRegister::write(uint8_t data) {
    flags = data;
}

uint16_t ControlRegister::nametableAddress() {
    switch (flags & 0b11) {
        case 0:
            return 0x2000;
        case 1:
            return 0x2400;
        case 2:
            return 0x2800;
        case 3:
            return 0x2c00;
        default:
            throw std::logic_error("not possible");
    }
}

uint8_t ControlRegister::vramIncrement() {
    return (flags & INCREMENT_MODE) ? 32 : 1;
}

uint16_t ControlRegister::spritePatternTableAddress() {
    return (flags & PATTERN_TABLE_SPRITE) ? 0x1000 : 0;
}

uint16_t ControlRegister::backgroundPatternTableAddress() {
    return (flags & PATTERN_TABLE_BACKGROUND) ? 0x1000 : 0;
}

uint8_t ControlRegister::spriteSize() {
    return (flags & SPRITE_SIZE) ? 16 : 8;
}

uint8_t ControlRegister::masterSlave() {
    return (flags & MASTER_SLAVE) ? 1 : 0;
}

bool ControlRegister::generateNMI() {
    return (flags & NMI) ? true : false;
}
