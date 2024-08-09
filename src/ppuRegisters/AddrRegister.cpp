#include <ppuRegisters/AddrRegister.hpp>

AddrRegister::AddrRegister() : value{0, 0}, hi_ptr(true) {}

void AddrRegister::set(uint16_t data) {
    value[0] = (data >> 8) & 0xFF;
    value[1] = data & 0xFF;
}

void AddrRegister::update(uint8_t data) {
    if (hi_ptr) {
        value[0] = data;
    } else {
        value[1] = data;
    }

    if (get() > 0x3FFF) {
        set(get() & 0x3FFF);
    }

    hi_ptr = !hi_ptr;
}

void AddrRegister::increment(uint8_t inc) {
    uint8_t lo = value[1];
    value[1] = value[1] + inc;
    if (lo > value[1]) {
        value[0] = value[0] + 1;
    }
    if (get() > 0x3FFF) {
        set(get() & 0x3FFF);
    }
}

void AddrRegister::reset_latch() {
    hi_ptr = true;
}

uint16_t AddrRegister::get() const {
    return (static_cast<uint16_t>(value[0]) << 8) | value[1];
}