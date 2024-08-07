#pragma once

#include <cstdint>

#include <vector>
#include <ppu/AddressRegister.hpp>
#include <ppu/ControlRegister.hpp>
#include <ppu/MaskRegister.hpp>
#include <ppu/ScrollRegister.hpp>
#include <ppu/StatusRegister.hpp>

enum class Mirroring {
    HORIZONTAL,
    VERTICAL,
    FOUR_SCREEN
};

class PPU {
public:
    std::vector<uint8_t> chrRom;
    uint8_t* paletteTable;
    uint8_t* vram;
    uint8_t* oam;

    Mirroring mirroring;

    PPU();

    void writeToOAMAddress(uint8_t data);
    void writeToAddressRegister(uint8_t data);
    void writeToControlRegister(uint8_t data);
    void writeToMaskRegister(uint8_t data);
    void writeToScrollRegister(uint8_t data);

    void incrementVramAddress();

    uint8_t readStatusRegister();

    uint8_t read();
    uint8_t readOAMData();
    void writeOAMData(uint8_t data);
    void write(uint8_t data);

    void writeOAMDMA(uint8_t* data);

    void setChrRom(uint8_t* data, int size);
    void setMirror(Mirroring mirror);

    bool tick(uint8_t cycles);

private:
    AddressRegister* addressRegister;
    ControlRegister* controlRegister;
    MaskRegister* maskRegister;
    ScrollRegister* scrollRegister;
    StatusRegister* statusRegister;

    uint16_t mirrorVramAddress(uint16_t address);

    uint8_t dataBuffer;
    uint8_t oamAddress;

    size_t cycles;
    uint16_t scanline;
};