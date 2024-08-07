#include <ppu/AddressRegister.hpp>

AddressRegister::AddressRegister() {
    high = 0;
    low = 0;
    highLatch = false;
}

void AddressRegister::set(uint16_t data) {
    high = (data & 0xFF00) >> 8;
    low = data & 0x00FF;
}

void AddressRegister::write(uint8_t data) {
    if (highLatch) {
        high = data;
    } else {
        low = data;
    }

    if (get() > 0x3FFF) {
        set(get() & 0x3FFF);
    }

    highLatch = !highLatch;
}

void AddressRegister::increment(uint8_t increment) {
    uint8_t lo = low;
    low += increment;
    if (lo > low) {
        high += 1;
    }

    if (get() > 0x3FFF) {
        set(get() & 0x3FFF);
    }
}

void AddressRegister::resetLatch() {
    highLatch = false;
}

uint16_t AddressRegister::get() {
    return (high << 8) | low;
}