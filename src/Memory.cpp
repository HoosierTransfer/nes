#include <Memory.hpp>

#include <iostream>

Memory::Memory() {
    memory = new uint8_t[0xFFFF];
}

Memory::~Memory() {
    delete[] memory;
}

void Memory::Zero() {
    for (int i = 0; i < 0xffff; i++) {
        memory[i] = 0;
    }
}

uint8_t Memory::read(uint16_t address) {
    return memory[address];
}

uint16_t Memory::readWord(uint16_t address) {
    return memory[address] | (memory[address + 1] << 8);
}

void Memory::write(uint16_t address, uint8_t data) {
    memory[address] = data;
}

void Memory::writeWord(uint16_t address, uint16_t data) {
    memory[address] = data & 0xFF;
    memory[address + 1] = (data >> 8) & 0xFF;
}

void Memory::writeBytes(uint16_t address, uint8_t* data, long size) {
    for (int i = 0; i < size; i++) {
        memory[address + i] = data[i];
    }
}
