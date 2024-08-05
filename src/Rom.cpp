#include <Rom.hpp>

#include <fstream>
#include <iostream>

using namespace Rom;

void Rom::loadRom(const char* path, Bus* memory) {
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::in);

    nes_header header;
    file.read((char*)&header, sizeof(nes_header));
    if (header.flag6 & 0x04) {
        file.seekg(512, std::ios::cur);
    }

    bool verticalMirroring = header.flag6 & 0x01;

    if (header.flag7 == 0x44) {
        header.flag7 = 0;
    }

    int mapper = ((header.flag6 & 0xf0) >> 4) + ((header.flag7 & 0xf0));

    int prgSize = header.prg_size * 0x4000;
    int chrSize = header.chr_size * 0x2000;

    uint8_t* prg_rom = new uint8_t[prgSize];
    uint8_t* chr_rom = new uint8_t[chrSize];
    file.read((char*)prg_rom, prgSize);
    file.read((char*)chr_rom, chrSize);

    if (mapper == 0) {
        memory->writeBytes(0x8000, prg_rom, prgSize);
        if (prgSize == 0x4000) {
            memory->writeBytes(0xC000, prg_rom, prgSize);
        }
    } else {
        std::cout << "Unsupported mapper: " << mapper << std::endl;
    }
}