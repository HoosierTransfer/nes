#pragma once

#include <cstdint>

enum PPUControlFlags {
    NAMETABLE_X = 0x01,
    NAMETABLE_Y = 0x02,
    INCREMENT_MODE = 0x04,
    PATTERN_TABLE_SPRITE = 0x08,
    PATTERN_TABLE_BACKGROUND = 0x10,
    SPRITE_SIZE = 0x20,
    MASTER_SLAVE = 0x40,
    NMI = 0x80
};

class ControlRegister {
public:
    ControlRegister();

    void write(uint8_t data);

    uint16_t nametableAddress();

    uint8_t vramIncrement();

    uint16_t spritePatternTableAddress();

    uint16_t backgroundPatternTableAddress();

    uint8_t spriteSize();

    uint8_t masterSlave();

    bool generateNMI();

private:
    uint8_t flags;
};