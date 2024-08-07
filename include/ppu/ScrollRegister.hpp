#pragma once

#include <cstdint>

class ScrollRegister {
public:
    bool latch;
    uint8_t x;
    uint8_t y;

    ScrollRegister();

    void write(uint8_t data);

    void resetLatch();
};