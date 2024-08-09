#pragma once

#include <cstdint>

class AddrRegister {
public:
    AddrRegister();

    void update(uint8_t data);
    void increment(uint8_t inc);
    void reset_latch();
    uint16_t get() const;

private:
    void set(uint16_t data);

    uint8_t value[2];
    bool hi_ptr;
};