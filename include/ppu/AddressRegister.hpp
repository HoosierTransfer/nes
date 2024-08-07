#pragma once

#include <cstdint>

class AddressRegister {
public:
    AddressRegister();

    void set(uint16_t data);
    void write(uint8_t data);
    void increment(uint8_t increment);
    void resetLatch();
    uint16_t get();
private:
    uint8_t high;
    uint8_t low;
    bool highLatch;
};
