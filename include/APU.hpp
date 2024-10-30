#pragma once

#include <cstdint>

class APU {
public:
    APU();
    ~APU();

    void writeStatus(uint8_t value);
    uint8_t readStatus();

private:
    uint8_t status;
};