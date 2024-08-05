#include <Memory.hpp>

Memory::Memory() {
    memory = new uint8_t[0xFFFF];
    this->size = size;
}

Memory::~Memory() {
    delete[] memory;
}

void Memory::Zero() {
    for (int i = 0; i < size; i++) {
        memory[i] = 0;
    }
}

uint8_t Memory::read(uint16_t address) {
    if (address >= size) {
        std::cerr << "Address out of bounds" << std::endl;
        return 0;
    }
    return memory[address];
}

uint16_t Memory::readWord(uint16_t address) {
    if (address >= size - 1) {
        std::cerr << "Address out of bounds" << std::endl;
        return 0;
    }
    return memory[address] | (memory[address + 1] << 8);
}

void Memory::write(uint16_t address, uint8_t data) {
    if (address >= size) {
        std::cerr << "Address out of bounds" << std::endl;
        return;
    }
    memory[address] = data;
}

void Memory::writeWord(uint16_t address, uint16_t data) {
    if (address >= size - 1) {
        std::cerr << "Address out of bounds" << std::endl;
        return;
    }
    memory[address] = data & 0xFF;
    memory[address + 1] = (data >> 8) & 0xFF;
}

void Memory::writeBytes(uint16_t address, uint8_t* data, size_t size) {
    if (address + size >= this->size) {
        std::cerr << "Address out of bounds" << std::endl;
        return;
    }
    for (int i = 0; i < size; i++) {
        memory[address + i] = data[i];
    }
}
