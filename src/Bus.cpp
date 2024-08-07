#include <Bus.hpp>
#include <iostream>

#define RAM 0x0000
#define RAM_MIRRORS_END 0x1FFF
#define PPU_REGISTERS 0x2000
#define PPU_REGISTERS_MIRRORS_END 0x3FFF

Bus::Bus(PPU* ppu) {
    cpuMemory = new uint8_t[2048];
    prgMemory = new uint8_t[0xFFFF-0x8000];
    this->ppu = ppu;
}   

Bus::~Bus() {
    delete[] cpuMemory;
    delete[] prgMemory;
}

void Bus::Zero() {
    for (int i = 0; i < 0x0800; i++) {
        cpuMemory[i] = 0;
    }
    for (int i = 0; i < 0xFFFF-0x8000; i++) {
        prgMemory[i] = 0;
    }
}

uint8_t Bus::read(uint16_t address) {
    if (address >= RAM && address <= RAM_MIRRORS_END) {
        return cpuMemory[address & 0x07FF];
    } else if (address == 0x2000 || address == 0x2001 || address == 0x2003 || address == 0x2005 || address == 0x2006 || address == 0x4014) {
        throw std::runtime_error("Attempt to read from write-only PPU address");
    } else if (address == 0x2002) {
        return ppu->readStatusRegister();
    } else if (address == 0x2004) {
        return ppu->readOAMData();
    } else if (address == 0x2007) {
        return ppu->read();
    } else if (address >= 0x2008 && address <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirrorDown = address & 0x2007;
        return read(mirrorDown);
    } if (address >= 0x8000 && address <= 0xFFFF) {
        return prgMemory[address - 0x8000];
    } else {
        std::cerr << "Address not implemented" << std::endl;
        return 0;
    }
}

uint16_t Bus::readWord(uint16_t address) {
    return read(address) | (read(address + 1) << 8);
}

void Bus::write(uint16_t address, uint8_t data) {
    if (address >= RAM && address <= RAM_MIRRORS_END) {
        cpuMemory[address & 0x07FF] = data;
    } else if (address == 0x2000) {
        ppu->writeToControlRegister(data);
    } else if (address == 0x2001) {
        ppu->writeToMaskRegister(data);
    } else if (address == 0x2002) {
        throw std::runtime_error("Attempt to write to read-only PPU address");
    } else if (address == 0x2003) {
        ppu->writeToOAMAddress(data);
    } else if (address == 0x2004) {
        ppu->writeOAMData(data);
    } else if (address == 0x2005) {
        ppu->writeToScrollRegister(data);
    } else if (address == 0x2006) {
        ppu->writeToAddressRegister(data);
    } else if (address == 0x2007) {
        ppu->write(data);
    } else if (address >= 0x2008 && address <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirrorDown = address & 0x2007;
        write(mirrorDown, data);
    } else if (address >= 0x8000 && address <= 0xFFFF) {
        prgMemory[address - 0x8000] = data;
    } else {
        std::cerr << "Address not implemented" << std::endl;
    }
}

void Bus::writeWord(uint16_t address, uint16_t data) {
    write(address, data & 0xFF);
    write(address + 1, data >> 8);
}

void Bus::writeBytes(uint16_t address, uint8_t* data, int size) {
    for (int i = 0; i < size; i++) {
        write(address + i, data[i]);
    }
}