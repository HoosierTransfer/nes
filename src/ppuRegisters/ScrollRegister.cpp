#include <ppuRegisters/ScrollRegister.hpp>

ScrollRegister::ScrollRegister() : scroll_x(0), scroll_y(0), latch(false) {}

void ScrollRegister::write(uint8_t data) {
    if (!latch) {
        scroll_x = data;
    } else {
        scroll_y = data;
    }
    latch = !latch;
}

void ScrollRegister::reset_latch() {
    latch = false;
}