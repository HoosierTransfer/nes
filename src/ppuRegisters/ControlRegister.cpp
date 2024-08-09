#include <ppuRegisters/ControlRegister.hpp>

#include <stdexcept>

ControlRegister::ControlRegister() : bits(0b00000000) {}

bool ControlRegister::contains(uint8_t flag) const {
    return (bits & flag) != 0;
}

uint16_t ControlRegister::nametable_addr() const {
    switch (bits & 0b11) {
        case 0: return 0x2000;
        case 1: return 0x2400;
        case 2: return 0x2800;
        case 3: return 0x2c00;
        default: throw std::runtime_error("not possible");
    }
}

uint8_t ControlRegister::vram_addr_increment() const {
    return contains(VRAM_ADD_INCREMENT) ? 32 : 1;
}

uint16_t ControlRegister::sprt_pattern_addr() const {
    return contains(SPRITE_PATTERN_ADDR) ? 0x1000 : 0;
}

uint16_t ControlRegister::bknd_pattern_addr() const {
    return contains(BACKROUND_PATTERN_ADDR) ? 0x1000 : 0;
}

uint8_t ControlRegister::sprite_size() const {
    return contains(SPRITE_SIZE) ? 16 : 8;
}

uint8_t ControlRegister::master_slave_select() const {
    return contains(MASTER_SLAVE_SELECT) ? 1 : 0;
}

bool ControlRegister::generate_vblank_nmi() const {
    return contains(GENERATE_NMI);
}

void ControlRegister::update(uint8_t data) {
    bits = data;
}
