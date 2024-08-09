#pragma once

#include <cstdint>

class ControlRegister {
public:
    static constexpr uint8_t NAMETABLE1              = 0b00000001;
    static constexpr uint8_t NAMETABLE2              = 0b00000010;
    static constexpr uint8_t VRAM_ADD_INCREMENT      = 0b00000100;
    static constexpr uint8_t SPRITE_PATTERN_ADDR     = 0b00001000;
    static constexpr uint8_t BACKROUND_PATTERN_ADDR  = 0b00010000;
    static constexpr uint8_t SPRITE_SIZE             = 0b00100000;
    static constexpr uint8_t MASTER_SLAVE_SELECT     = 0b01000000;
    static constexpr uint8_t GENERATE_NMI            = 0b10000000;

    ControlRegister();

    uint16_t nametable_addr() const;
    uint8_t vram_addr_increment() const;
    uint16_t sprt_pattern_addr() const;
    uint16_t bknd_pattern_addr() const;
    uint8_t sprite_size() const;
    uint8_t master_slave_select() const;
    bool generate_vblank_nmi() const;
    void update(uint8_t data);

private:
    uint8_t bits;
    bool contains(uint8_t flag) const;
};