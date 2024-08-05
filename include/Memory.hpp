#pragma once

#include <cstdint>

// a bus but all the memory is in one piece
class Memory {
public:
    Memory();
    ~Memory();

    void Zero();
    uint8_t read(uint16_t address);
    uint16_t readWord(uint16_t address);
    void write(uint16_t address, uint8_t data);
    void writeWord(uint16_t address, uint16_t data);
    void writeBytes(uint16_t address, uint8_t* data, long size);
private:
    uint8_t* memory;
};