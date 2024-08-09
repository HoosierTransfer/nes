#pragma once

#include <cstdint>
#include <cstddef>

#include <vector>

#include <ppuRegisters/AddrRegister.hpp>
#include <ppuRegisters/ControlRegister.hpp>
#include <ppuRegisters/MaskRegister.hpp>
#include <ppuRegisters/StatusRegister.hpp>
#include <ppuRegisters/ScrollRegister.hpp>

enum class Mirroring {
    HORIZONTAL,
    VERTICAL,
    FOUR_SCREEN
};

class PPU {
public:
    uint8_t* vram;
    uint8_t* oam;
    uint8_t* palette;
    std::vector<uint8_t> chrRom;
    uint8_t oamAddr;
    Mirroring mirroring;

    bool nmiInterrupt;

    ControlRegister* controlRegister;
    MaskRegister* maskRegister;
    StatusRegister* statusRegister;
    ScrollRegister* scrollRegister;
    AddrRegister* addrRegister;

    PPU();
    ~PPU();

    bool tick(uint8_t cycles);

    void setChrRom(uint8_t* chrRom, size_t size);

    void setMirroring(Mirroring mirroring);

    void writeToControlRegister(uint8_t data);
    void writeToMaskRegister(uint8_t data);
    void writeToOamAddress(uint8_t data);
    void writeToOamData(uint8_t data);
    void writeToScrollRegister(uint8_t data);
    void writeToAddrRegister(uint8_t data);
    void writeToDataRegister(uint8_t data);

    void writeToOamDma(uint8_t* data);

    uint8_t readFromStatusRegister();
    uint8_t readFromOamData();
    uint8_t readFromDataRegister();

    bool pollNmiInterrupt();
private:
    uint8_t dataBuffer;

    uint16_t mirrorVramAddress(uint16_t address);
    void incrementVramAddress();

    size_t cycles;
    uint16_t scanline;
};