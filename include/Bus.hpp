#pragma once

#include <cstdint>

#include <PPU.hpp>

class Bus {
public:
    Bus(PPU* ppu);
    ~Bus();

    void write(uint16_t address, uint8_t data);
    void writeWord(uint16_t address, uint16_t data);

    uint8_t read(uint16_t address);
    uint16_t readWord(uint16_t address);

    void writeBytes(uint16_t address, uint8_t* data, int size);

    void Zero();

private:
    uint8_t* cpuMemory;
    uint8_t* prgMemory;
    PPU* ppu;
};