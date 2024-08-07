#include <ppu/ScrollRegister.hpp>

ScrollRegister::ScrollRegister() {
    latch = false;
    x = 0;
    y = 0;
}

void ScrollRegister::write(uint8_t data) {
    if (latch) {
        y = data;
    } else {
        x = data;
    }

    latch = !latch;
}

void ScrollRegister::resetLatch() {
    latch = false;
}