#include <Bus.hpp>
#include <iostream>

#define RAM 0x0000
#define RAM_MIRRORS_END 0x1FFF
#define PPU_REGISTERS 0x2000
#define PPU_REGISTERS_MIRRORS_END 0x3FFF

Bus::Bus() {
    cpuMemory = new uint8_t[2048];
    prgMemory = new uint8_t[0xFFFF-0x8000];
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
    } else if (address >= PPU_REGISTERS && address <= PPU_REGISTERS_MIRRORS_END) {
        std::cerr << "PPU registers not implemented" << std::endl;
        return 0;
    } else if (address >= 0x8000 && address <= 0xFFFF) {
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
    } else if (address >= PPU_REGISTERS && address <= PPU_REGISTERS_MIRRORS_END) {
        std::cerr << "PPU registers not implemented" << std::endl;
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