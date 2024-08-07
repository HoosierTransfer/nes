#pragma once

#include <cstdint>
#include <Memory.hpp>

namespace Rom {
struct nes_header
{
    uint8_t magic[4];       // 0x4E, 0x45, 0x53, 0x1A
    uint8_t prg_size;       // PRG ROM in 16K
    uint8_t chr_size;       // CHR ROM in 8K, 0 -> using CHR RAM
    uint8_t flag6;
    uint8_t flag7;
    uint8_t prg_ram_size;   // PRG RAM in 8K
    uint8_t flag9;
    uint8_t flag10;         // unofficial
    uint8_t reserved[5];    // reserved
};

void loadRom(const char* path, Memory* memory);
}